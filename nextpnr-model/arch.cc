/*
 *  nextpnr -- Next Generation Place and Route
 *
 *  Copyright (C) 2022  gatecat <gatecat@ds0.me>
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include "log.h"
#include "nextpnr.h"
#include "util.h"
#include "viaduct_api.h"
#include "viaduct_helpers.h"

#include <cmath>
#include <sstream>
#include <fstream>

#define GEN_INIT_CONSTIDS
#define VIADUCT_CONSTIDS "constids.inc"
#include "viaduct_constids.h"

NEXTPNR_NAMESPACE_BEGIN

namespace {
struct FutureFpgaImpl : ViaductAPI
{
    ~FutureFpgaImpl(){};
    void init(Context *ctx) override
    {
        init_uarch_constids(ctx);
        ViaductAPI::init(ctx);
        h.init(ctx);
        init_pcpi_ports();
        init_wires();
        init_bels();
        init_pips();
        print_size_estimate();
        generate_device();
    }

    BelId pcpi_bel;

    void pack() override
    {
        // Trim nextpnr IOBs - assume IO buffer insertion has been done in synthesis
        const pool<CellTypePort> top_ports{
                CellTypePort(id_INBUF, id_PAD),
                CellTypePort(id_OUTBUF, id_PAD),
        };
        h.remove_nextpnr_iobs(top_ports);
        // Replace constants with LUTs
        const dict<IdString, Property> vcc_params = {{id_INIT, Property(0xFFFF, 16)}};
        const dict<IdString, Property> gnd_params = {{id_INIT, Property(0x0000, 16)}};
        h.replace_constants(CellTypePort(id_LUT4, id_F), CellTypePort(id_LUT4, id_F), vcc_params, gnd_params);
        // Constrain directly connected LUTs and FFs together to use dedicated resources
        int lutffs = h.constrain_cell_pairs(pool<CellTypePort>{{id_LUT4, id_F}}, pool<CellTypePort>{{id_DFF, id_D}}, 1,
                                            false);
        log_info("Constrained %d LUTFF pairs.\n", lutffs);
        // Treat clock as implicitly routed
        for (auto &cell : ctx->cells) {
            CellInfo *ci = cell.second.get();
            if (ci->type == id_DFF)
                ci->disconnectPort(id_CLK);
        }
        // Place PCPI bel
        for (auto &cell : ctx->cells) {
            CellInfo *ci = cell.second.get();
            if (ci->type == id_PCPI_IF) {
                ci->disconnectPort(ctx->id("pcpi_clock"));
                ci->disconnectPort(ctx->id("pcpi_reset"));
                ci->ports.erase(ctx->id("pcpi_clock"));
                ci->ports.erase(ctx->id("pcpi_reset"));
                ctx->bindBel(pcpi_bel, ci, STRENGTH_LOCKED);
            }
        }
    }

    void prePlace() override { assign_cell_info(); }

    void postRoute() override { write_bitstream(); }

    bool isBelLocationValid(BelId bel) const override
    {
        Loc l = ctx->getBelLocation(bel);
        if (is_io(l.x, l.y)) {
            return true;
        } else {
            return slice_valid(l.x, l.y, l.z / 2);
        }
    }

  private:
    ViaductHelpers h;
    // Configuration
    // Grid size including IOBs at edges
    const int M = 15;
    const int X = M, Y = M;
    // SLICEs per tile
    const int N = 8;
    // LUT input count
    const int K = 4;
    // LUT input mux size
    const int ImuxN = 16;
    const int Nbounce = 32;

    // For fast wire lookups
    struct TileWires
    {
        std::vector<WireId> clk, q, f, d;
        std::vector<WireId> slice_inputs, lut_inputs;
        std::vector<WireId> slice_outputs;
        std::vector<WireId> bounce;
    };

    std::vector<std::vector<TileWires>> wires_by_tile;

    std::vector<std::pair<IdString, PortType>> pcpi_ports;

    void init_pcpi_ports()
    {
        pcpi_ports.emplace_back(ctx->id("pcpi_clock"), PORT_OUT);
        pcpi_ports.emplace_back(ctx->id("pcpi_reset"), PORT_OUT);
        pcpi_ports.emplace_back(ctx->id("pcpi_valid"), PORT_OUT);
        for (int i = 0; i < 32; i++)
            pcpi_ports.emplace_back(ctx->id(stringf("pcpi_insn[%d]", i)), PORT_OUT);
        for (int i = 0; i < 32; i++)
            pcpi_ports.emplace_back(ctx->id(stringf("pcpi_rs1[%d]", i)), PORT_OUT);
        for (int i = 0; i < 32; i++)
            pcpi_ports.emplace_back(ctx->id(stringf("pcpi_rs2[%d]", i)), PORT_OUT);
        pcpi_ports.emplace_back(ctx->id("pcpi_wr"), PORT_IN);
        for (int i = 0; i < 32; i++)
            pcpi_ports.emplace_back(ctx->id(stringf("pcpi_rd[%d]", i)), PORT_IN);
        pcpi_ports.emplace_back(ctx->id("pcpi_wait"), PORT_IN);
        pcpi_ports.emplace_back(ctx->id("pcpi_ready"), PORT_IN);
    }

    // Create wires to attach to bels and pips
    void init_wires()
    {
        NPNR_ASSERT(X >= 3);
        NPNR_ASSERT(Y >= 3);
        NPNR_ASSERT(K >= 2);
        NPNR_ASSERT(N >= 1);

        log_info("Creating wires...\n");
        wires_by_tile.resize(Y);
        for (int y = 0; y < Y; y++) {
            auto &row_wires = wires_by_tile.at(y);
            row_wires.resize(X);
            for (int x = 0; x < X; x++) {
                auto &w = row_wires.at(x);
                for (int z = 0; z < N; z++) {
                    // FF input
                    w.clk.push_back(ctx->addWire(h.xy_id(x, y, ctx->id(stringf("CLK%d", z))), ctx->id("CLK"), x, y));
                    w.d.push_back(ctx->addWire(h.xy_id(x, y, ctx->id(stringf("D%d", z))), ctx->id("D"), x, y));
                    // FF and LUT outputs
                    w.q.push_back(ctx->addWire(h.xy_id(x, y, ctx->id(stringf("Q%d", z))), ctx->id("Q"), x, y));
                    w.f.push_back(ctx->addWire(h.xy_id(x, y, ctx->id(stringf("F%d", z))), ctx->id("F"), x, y));
                    // LUT inputs inc permutation
                    for (int i = 0; i < K; i++) {
                        w.slice_inputs.push_back(
                                ctx->addWire(h.xy_id(x, y, ctx->id(stringf("S%dI%d", z, i))), ctx->id("S"), x, y));
                        w.lut_inputs.push_back(
                                ctx->addWire(h.xy_id(x, y, ctx->id(stringf("L%dI%d", z, i))), ctx->id("I"), x, y));
                    }
                    w.slice_outputs.push_back(ctx->addWire(h.xy_id(x, y, ctx->id(stringf("SLICEOUT[%d]", z))),
                                                           ctx->id("SLICEOUT"), x, y));
                }
                for (int i = 0; i < Nbounce; i++)
                    w.bounce.push_back(ctx->addWire(h.xy_id(x, y, ctx->id(stringf("BOUNCE%d", i))), ctx->id("B"), x, y));
            }
        }
    }
    bool is_io(int x, int y) const
    {
        // IO are on the edge of the device
        return (x == 0);
    }
    // Create IO bels in an IO tile
    void add_io_bel()
    {
        BelId b = ctx->addBel(h.xy_id(0, 0, id_PCPI_IF), id_PCPI_IF, Loc(0, 0, 0), false, false);
        unsigned i = 0, j = 0;
        for (auto port : pcpi_ports) {
            IdString name = port.first;
            if (name == ctx->id("pcpi_clock") || name == ctx->id("pcpi_reset")) {
                // implicit, no wire
                ctx->addBelOutput(b, name, WireId());
            } else if (port.second == PORT_OUT) {
                // output from picorv32 to efpga
                unsigned y = (i / N), z = (i % N);
                ctx->addBelOutput(b, name, wires_by_tile.at(y).at(0).slice_outputs.at(z));
                i++;
            } else if (port.second == PORT_IN) {
                // output from efpga to picorv32
                unsigned y = (j / N), z = (j % N);
                ctx->addBelInput(b, name, wires_by_tile.at(y).at(0).slice_inputs.at(z));
                j++;
            }
        }
        pcpi_bel = b;
    }
    PipId add_pip(Loc loc, WireId src, WireId dst, delay_t delay = 0.05)
    {
        IdStringList name = IdStringList::concat(ctx->getWireName(dst), ctx->getWireName(src));
        return ctx->addPip(name, ctx->id("PIP"), src, dst, delay, loc);
    }
    // Create LUT and FF bels in a logic tile
    void add_slice_bels(int x, int y)
    {
        auto &w = wires_by_tile.at(y).at(x);
        for (int z = 0; z < N; z++) {
            // Create LUT bel
            BelId lut = ctx->addBel(h.xy_id(x, y, ctx->id(stringf("SLICE%d_LUT", z))), id_LUT4, Loc(x, y, z * 2), false,
                                    false);
            for (int k = 0; k < K; k++)
                ctx->addBelInput(lut, ctx->id(stringf("I[%d]", k)), w.lut_inputs.at(z * K + k));
            ctx->addBelOutput(lut, id_F, w.f.at(z));
            // FF data can come from LUT output or LUT I3
            add_pip(Loc(x, y, 0), w.f.at(z), w.d.at(z));
            add_pip(Loc(x, y, 0), w.slice_inputs.at(z * K + (K - 1)), w.d.at(z));
            // Create DFF bel
            BelId dff = ctx->addBel(h.xy_id(x, y, ctx->id(stringf("SLICE%d_FF", z))), id_DFF, Loc(x, y, z * 2 + 1),
                                    false, false);
            ctx->addBelInput(dff, id_CLK, w.clk.at(z));
            ctx->addBelInput(dff, id_D, w.d.at(z));
            ctx->addBelOutput(dff, id_Q, w.q.at(z));
            for (int k = 0; k < K; k++) {
                // LUT permutation
                for (int k2 = 0; k2 < K; k2++) {
                    add_pip(Loc(x, y, 0), w.slice_inputs.at(z * K + k2), w.lut_inputs.at(z * K + k));
                }
                // LUT thru
                add_pip(Loc(x, y, 0), w.slice_inputs.at(z * K + k), w.f.at(z));
            }
            // LUT/FF mux
            add_pip(Loc(x, y, 0), w.f.at(z), w.slice_outputs.at(z));
            add_pip(Loc(x, y, 0), w.q.at(z), w.slice_outputs.at(z));
        }
    }
    // Create bels according to tile type
    void init_bels()
    {
        log_info("Creating bels...\n");
        add_io_bel();
        for (int y = 0; y < Y; y++) {
            for (int x = 0; x < X; x++) {
                if (!is_io(x, y)) {
                    add_slice_bels(x, y);
                }
            }
        }
    }

    struct Mux {
        WireId output;
        std::vector<PipId> inputs;
        int start_bit = 0, bit_count = 0;
    };

    dict<std::pair<int, int>, std::vector<Mux>> muxes;

    void create_pips(int x, int y)
    {
        std::vector<WireId> friends, bounce_friends;
        auto &w = wires_by_tile.at(y).at(x);
        auto proc_xy = [&](int x1, int y1) {
            if (x1 < 0 || x1 >= X)
                return;
            if (y1 < 0 || y1 >= Y)
                return;
            auto &w1 = wires_by_tile.at(y1).at(x1);
            for (WireId wire : w1.slice_outputs) {
                friends.push_back(wire);
                if (x1 == x && y1 == y)
                    bounce_friends.push_back(wire);
            }
            for (WireId wire : w1.bounce) {
                friends.push_back(wire);
                bounce_friends.push_back(wire);
            }
        };
        proc_xy(x, y);
        proc_xy(x-1, y);
        proc_xy(x+1, y);
        proc_xy(x, y-1);
        proc_xy(x, y+1);
        int offset = 0;
        int delta = (int(friends.size()) / ImuxN);
        for (WireId sink : w.slice_inputs) {
            Mux imux;
            imux.output = sink;
            for (int i = 0; i < ImuxN; i++) {
                WireId src = friends.at((offset + i * delta) % int(friends.size()));
                imux.inputs.push_back(add_pip(Loc(x, y, 0), src, sink));
            }
            muxes[std::make_pair(x, y)].push_back(imux);
            offset++;
        }
        delta = (int(bounce_friends.size()) / ImuxN);
        for (WireId sink : w.bounce) {
            Mux imux;
            imux.output = sink;
            for (int i = 0; i < ImuxN; i++) {
                WireId src = bounce_friends.at((offset + i * delta) % int(bounce_friends.size()));
                imux.inputs.push_back(add_pip(Loc(x, y, 0), src, sink));
            }
            muxes[std::make_pair(x, y)].push_back(imux);
            offset++;
        }
    }

    void init_pips()
    {
        log_info("Creating pips...\n");
        for (int y = 0; y < Y; y++)
            for (int x = 0; x < X; x++) {
                create_pips(x, y);
            }
    }

    int slice_cfg_size = 20;
    void print_size_estimate()
    {
        int route_bits = 0;
        for (const auto &tile : muxes) {
            for (const auto &m : tile.second)
                route_bits += int(std::ceil(std::log2(m.inputs.size())));
        }
        int logic_bits = (X - 2) * (Y - 2) * N * slice_cfg_size;
        log_info("%d route bits, %d logic bits, %dK total\n",
            route_bits, logic_bits, (route_bits + logic_bits) / 1024);
    }

    // Validity checking
    struct FutureFpgaCellInfo
    {
        const NetInfo *lut_f = nullptr, *ff_d = nullptr;
        bool lut_i3_used = false;
    };
    std::vector<FutureFpgaCellInfo> fast_cell_info;
    void assign_cell_info()
    {
        fast_cell_info.resize(ctx->cells.size());
        for (auto &cell : ctx->cells) {
            CellInfo *ci = cell.second.get();
            auto &fc = fast_cell_info.at(ci->flat_index);
            if (ci->type == id_LUT4) {
                fc.lut_f = ci->getPort(id_F);
                fc.lut_i3_used = (ci->getPort(ctx->id(stringf("I[%d]", K - 1))) != nullptr);
            } else if (ci->type == id_DFF) {
                fc.ff_d = ci->getPort(id_D);
            }
        }
    }
    bool slice_valid(int x, int y, int z) const
    {
        const CellInfo *lut = ctx->getBoundBelCell(ctx->getBelByLocation(Loc(x, y, z * 2)));
        const CellInfo *ff = ctx->getBoundBelCell(ctx->getBelByLocation(Loc(x, y, z * 2 + 1)));
        if (!lut || !ff)
            return true; // always valid if only LUT or FF used
        const auto &lut_data = fast_cell_info.at(lut->flat_index);
        const auto &ff_data = fast_cell_info.at(ff->flat_index);
        // In our example arch; the FF D can either be driven from LUT F or LUT I3
        // so either; FF D must equal LUT F or LUT I3 must be unused
        if (ff_data.ff_d == lut_data.lut_f && lut_data.lut_f->users.entries() == 1)
            return true;
        // Can't route FF and LUT output separately
        return false;
    }
    // Bel bucket functions
    IdString getBelBucketForCellType(IdString cell_type) const override
    {
        if (cell_type.in(id_INBUF, id_OUTBUF))
            return id_IOB;
        return cell_type;
    }
    bool isValidBelForCellType(IdString cell_type, BelId bel) const override
    {
        IdString bel_type = ctx->getBelType(bel);
        if (bel_type == id_IOB)
            return cell_type.in(id_INBUF, id_OUTBUF);
        else
            return (bel_type == cell_type);
    }

    int cfg_depth = 50;

    std::string veri_cbit(int b) {
        return stringf("C[%d][%d]", b / cfg_depth, b % cfg_depth);
    }

    std::string veri_bit_range(int &b, int count) {
        std::string result = "{";
        for (int i = 0; i < count; i++) {
            if (result.size() > 1) result += ",";
            result += veri_cbit(b + ((count - 1) - i));
        }
        result += "}";
        b += count;
        return result;
    }

    std::string veri_wire(WireId wire) {
        return stringf("W%d", wire.index);
    }

    std::string veri_wire_range(const std::vector<WireId> &wires, int offset = 0, int count = -1) {
        std::string result = "{";
        if (count == -1)
            count = wires.size();
        for (int i = 0; i < count; i++) {
             if (result.size() > 1) result += ",";
            result += veri_wire(wires.at(offset + ((count - 1) - i)));
        }
        result += "}";
        return result;
    }

    int cfg_height = 0;
    dict<std::pair<int, int>, std::vector<int>> slice_start_bit;
    void generate_device() {
        int bit = 0;
        std::ostringstream insts;

        for (int y = 0; y < Y; y++) {
            for (int x = 0; x < X; x++) {
                // Generate muxes
                int mux_i = 0;
                for (auto &mux : muxes[std::make_pair(x, y)]) {
                    int bits = std::ceil(std::log2(mux.inputs.size()));
                    insts << stringf("rmux #(%d) X%dY%dM%d (", bits, x, y, mux_i++);
                    mux.start_bit = bit;
                    mux.bit_count = bits;
                    std::vector<WireId> input_wires;
                    for (auto pip : mux.inputs) input_wires.push_back(ctx->getPipSrcWire(pip));
                    insts << veri_bit_range(bit, bits) << "," << veri_wire_range(input_wires) << "," << veri_wire(mux.output);
                    insts << ");\n";
                }
                // Generate slices - TODO: IO...
                insts << "\n";
                auto &w = wires_by_tile.at(y).at(x);
                if (!is_io(x, y)) {
                    for (int z = 0; z < N; z++) {
                        insts << stringf("futurefpga_slice X%dY%dZ%d(clk, rst, ", x, y, z);
                        insts << veri_wire_range(w.slice_inputs, z*K, K) << "," << veri_wire(w.slice_outputs.at(z)) << ",";
                        slice_start_bit[std::make_pair(x, y)].push_back(bit);
                        insts << veri_bit_range(bit, slice_cfg_size);
                        insts << ");\n";
                    }
                }
                insts << "\n";
            }
        }
        cfg_height = (bit + cfg_depth - 1) / cfg_depth;
        std::ofstream out("futurefpga_arch.v"); // TODO: don't hardcode
        out << "`define CFG_DEPTH " << cfg_depth << "\n";
        out << "`define CFG_HEIGHT " << cfg_height << "\n";

        out << "module fpga(\n";
        out << "    input wire clk, rst, shift,\n";
        out << stringf("    input wire [%d:0] cdata,\n", cfg_height - 1);
        out << "    input wire pcpi_valid,\n";
        out << "    input wire [31:0] pcpi_insn, pcpi_rs1, pcpi_rs2,\n";
        out << "    output wire pcpi_wr, pcpi_wait, pcpi_ready,\n";
        out << "    output wire [31:0] pcpi_rd\n";
        out << ");\n";
        for (int i = 0; i < int(ctx->wires.size()); i += 30) {
            out << "wire ";
            for (int j = 0; j < 30; j++) {
                if ((j + i) >= int(ctx->wires.size()))
                    break;
                if (j > 0) out << ", ";
                out << "W" << (j + i);
            }
            out << ";\n";
        }
        out << stringf("reg [%d:0] C[0:%d];\n", cfg_depth-1, cfg_height-1);
        out << "generate\n";
        out << "genvar ii;\n";
        out << stringf("   for (ii = 0; ii < %d; ii = ii + 1'b1) begin: cr\n", cfg_height);
        out << stringf("        always @(posedge clk) if (shift) C[ii] <= {C[ii][%d:0], cdata[ii]};\n", cfg_depth-2);
        out << "    end\n";
        out << "endgenerate\n";
        out << insts.str();
        // PCPI wiring
        for (auto pin : ctx->getBelPins(pcpi_bel)) {
            if (pin == ctx->id("pcpi_clock") || pin == ctx->id("pcpi_reset"))
                continue;
            if (ctx->getBelPinType(pcpi_bel, pin) == PORT_OUT)
                out << "assign "<< veri_wire(ctx->getBelPinWire(pcpi_bel, pin)) << stringf(" = %s;\n", pin.c_str(ctx));
            else
                out << stringf("assign %s = ", pin.c_str(ctx)) << veri_wire(ctx->getBelPinWire(pcpi_bel, pin)) << ";\n";
        }
        out << "endmodule\n";
    }

    unsigned depermute_lut_init(const TileWires &w, int z, unsigned orig_init) {
        std::vector<std::vector<unsigned>> phys_to_log;
        phys_to_log.resize(K);
        for (int i = 0; i < K; i++) {
            WireId pin_wire = w.lut_inputs.at(z * K + i);
            for (PipId pip : ctx->getPipsUphill(pin_wire)) {
                if (!ctx->getBoundPipNet(pip))
                    continue;
                for (int j = 0; j < K; j++) {
                    if (w.slice_inputs.at(z * K + j) != ctx->getPipSrcWire(pip))
                        continue;
                    phys_to_log.at(j).push_back(i);
                }
            }
        }
        unsigned permuted_init = 0;
        for (int i = 0; i < (1 << K); i++) {
            unsigned log_idx = 0;
            for (int j = 0; j < K; j++) {
                if ((i >> j) & 0x1) {
                    for (auto log_pin : phys_to_log[j])
                        log_idx |= (1 << log_pin);
                }
            }
            if ((orig_init >> log_idx) & 0x1)
                permuted_init |= (1 << i);
        }
        return permuted_init;
    }

    void write_bitstream() {
        std::vector<bool> bits;
        bits.resize(cfg_height * cfg_depth, false);
        for (int y = 0; y < Y; y++) {
            for (int x = 0; x < X; x++) {
                auto &w = wires_by_tile.at(y).at(x);
                if (!is_io(x, y)) {
                    for (int z = 0; z < N; z++) {
                        int start_bit = slice_start_bit.at(std::make_pair(x, y)).at(z);
                        unsigned slice_config = 0;
                        // lut route-thru ?
                        for (PipId p : ctx->getPipsUphill(w.f.at(z))) {
                            if (!ctx->getBoundPipNet(p))
                                continue;
                            for (int i = 0; i < K; i++) {
                                if (ctx->getPipSrcWire(p) != w.slice_inputs.at(z * K + i))
                                    continue;
                                // it is! add route through LUT config
                                for (int j = 0; j < (1 << K); j++)
                                    if ((j >> i) & 0x1)
                                        slice_config |= (1 << j);
                            }
                        }
                        CellInfo *lut = ctx->getBoundBelCell(ctx->getBelByLocation(Loc(x, y, z * 2)));
                        if (lut) {
                            NPNR_ASSERT(slice_config == 0); // thru not used
                            slice_config |= depermute_lut_init(w, z, int_or_default(lut->params, ctx->id("INIT")));
                        }
                        CellInfo *ff = ctx->getBoundBelCell(ctx->getBelByLocation(Loc(x, y, z * 2 + 1)));
                        if (ff) {
                            slice_config |= (1U << 16U); // FF used
                            for (PipId pip : ctx->getPipsUphill(w.d.at(z))) {
                                if (!ctx->getBoundPipNet(pip))
                                    continue;
                                if (ctx->getPipSrcWire(pip) != w.f.at(z)) // FF input not from LUT
                                    slice_config |= (1U << 17U); // FF isel
                            }
                        }
                        for (int i = 0; i < slice_cfg_size; i++)
                            bits.at(start_bit + i) = bool((slice_config >> i) & 0x1);
                    }
                }
            }
        }
        for (auto &mux_tile : muxes) {
            for (auto &mux : mux_tile.second) {
                for (unsigned i = 0; i < mux.inputs.size(); i++) {
                    if (!ctx->getBoundPipNet(mux.inputs.at(i)))
                        continue;
                    for (int j = 0; j < mux.bit_count; j++) {
                        bits.at(mux.start_bit + j) = ((i >> j) & 0x1); // set mux pattern
                    }
                    break;
                }
            }
        }
        std::ofstream out("futurefpga.bit"); // todo: output path
        for (int y = 0; y < cfg_height; y++) {
            for (int x = cfg_depth - 1; x >= 0; x--) {
                out << (bits.at(y * cfg_depth + x) ? '1' : '0');
            }
            out << "\n";
        }
    }
};

struct FutureFpgaArch : ViaductArch
{
    FutureFpgaArch() : ViaductArch("futurefpga"){};
    std::unique_ptr<ViaductAPI> create(const dict<std::string, std::string> &args)
    {
        return std::make_unique<FutureFpgaImpl>();
    }
} exampleArch;
} // namespace

NEXTPNR_NAMESPACE_END

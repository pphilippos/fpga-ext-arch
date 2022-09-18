XLEN ?= 32
TOOL_DIR= /home/opt/riscv-gnu-toolchain-newlib-multilib-jul-10-2020/
CROSS   = $(TOOL_DIR)/bin/riscv64-unknown-elf-
CC      = $(CROSS)gcc
CPP     = $(CROSS)cpp
OBJCOPY = $(CROSS)objcopy
OBJDUMP = $(CROSS)objdump
ARCH    = $(CROSS)ar
DEBUG ?= 0
BASE_ADDRESS ?= 0x00008000

#ifeq ($(XLEN), 64)
#    MARCH = rv64imf
#    MABI = lp64f
#	STACK_SIZE = 65536
#else
#    MARCH = rv32imf
#    MABI = ilp32f
#	STACK_SIZE = 65536
#endif

STACK_SIZE = 65536

BUILD_DIR       = build
RTOS_SOURCE_DIR = $(abspath ../../Source)
DEMO_SOURCE_DIR = $(abspath ../Common/Minimal)

CPPFLAGS = \
	-DportasmHANDLE_INTERRUPT=handle_trap \
	-I . -I ../Common/include \
	-I $(RTOS_SOURCE_DIR)/include \
	-I $(RTOS_SOURCE_DIR)/portable/GCC/RISC-V -fno-schedule-insns 

ifeq ($(MARCH), rv32imf)
	CPPFLAGS += -I $(RTOS_SOURCE_DIR)/portable/GCC/RISC-V/chip_specific_extensions/RISCV_MTIME_CLINT_F
endif
ifeq ($(MARCH), rv32if)
	CPPFLAGS += -I $(RTOS_SOURCE_DIR)/portable/GCC/RISC-V/chip_specific_extensions/RISCV_MTIME_CLINT_F
endif
ifeq ($(MARCH), rv32i)
	CPPFLAGS += -I $(RTOS_SOURCE_DIR)/portable/GCC/RISC-V/chip_specific_extensions/RV32I_CLINT_no_extensions
endif 
ifeq ($(MARCH), rv32im)
	CPPFLAGS += -I $(RTOS_SOURCE_DIR)/portable/GCC/RISC-V/chip_specific_extensions/RV32I_CLINT_no_extensions
endif 
	# new header for including F
#-D__riscv_float_abi_soft \
	#-I $(RTOS_SOURCE_DIR)/portable/GCC/RISC-V/chip_specific_extensions/RV32I_CLINT_no_extensions

CFLAGS  += -Bstatic -march=$(MARCH) -mabi=$(MABI) -mcmodel=medany \
	-fmessage-length=0 \
	-ffunction-sections \
	-fdata-sections \
	-g -pedantic -ffreestanding  -Wextra -Wshadow -Wundef -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Wredundant-decls
#-Wall \		-fno-builtin-printf \
ASFLAGS = -march=$(MARCH) -mabi=$(MABI) -mcmodel=medany -lm
LDFLAGS = -nostartfiles \
	-Xlinker --gc-sections \
	-Xlinker --defsym=__stack_size=$(STACK_SIZE) \
	-Xlinker -lm

ifeq ($(DEBUG), 1)
    CFLAGS += -Og -ggdb3 -lm
else
    CFLAGS += -Os -lm -O3 -ffast-math -fno-schedule-insns 
endif

#CFLAGS += -DBe0 -DBe1

SRCS = boardsupport.c main.c main_blinky.c riscv-virt.c htif.c \
	$(DEMO_SOURCE_DIR)/EventGroupsDemo.c \
	$(DEMO_SOURCE_DIR)/TaskNotify.c \
	$(DEMO_SOURCE_DIR)/TimerDemo.c \
	$(DEMO_SOURCE_DIR)/blocktim.c \
	$(DEMO_SOURCE_DIR)/dynamic.c \
	$(DEMO_SOURCE_DIR)/recmutex.c \
	$(RTOS_SOURCE_DIR)/event_groups.c \
	$(RTOS_SOURCE_DIR)/list.c \
	$(RTOS_SOURCE_DIR)/queue.c \
	$(RTOS_SOURCE_DIR)/stream_buffer.c \
	$(RTOS_SOURCE_DIR)/tasks.c \
	$(RTOS_SOURCE_DIR)/timers.c \
	$(RTOS_SOURCE_DIR)/portable/MemMang/heap_4.c \
	$(RTOS_SOURCE_DIR)/portable/GCC/RISC-V/port.c \
	embench_allinone/beebsc.c 
	

ASMS = start.S \
	$(RTOS_SOURCE_DIR)/portable/GCC/RISC-V/portASM.S

OBJS = $(SRCS:%.c=$(BUILD_DIR)/%$(XLEN).o) $(ASMS:%.S=$(BUILD_DIR)/%$(XLEN).o)
DEPS = $(SRCS:%.c=$(BUILD_DIR)/%$(XLEN).d) $(ASMS:%.S=$(BUILD_DIR)/%$(XLEN).d)

$(BUILD_DIR)/RTOSDemo$(XLEN).axf: $(OBJS) $(BUILD_DIR)/fake_rom$(BASE_ADDRESS).lds Makefile
	$(CC) $(LDFLAGS) $(CFLAGS) $(OBJS) -T$(BUILD_DIR)/fake_rom$(BASE_ADDRESS).lds -o $@ -lm

$(BUILD_DIR)/%$(XLEN).o: %.c Makefile
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@ -lm

$(BUILD_DIR)/%$(XLEN).o: %.S Makefile
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(ASFLAGS) -MMD -MP -c $< -o $@ -lm

# Run lds through the C preprocessor, to replace BASE_ADDRESS with the actual
# value. It might be simpler to use sed instead.
$(BUILD_DIR)/%$(BASE_ADDRESS).lds: fake_rom.lds Makefile
	$(CPP) $(CPPFLAGS) -DBASE_ADDRESS=$(BASE_ADDRESS) $< | grep -v '^#' > $@

clean:
	rm -rf $(BUILD_DIR)
	
binary: 
	$(OBJCOPY) -O binary $(BUILD_DIR)/RTOSDemo$(XLEN).axf $(BUILD_DIR)/firmware.bin
	$(OBJDUMP) -s -t -r -d -f --source $(BUILD_DIR)/RTOSDemo$(XLEN).axf > $(BUILD_DIR)/objdump.txt
	cat $(BUILD_DIR)/objdump.txt | egrep ":	" | awk '{print $$3}' | sort | uniq | grep -v "[0-9]" | grep -v "\"" | tr '\n' ' '| cat <(echo "Unique Instructions:") - <(echo "")
	cat $(BUILD_DIR)/objdump.txt | grep ":	" | awk '{print $$3}' | sort | uniq | grep -v "[0-9]" | grep -v "\"" | wc -l
	
csr: 
	cat $(BUILD_DIR)/objdump.txt | grep ":	" | grep csr
-include $(DEPS)

upload:
	cp $(BUILD_DIR)/firmware.bin  ~/mountpoint/home/root/simodenseM/firmware.bin
	cp $(BUILD_DIR)/RTOSDemo32.axf ~/mountpoint/home/root/simodenseM/firmware.elf



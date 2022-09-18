NEXTPNR=$(HOME)/nextpnr

CXXFLAGS=-std=c++14 -g -Wall -fPIC -I$(NEXTPNR)/common/kernel -I$(NEXTPNR)/common/place -I$(NEXTPNR)/common/route\
 -I. -I$(NEXTPNR)/generic -DARCHNAME=generic -DARCH_GENERIC -DNEXTPNR_NAMESPACE=nextpnr_generic
LDFLAGS=-std=c++14 -g -shared
LIB_NAME=arch_gen.so
OBJECTS=$(patsubst %.cc,%.o,$(wildcard *.cc))

all: $(LIB_NAME)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(LIB_NAME): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^

clean:
	rm -f *.o $(LIB_NAME)

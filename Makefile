CPPFLAGS = -ILeechCore/includes -DLINUX -g
LDFLAGS = -LLeechCore/files -LLeechCore-plugins/files
LDLIBS = -l:leechcore.so -ldl -lstdc++

SRCS = main.cpp trace_reader.cpp page_table.cpp device.cpp
OBJS = $(SRCS:.cpp=.o)

all: leechcore l4trace l4ctf

l4trace: $(OBJS)
	g++ $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

.PHONY: leechcore
leechcore:
	$(MAKE) -C LeechCore/leechcore
	$(MAKE) -C LeechCore-plugins/leechcore_ft601_driver_linux
	mkdir -p lib
	cp LeechCore/files/leechcore.so lib/
	cp LeechCore-plugins/files/leechcore_ft601_driver_linux.so lib/

.PHONY: l4ctf
l4ctf:
	$(MAKE) -C l4ctf

clean:
	rm -rf lib/ l4trace *.o


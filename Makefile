CPPFLAGS = -ILeechCore/includes -DLINUX -g
LDFLAGS = -LLeechCore/files -LLeechCore-plugins/files
LDLIBS = -l:leechcore.so -ldl -lstdc++

SRCS = main.cpp trace_reader.cpp page_table.cpp device.cpp
OBJS = $(SRCS:.cpp=.o)

#TODO: dependency on header file

all: l4trace run

l4trace: $(OBJS)
	g++ $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

.PHONY: run
run: l4trace
	LD_LIBRARY_PATH=./lib ./l4trace

.PHONY: leechcore
leechcore:
	$(MAKE) -C LeechCore/leechcore
	$(MAKE) -C LeechCore-plugins/leechcore_ft601_driver_linux
	mkdir -p lib
	cp LeechCore/files/leechcore.so lib/
	cp LeechCore-plugins/files/leechcore_ft601_driver_linux.so lib/

clean:
	rm -rf lib/ l4trace *.o


CPPFLAGS = -ILeechCore/leechcore -DLINUX
LDFLAGS = -Llib
LDLIBS = -l:leechcore.so

SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)

l4trace: leechcore $(OBJS)
	c++ $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

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
	rm -f l4trace *.o

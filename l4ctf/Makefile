LD_FLAGS = -shared $(pkg-config --libs babeltrace2)
CFLGAS = -fPIC -c $(pkg-config --cflags babeltrace2)

SRCS = plugin.c
OBJS = $(SRCS:.c=.o)

plugin.so: $(OBJS)
	ld $? -o $@ $(LD_FLAGS)

.PHONY: clean
clean:
	rm -f *.so *.o




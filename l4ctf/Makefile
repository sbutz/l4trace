LD_FLAGS = -shared $(pkg-config --libs babeltrace2)
CFLGAS = -fPIC -c $(pkg-config --cflags babeltrace2)

plugin.so: plugin.o
	ld $? -o $@ $(LD_FLAGS)

.PHONY: clean
clean:
	rm -f *.so *.o




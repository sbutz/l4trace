LD_FLAGS = -shared $(pkg-config --libs babeltrace2)
CFLGAS = -fPIC -c $(pkg-config --cflags babeltrace2)

plugin.so: plugin.o
	ld $? -o $@ $(LD_FLAGS)

.PHONY: run
run: plugin.so
	#babeltrace2 --plugin-path=. --component=source.l4trace.input
	babeltrace2 \
		--plugin-path=. \
		--component=source.l4trace.input \
		--params='path="/home/sbutz/git/l4trace/l4trace.out.60"' \
		--component=sink.ctf.fs \
		--params='path="/tmp/l4trace.ctf"'

.PHONY: clean
clean:
	rm -f *.so *.o




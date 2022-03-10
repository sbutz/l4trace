#!/bin/sh

OUT_FILE=/tmp/l4trace.out
OUT_CTF=/tmp/l4trace.ctf

# Extract tracerecords
LD_LIBRARY_PATH=./lib ./l4trace ${OUT_FILE}

if [ $? -eq 0 ]; then
    # Convert tracerecords
    babeltrace2 \
        --plugin-path=./l4ctf \
        --component=source.l4trace.input \
        --params="path=\"${OUT_FILE}\"" \
        --component=sink.ctf.fs \
        --params="path=\"${OUT_CTF}\""
fi
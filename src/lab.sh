#!/bin/bash
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/home/yaros/Yaros/Study/BCD/src";
make \
&& rm -f *.log \
&& rm -rf ./strace/* \
&& rm -rf ./strace/.strace* \
&& LD_PRELOAD=/home/yaros/Yaros/Study/BCD/src/libruntime.so strace -o strace/strace -f -ff ../bin/pa23 -p 7 1 2 3 4 5 6 7  \
&& sed -i '/.*\(temporarily\|fstat\|nanosleep\).*/d' strace/strace*

#!/bin/bash
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/home/yaros/Yaros/Study/BCD/src";
make \
&& rm -f *.log \
&& rm -rf ./strace/* \
&& rm -rf ./strace/.strace* \
&& echo 'start: '`date` \
&& LD_PRELOAD=/home/yaros/Yaros/Study/BCD/src/libruntime.so ; if [[ -z $1 ]] ; then strace -o strace/strace -f -ff ../bin/pa23 -p 2 10 20 ; else ../bin/pa23 -p 2 10 20 ; fi\
&& echo 'done: '`date` \
&& sed -i '/.*\(temporarily\|fstat\|nanosleep\).*/d' strace/strace*

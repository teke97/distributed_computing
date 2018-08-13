#!/bin/bash

make \
&& rm -f *.log \
&& rm -rf ./strace/* \
&& strace -o strace/strace -f -ff ../bin/lab1 -p 2

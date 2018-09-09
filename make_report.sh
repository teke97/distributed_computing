#!/bin/bash

DIR=pa5

if [ ! -d "$DIR" ]; then
	mkdir $DIR
fi

if [ -f "$DIR.tar.gz" ]; then
	rm $DIR.tar.gz
fi

rm -rf $DIR/*
cp src/* $DIR/
tar cfz $DIR.tar.gz $DIR

#!/bin/bash

if test -f "SVF-2.2/Release-build/Makefile"
then
	cd SVF-2.2/Release-build/
	make -j8
	cd ../../
else
	cd SVF-2.2/
	./build.sh
	cd ../
fi

for bench in bitcode-files/*.orig; do 
	SVF-2.2/Release-build/bin/svf-ex "$bench"
	sort -o graph.dot graph.dot
	mv graph.dot dot-files/$(basename "$bench" .orig).dot
done

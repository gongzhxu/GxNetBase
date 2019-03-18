#!/bin/bash

mkdir -p cbp
cd cbp
cmake -DCMAKE_BUILD_TYPE=Debug -B. -H../ -G "CodeBlocks - Unix Makefiles"	

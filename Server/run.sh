#!/bin/bash

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./libserver/build

./server -d/home/student/workspace/CS_Prjoect/Server -p5556

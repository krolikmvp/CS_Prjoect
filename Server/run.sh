#!/bin/bash

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./libserver/build
valgrind --leak-check=full ./server -d/home/student/workspace/CS_Prjoect/Server -p5556
#cgdb ./server 
#./server -d/home/student/workspace/CS_Prjoect/Server -p5556

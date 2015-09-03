#!/bin/bash

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./libclient/build

#./client -i127.0.0.1 -p5556
valgrind --leak-check=full ./client -i127.0.0.1 -p5556
#cgdb client

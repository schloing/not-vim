#!/bin/bash
set -xe
valgrind -s --track-origins=yes --leak-check=full --show-leak-kinds=all --log-file="valgrind" ./bin/nv --args $*

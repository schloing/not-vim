#!/bin/bash
set -xe
gdb ./bin/nv --args $*

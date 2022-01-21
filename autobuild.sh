#!/bin/bash

set -e
mkdir build

rm -rf `pwd`/build/*
cd `pwd`/build &&
	cmake .. &&
	make
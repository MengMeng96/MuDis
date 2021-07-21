#!/bin/bash
g++ stress-test/stress_test.cpp -o ./bin/stress  --std=c++11 -pthread  
./bin/stress

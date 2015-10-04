#!bin/sh
g++ coveralls.cpp -o  ../coveralls_cpp
cd ..
./coveralls_cpp Parser.h.gcov `md5sum Parser.h.gcov` VM.cpp.gcov `md5sum VM.cpp.gcov`
##curl -H "Content-Type: application/json" --data @coverage.json https://coveralls.io/api/v1/jobs

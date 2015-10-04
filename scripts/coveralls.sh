#!bin/bash
gcov -r src/*.cpp > coverage.txt
g++ scripts/coveralls.cpp -o  coveralls_cpp
ARGS_ARRAY=("$COV_TOKEN")

for f in `ls *.gcov`
do
  ARGS_ARRAY+=("$f");
  ARGS_ARRAY+=(`md5sum $f | awk '{ print $1 }'`); 
done

./coveralls_cpp ${ARGS_ARRAY[*]}
cat coveralls.json
curl -H "Content-Type: application/json" --data @coveralls.json https://coveralls.io/api/v1/jobs

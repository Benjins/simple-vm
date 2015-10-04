#!bin/bash
g++ coveralls.cpp -o  ../coveralls_cpp
cd ..
ARGS_ARRAY=("TokenIsGood214135135")

for f in `ls *.gcov`
do
 ARGS_ARRAY+=("$f");
 ARGS_ARRAY+=(`md5sum $f | awk '{ print $1 }'`);
done

./coveralls_cpp ${ARGS_ARRAY[*]}
curl -H "Content-Type: application/json" --data @coverage.json https://coveralls.io/api/v1/jobs

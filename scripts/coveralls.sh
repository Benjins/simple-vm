#!bin/bash
g++ coveralls.cpp -o  ../coveralls_cpp
cd ..
ARGS_ARRAY=("$COV_TOKEN")

for f in `ls *.gcov`
do
if  [[ "$f" != /* ]] ;
 then
  ARGS_ARRAY+=("$f");
  ARGS_ARRAY+=(`md5sum $f | awk '{ print $1 }'`); 
 fi
done

./coveralls_cpp ${ARGS_ARRAY[*]}
#curl -H "Content-Type: application/json" --data @coverage.json https://coveralls.io/api/v1/jobs

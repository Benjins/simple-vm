#!bin/bash
mv *.gcda src/
mv *.gcno src/
gcov -r src/*.cpp > coverage.txt
g++ scripts/coveralls.cpp -o  coveralls_cpp
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
cat coverage.json
#curl -H "Content-Type: application/json" --data @coverage.json https://coveralls.io/api/v1/jobs

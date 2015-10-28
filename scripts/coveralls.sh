#!bin/bash

mv *.gcda src/
mv *.gcno src/
gcov -r src/*.cpp > coverage.txt
g++ scripts/coveralls.cpp -o  coveralls_cpp
ARGS_ARRAY=("____")

for f in `ls *.gcov`
do
if  [[ "$f" != /* ]] ;
 then
  ARGS_ARRAY+=("$f");
  ARGS_ARRAY+=(`md5sum $f | awk '{ print $1 }'`); 
 fi
done

./coveralls_cpp ${ARGS_ARRAY[*]}

ssh-keygen -R login.ccs.neu.edu
sshpass -e scp -o 'StrictHostKeyChecking=no' coveralls.json $FTP_USER@login.ccs.neu.edu:~/.www/personal-projects/deployment/simple-vm/

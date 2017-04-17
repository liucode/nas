g++ -lssl -lcrypto -O0 -g3 -rdynamic ./main.cpp SSDsim.cpp FTL.cpp mytool.cpp -o test
for i in 1 2 3 4 5
do
  rm data 
  ./test 4 $i
done

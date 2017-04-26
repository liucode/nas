g++ -lssl -lcrypto -O0 -g3 -rdynamic ./main.cpp SSDsim.cpp FTL.cpp mytool.cpp generator.c -o test
for i in 2 3 4 5
do
  fstrim /mnt
  ./test 4 1000 $i 5 64 1
done

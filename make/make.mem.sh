g++ -lssl -lcrypto -O0 -g3 -rdynamic ./main.cpp SSDsim.cpp FTL.cpp mytool.cpp generator.c -o test
for i in 400 500 600 700
do
  fstrim /mnt
  ./test 3 $i 5 5 64 1
done

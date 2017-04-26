g++ -lssl -lcrypto -O0 -g3 -rdynamic ./main.cpp SSDsim.cpp FTL.cpp mytool.cpp generator.c -o test
for i in 1 3 4
do
  fstrim /mnt
  ./test $i 400 5 5 64 2
done

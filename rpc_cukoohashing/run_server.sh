make clean
make
n=$RANDOM
echo "server port $n"
./server $n
#valgrind --track-origins=yes ./server $n


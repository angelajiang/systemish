for i in `seq 4 4 64`; do
	echo "Size $i"
	./bench $i
done

echo "Size    ns/memcpy    inst/memcpy"
for i in `seq 4 16 256`; do
	./bench $i
done

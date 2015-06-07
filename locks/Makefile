all:
	gcc -O3 -o spinlock spinlock.c -lpthread -lrt
	gcc -O3 -o fetch-add fetch-add.c -lpthread -lrt
	gcc -O3 -o bouncing bouncing.c -lpthread -lrt -D_GNU_SOURCE
	gcc -O3 -o manylock manylock.c -lpthread -lrt -D_GNU_SOURCE
clean:
	rm spinlock fetch-add bouncing manylock


#include<stdlib.h>
#include<stdio.h>
#include<assert.h>

#include "ds_queue.h"

int main()
{
	struct ds_queue q;
	ds_queue_init(&q);

	ds_queue_add(&q, 1);
	assert(ds_queue_remove(&q) == 1);

	ds_queue_add(&q, 2);
	ds_queue_add(&q, 3);
	ds_queue_add(&q, 4);
	ds_queue_add(&q, 5);

	assert(ds_queue_remove(&q) == 2);
	assert(ds_queue_remove(&q) == 3);

	assert(ds_queue_size(&q) == 2);

	printf("Test passed\n");
	return 0;
}

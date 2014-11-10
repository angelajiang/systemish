#include<stdlib.h>
#include<assert.h>

#include "ds_queue.h"

void ds_queue_init(struct ds_queue q)
{
	q.head = NULL;
	q.tail = NULL;
	q.count = 0;
}

void ds_queue_add(struct ds_queue *q, int data)
{
	/**< Create a new null-terminated node */
	struct ds_qnode *new_node = malloc(sizeof(struct ds_qnode));
	assert(new_node != NULL);

	new_node->data = data;
	new_node->next = NULL;

	/**< If the queue is empty */
	if(q->head == NULL) {
		q->head = new_node;
		q->tail = new_node;
	} else {
		q->tail->next = new_node;
		q->tail = new_node;
	}

	q->count++;
}

int ds_queue_remove(struct ds_queue *q)
{
	int data;
	assert(q->head != NULL);

	struct ds_qnode *old_head;
	old_head = q->head;
	data = old_head->data;

	q->head = q->head->next;
	q->count --;
	if(q->head == NULL) {
		q->head = NULL;
		q->count = 0;
	}

	free(old_head);

	return data;
}


int ds_queue_count(struct ds_queue *q)
{
	return q->count;
}


void ds_queue_free(struct ds_queue *q)
{
	while(ds_queue_count(q)) {
		ds_queue_remove(q);
	}
}


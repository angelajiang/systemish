struct ds_qnode {
	int data;
	struct ds_qnode *next;
};

struct ds_queue {
	struct ds_qnode *head, *tail;
	int count;
};


void ds_queue_init(struct ds_queue q);
void ds_queue_add(struct ds_queue *q, int data);
int ds_queue_remove(struct ds_queue *q);
int ds_queue_count(struct ds_queue *q);
void ds_queue_free(struct ds_queue *q);

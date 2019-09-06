import multiprocessing
from multiprocessing import Process, Queue
import time
import sys

class QueueElement:
    def __init__(self, image_ids):
        self.image_ids = image_ids

def writer(queue):
    ## Write to the queue
    for i in range(1):
        image_ids = range(128)
        elem = QueueElement(image_ids)
        queue.put(elem)
    queue.put("DONE")
    print('writer is done')

class AsyncThing:
    def __init__(self):
        self.pqueue = Queue() # writer() writes to pqueue from _this_ process

    def run(self):
        ### reader_proc() reads from pqueue as a separate process
        writer_p = Process(target=writer, args=((self.pqueue),))
        writer_p.daemon = False
        writer_p.start()        # Launch reader_proc() as a separate python process
        time.sleep(15)
        print("sleep is done")
        while True:
            elem = self.pqueue.get()
            print(elem)
        #writer_p.join()

def run():
    async_thing = AsyncThing()
    async_thing.run()

    '''
    pqueue = Queue() # writer() writes to pqueue from _this_ process
    for count in [10**2]:             
        ### reader_proc() reads from pqueue as a separate process
        reader_p = Process(target=reader_proc, args=((pqueue),))
        reader_p.daemon = True
        reader_p.start()        # Launch reader_proc() as a separate python process

        _start = time.time()
        writer(count, pqueue)    # Send a lot of stuff to reader()
        reader_p.join()         # Wait for the reader to finish
        print("Sending {0} numbers to Queue() took {1} seconds".format(count, 
            (time.time() - _start)))
    '''

if __name__=='__main__':
    multiprocessing.set_start_method('spawn', force=True)

    run()

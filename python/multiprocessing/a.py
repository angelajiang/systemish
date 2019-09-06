import multiprocessing
from multiprocessing import Process, Queue
import time
import sys

class QueueElement:
    def __init__(self, image_ids):
        self.image_ids = image_ids

def writer(array):
    ## Write to the queue
    count = 0
    while True:
        count += 1
        array[0] = count

class AsyncThing:
    def __init__(self):
        self.array = multiprocessing.Array('i', 1, lock=True)

    def run(self):
        writer_p = Process(target=writer, args=((self.array),))
        writer_p.daemon = True
        writer_p.start()
        while True:
            time.sleep(1)
            print(self.array[0])

def run():
    async_thing = AsyncThing()
    async_thing.run()

if __name__=='__main__':
    multiprocessing.set_start_method('spawn', force=True)

    run()

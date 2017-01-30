#!/usr/bin/env python

"""
Custom queue definitions.
"""

from queue import Queue

__author__ = 'Yun Hua'
__email__ = 'huayunflys@126.com'
__url__ = 'https://github.com/huayunfly/goiot'
__license__ = 'Apache License, Version 2.0'
__version__ = '0.1'
__status__ = 'Beta'


class ClosableQueue(Queue):
    """
    A queue subclass telling the worker thread when it should stop processing.
    Idea from Effective Python book.
    """
    SENTINEL = object()

    def __init__(self, maxsize):
        super().__init__(maxsize=maxsize)

    def close(self):
        self.put(self.SENTINEL)

    def __iter__(self):
        while True:
            item = self.get()
            try:
                if item is self.SENTINEL:
                    return  # Cause the thread to exit
                yield item
            finally:
                self.task_done()


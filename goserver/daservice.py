#!/use/bin/env python

"""
Data access local service definition. It has two cache. The main cache is a
fixed hash table. The secondary cache' data is refreshed by the drivers. A
separated thread synchronize the 2nd and the main cache.

While the clients visit the service. The requests are queued and update the
main cache or 'write' to the driver.
"""

import time
import threading
from goserver import hashtable

__author__ = 'Yun Hua'
__email__ = 'huayunflys@126.com'
__url__ = 'https://github.com/huayunfly/goiot'
__license__ = 'Apache License, Version 2.0'
__version__ = '0.1'
__status__ = 'Beta'


class DAService(object):
    """
    Data access service definition.
    """
    def __init__(self, entry_size):
        """

        Args:
            entry_size: maximum entry size.
        """
        assert entry_size > 0
        if entry_size <= 0:
            raise ValueError('entry_size less than or equal to 0')
        self.entry_size = entry_size
        self.update_threads = []
        self.update_interval = 0.5
        self.keep_updating = True
        self.main = hashtable.FixedDict(entry_size)
        self.secondary = [hashtable.TagValue(tag_id=i) for i in range(self.main.reserved_size)]

    def update_main_from_secondary(self):
        """
        Routine for update the main cache from the secondary cache.
        If a main entry is 'empty' or 'dummy', the operation jumps to the next
        entry update.

        """
        while self.keep_updating:
            for i in range(self.entry_size):
                if (self.main.slots[i].name is not None) or \
                        (self.main.slots[i].name != hashtable.GO_DUMMY_KEY):
                    self.main.slots[i].prim_value = self.secondary[i].value
                    self.main.slots[i].time = time.time()
            time.sleep(self.update_interval)
            print('update...')

    def run(self):
        """
        Service running routine

        """
        self.keep_updating = True
        thread = threading.Thread(target=self.update_main_from_secondary)
        self.update_threads.append(thread)
        thread.start()
        # TODO: do something here
        for thread in self.update_threads:
            thread.join()

    def stop(self):
        """
        Stop the running.

        """
        self.keep_updating = False

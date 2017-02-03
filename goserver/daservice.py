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
        # Size and caches
        self.entry_size = entry_size
        self.main = hashtable.FixedDict(entry_size)
        self.secondary = [hashtable.TagValue(tag_id=i) for i in range(self.main.reserved_size)]
        # Threads
        self.update_threads = []
        self.update_interval = 0.1
        self.keep_updating = True
        self.status_lock = threading.Lock()
        # Drivers
        self.devices = []

    def update_main_from_secondary(self):
        """
        Routine for update the main cache from the secondary cache.
        If a main entry is 'empty' or 'dummy', the operation jumps to the next
        entry update.

        """
        while self.keep_updating:
            if __debug__:
                print('update...')
            for i in range(self.entry_size):
                if (self.main.slots[i].name is not None) or \
                        (self.main.slots[i].name != hashtable.GO_DUMMY_KEY):
                    self.main.slots[i].prim_value = self.secondary[i].value
                    self.main.slots[i].time = time.time()
            time.sleep(self.update_interval)

    def full(self):
        """
        Whether the cache is full.
        Returns:
            True or False.

        """
        return self.main.full()

    def register_device(self, device):
        """
        Register a driver.
        Args:
            device: Device or virtual driver

        Raises:
            ValueError

        """
        assert device is not None
        if device is None:
            raise ValueError('None driver object')
        if device in self.devices:
            raise ValueError('Driver existed')
        with self.status_lock:
            self.devices.append(device)

    def unregister_device(self, device_name):
        """
        Un-register a device
        Args:
            device_name: Name of the device

        Raises:
            AttributeError

        """
        assert device_name is not None
        if device_name is None:
            raise ValueError('Driver name is None.')
        with self.status_lock:
            for device in self.devices:
                if device_name == device.name:
                    self.devices.remove(device)
                    # device.stop()
                    break

    def add_tag(self, name):
        """
        Add a tag by name.
        Args:
            name: Tag name in string.

        Raises:
            ValueError.
        Returns:
            tag id

        """
        assert name is not None
        # TODO: thread safe
        try:
            tag_entry = self.main.add_item(name)
        except ValueError as e:
            raise ValueError('Name error') from e
        except hashtable.RepetitionKeyError as e:
            raise ValueError('Name existed') from e
        except hashtable.OutRangeError as e:
            raise ValueError('Out of range') from e
        else:
            return tag_entry.tag_id

    def remove_tag(self, name):
        """
        Remove tag.
        Args:
            name: Tag name

        """
        assert name is not None
        self.main.remove_item(name)

    def refresh(self, tag_ids, values, op_results):
        """
        Refresh the value of the secondary cache.
        Args:
            tag_ids: Tag id list.
            values: Tag value list.
            op_results: Operation result in list.

        """
        assert isinstance(tag_ids, list)
        assert isinstance(values, list)
        assert isinstance(op_results, list)
        # Todo: thread safe
        for tag_id, value in zip(tag_ids, values):
            self.secondary[tag_id].value = value
        if __debug__:
            print('Service refreshing... tag_ids={0}, values={1}'.format(tag_ids, values))

    def run(self):
        """
        Service running routine

        """
        self.keep_updating = True
        thread = threading.Thread(target=self.update_main_from_secondary)
        self.update_threads.append(thread)
        thread.start()
        # devices
        with self.status_lock:
            for device in self.devices:
                device.start()
        if __debug__:
            print('Service running...')

    def stop(self):
        """
        Stop the running.

        """
        self.keep_updating = False
        for thread in self.update_threads:
            thread.join()
        # devices
        with self.status_lock:
            for device in self.devices:
                device.stop()
        if __debug__:
            print('Service stopped')

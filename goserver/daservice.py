#!/use/bin/env python

"""
Data access local service definition. It has two cache. The main cache is a
fixed hash table. The secondary cache' data is refreshed by the drivers. A
separated thread synchronize the 2nd and the main cache.

While the clients visit the service. The requests are queued and update the
main cache or 'write' to the driver. The replies are also queued and dispatched.

The async flow data includes a transaction id. For the refresh data, a name-callback
function pair may be used. So its transaction id can be hash(name). For the async
read / write flow data, a transaction id is provided by the caller(?)

Service run routine: s.register_device() -> s.run() -> s.run() calls device.run() ->
s.stop() -> s.stop() calls device.stop() -> s.unregister_device(). Thus, hot add/remove
a device is not supported now.
"""

import time
import threading
from goserver import hashtable
from goserver import dataqueue
from goserver.constants import GoOperation, GoStatus
from goserver.common import FlowVar

__author__ = 'Yun Hua'
__email__ = 'huayunflys@126.com'
__url__ = 'https://github.com/huayunfly/goiot'
__license__ = 'Apache License, Version 2.0'
__version__ = '0.1'
__status__ = 'Beta'


class CallerInfo(object):
    """
    Caller information struct including the key(identification) and target tag ids.
    """
    def __init__(self, key, callback_func, tag_ids):
        """
        Init
        Args:
            key: Unique key, which can be a name string or a transaction number.
            callback_func: Call back function
            tag_ids: Tag id list.
        """
        if key is None:
            raise ValueError('key is None.')
        if callback_func is None:
            raise ValueError('callback_func is None')
        if not isinstance(tag_ids, list):
            raise ValueError('tag id list is needed')
        self.key = key
        self.callback = callback_func
        self.tag_ids = tag_ids
        self.hash_code = hash(self.key)

    def __hash__(self):
        return self.hash_code

    def __str__(self):
        return 'Caller_' + str(self.key)


class DAService(object):
    """
    Data access service definition.
    """
    QUEUE_SIZE = 100

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
        self.threads = []
        self.update_interval = 0.1
        self.keep_updating = True
        self.status_lock = threading.Lock()
        self.caller_lock = threading.Lock()
        self.request_queue = dataqueue.ClosableQueue(maxsize=self.QUEUE_SIZE)
        self.reply_queue = dataqueue.ClosableQueue(maxsize=self.QUEUE_SIZE)
        # Devices
        self.devices = []
        # Subscribers
        self.subscribers = []
        # Async read/write callers
        self.callers = []

    def update_main_from_secondary(self):
        """
        Routine for update the main cache from the secondary cache.
        If a main entry is 'empty' or 'dummy', the operation jumps to the next
        entry update.

        """
        while self.keep_updating:
            if __debug__:
                print('update secondary -> main...')
            for i in range(self.main.reserved_size):
                if (self.main.slots[i].name is not None) or \
                        (self.main.slots[i].name != hashtable.GO_DUMMY_KEY):
                    self.main.slots[i].prim_value = self.secondary[i].value
                    self.main.slots[i].time = time.time()
            for caller_info in self.subscribers:
                values = []
                op_results = []
                indexes = caller_info.tag_ids[:]
                for index in indexes:
                    values.append(self.main.slots[index].prim_value)
                    op_results.append(self.main.slots[index].error)

                flow_var = FlowVar(names=None, indexes=indexes, values=values,
                                   op_mode=GoOperation.OP_REFRESH, op_results=op_results,
                                   trans_id=hash(caller_info))
                self.reply_queue.put_nowait(flow_var)
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

    def add_tag(self, name, provider=None):
        """
        Add a tag by name.
        Args:
            name: Tag name in string.
            provider: Tag provider dealing with read/write operation.
                        None is permitted.

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
            tag_entry.provider = provider
            return tag_entry.tag_id

    def remove_tag(self, name):
        """
        Remove tag.
        Args:
            name: Tag name

        """
        # TODO: thread safe
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
        self.threads.append(thread)
        thread = threading.Thread(target=self.request_dispatch_worker)
        self.threads.append(thread)
        thread = threading.Thread(target=self.reply_dispatch_worker)
        self.threads.append(thread)
        for thread in self.threads:
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
        self.request_queue.close()
        self.reply_queue.close()
        for thread in self.threads:
            thread.join()
        # devices
        with self.status_lock:
            for device in self.devices:
                device.stop()
        if __debug__:
            print('Service stopped')

    def subscribe(self, caller_key, callback_func, tag_names):
        """
        Subscribe the service to get refreshing data.

        Args:
            caller_key: Unique caller key for identity.
            callback_func: Function with ([tag_id], [value], [op_result]) signature.
            tag_names: Tag name list subscribed. The names shall contained in
                            the service, or exceptions will be throw. If the list len
                            is zero, nothing done.

        Raises:
            ValueError.

        """
        if caller_key is None:
            raise ValueError('key is None')
        if callback_func is None:
            raise ValueError('Call function is None')
        if not isinstance(tag_names, list):
            raise ValueError('tag_names shall be list')

        if 0 == len(tag_names):
            return
        for caller_info in self.subscribers:
            if caller_info.key == caller_key:
                raise ValueError('Caller subscribed')

        tag_ids = []
        for name in tag_names:
            try:
                item = self.main.find_item(name)
            except ValueError as e:
                raise ValueError('Item does not exist.') from e
            else:
                tag_ids.append(item.tag_id)
        with self.status_lock:
            self.subscribers.append(CallerInfo(caller_key, callback_func, tag_ids))

    def unsubscribe(self, caller_key):
        """
        Unsubscribe a client from the service.
        Args:
            caller_key: Caller key.

        """
        with self.status_lock:
            for caller_info in self.subscribers:
                if caller_info.key == caller_key:
                    self.subscribers.remove(caller_info)

    def reply_dispatch_worker(self):
        """
        Reply queue dispatch worker.

        """
        for flow_var in self.reply_queue:
            if flow_var.op_mode == GoOperation.OP_REFRESH:
                for caller_info in self.subscribers:
                    if hash(caller_info) == flow_var.trans_id:
                        caller_info.callback(flow_var.indexes,
                                             flow_var.values, flow_var.op_results)
            elif flow_var.op_mode == GoOperation.OP_ASYNC_WRITE:
                for caller_info in self.callers:
                    if hash(caller_info) == flow_var.trans_id:
                        for tag_id in flow_var.indexes:
                            caller_info.tag_ids.remove(tag_id)
                        caller_info.callback(flow_var.indexes,
                                             flow_var.op_results, caller_info.key)
                        # Remove async write caller after all data results return
                        if not caller_info.tag_ids:
                            with self.caller_lock:
                                self.callers.remove(caller_info)
            elif flow_var.op_mode == GoOperation.OP_ASYNC_READ:
                for caller_info in self.callers:
                    if hash(caller_info) == flow_var.trans_id:
                        for tag_id in flow_var.indexes:
                            caller_info.tag_ids.remove(tag_id)
                        caller_info.callback(flow_var.indexes,
                                             flow_var.values, flow_var.op_results, caller_info.key)
                        # Remove async caller
                        if not caller_info.tag_ids:
                            with self.caller_lock:
                                self.callers.remove(caller_info)
            else:
                raise ValueError('Invalid operation type.')

    def request_dispatch_worker(self):
        """
        Request queue dispatch worker.

        """
        for flow_var in self.request_queue:
            if GoOperation.OP_ASYNC_READ == flow_var.op_mode:
                values = []
                op_results = []
                for tag_id in flow_var.indexes:
                    values.append(self.main.slots[tag_id].prim_value)
                    op_results.append(self.main.slots[tag_id].error)
                flow_var.values = values
                flow_var.op_results = op_results
                self.reply_queue.put(flow_var)
            elif GoOperation.OP_ASYNC_WRITE == flow_var.op_mode:
                var_grp = {}
                none_provider_indexes = []
                for var_index, value in zip(flow_var.indexes, flow_var.values):
                    provider = self.main.slots[var_index].provider
                    if provider is None:
                        none_provider_indexes.append(var_index)
                        continue
                    if provider in var_grp:
                        var_grp[provider][0].append(var_index)
                        var_grp[provider][1].append(value)
                    else:
                        var_grp[provider] = ([var_index], [value])
                if none_provider_indexes:
                    op_results = [GoStatus.S_INVALID_PROVIDER] * len(none_provider_indexes)
                    none_flow_var = FlowVar(names=None, indexes=none_provider_indexes,
                                            values=None, op_mode=GoOperation.OP_ASYNC_WRITE,
                                            op_results=op_results, trans_id=flow_var.trans_id)
                    self.reply_queue.put(none_flow_var)
                for key, content in var_grp.items():
                    key.async_write([], content[0], content[1], flow_var.trans_id)
            else:
                raise ValueError('Unexpected operation mode')

    def async_read_by_id(self, tag_ids, trans_id, callback_func):
        """
        Async read date by the tag id.

        Args:
            tag_ids: Tag id list. If the list is empty, nothing done.
            trans_id: Unique transaction id.
            callback_func: Call back function.

        Raises:
            ValueError.

        """
        if not isinstance(tag_ids, list):
            raise ValueError('tag_id is not list.')
        if (trans_id is None) or (callback_func is None):
            raise ValueError('trans_id or callback is None.')

        if not tag_ids:
            return
        for caller_info in self.callers:
            if caller_info.key == trans_id:
                raise ValueError('Caller existed.')

        caller_info = CallerInfo(key=trans_id, callback_func=callback_func, tag_ids=tag_ids)
        with self.caller_lock:
            self.callers.append(caller_info)
        tag_ids_copy = tag_ids[:]
        flow_var = FlowVar(names=None, indexes=tag_ids_copy, values=None,
                           op_mode=GoOperation.OP_ASYNC_READ, op_results=None, trans_id=hash(caller_info))
        self.request_queue.put(flow_var)

    def async_read(self, tag_names, trans_id, callback_func):
        """
        Async read data by the tag name. It lookups tag id by the tag name and
        call async_read_by_id()
        Args:
            tag_names: Tag name list. If the list len is zero, nothing done.
            trans_id: Transaction id. None or repetitive value will throw ValueError.
            callback_func: Callback function. None value will throw ValueError.

        Raises:
            ValueError.

        """
        if not isinstance(tag_names, list):
            raise ValueError('tag_names is not list.')
        if (trans_id is None) or (callback_func is None):
            raise ValueError('trans_id or callback is None.')
        if not tag_names:
            return

        tag_ids = []
        for name in tag_names:
            try:
                item = self.main.find_item(name)
            except ValueError as e:
                raise ValueError('Item does not exist.') from e
            else:
                tag_ids.append(item.tag_id)
        self.async_read_by_id(tag_ids, trans_id, callback_func)

    def async_write(self, tag_names, tag_values, trans_id, callback_func):
        """
        Async write data by the tag name. It lookup tag id by the tag name and
         puts the tags into the request queue.

        Args:
            tag_names: Tag name list. If the list len is zero, nothing done.
            tag_values: Tag values list matching names.
            trans_id: Transaction id. None or repetitive value will throw ValueError.
            callback_func: Callback function. None value will throw ValueError.

        Returns:
            ValueError.

        """
        if not isinstance(tag_names, list):
            raise ValueError('tag_names is not list')
        if not isinstance(tag_values, list):
            raise ValueError('tag_values is not list')
        if (trans_id is None) or (callback_func is None):
            raise ValueError('trans_id or callback_func is None')
        if not tag_names:
            return

        tag_ids = []
        for name in tag_names:
            try:
                item = self.main.find_item(name)
            except ValueError as e:
                raise ValueError('Item does not exits.') from e
            else:
                tag_ids.append(item.tag_id)
        self.async_write_by_id(tag_ids, tag_values, trans_id, callback_func)

    def async_write_by_id(self, tag_ids, tag_values, trans_id, callback_func):
        """
        Async write tags by the tag id.

        Args:
            tag_ids: Tag id in list. If it is empty, nothing done.
            tag_values: Tag value in list.
            trans_id: Unique transaction id.
            callback_func: Callback function.

        Raises:
            ValueError

        Returns:

        """
        if not isinstance(tag_ids, list):
            raise ValueError('tag_ids is not list')
        if not isinstance(tag_values, list):
            raise ValueError('tag_values is not list')
        if trans_id is None or callback_func is None:
            raise ValueError('trans_id or callback_func is None')
        if not tag_ids:
            return

        for caller_info in self.callers:
            if caller_info.key == trans_id:
                raise ValueError('Caller existed.')

        caller_info = CallerInfo(key=trans_id, callback_func=callback_func, tag_ids=tag_ids)
        with self.caller_lock:
            self.callers.append(caller_info)
        tag_ids_copy = tag_ids[:]
        flow_var = FlowVar(names=None, indexes=tag_ids_copy, values=tag_values,
                           op_mode=GoOperation.OP_ASYNC_WRITE, op_results=None, trans_id=hash(caller_info))
        self.request_queue.put(flow_var)

    def write_completed(self, tag_ids, op_results, trans_id):
        """
        Notify the service by devices putting the tag data back.
        Push the tag results into the reply queue.

        Args:
            tag_ids: Tag id list.
            op_results: Tag operation result list.
            trans_id: Transaction id.

        """

        if not isinstance(tag_ids, list):
            raise ValueError('tag_ids is not a list.')
        if not isinstance(op_results, list):
            raise ValueError('op_results is not a list.')

        flow_var = FlowVar(names=None, indexes=tag_ids, values=None,
                           op_mode=GoOperation.OP_ASYNC_WRITE, op_results=op_results, trans_id=trans_id)
        self.reply_queue.put(flow_var)

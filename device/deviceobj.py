#!/use/bin/env python

"""
Device base and derived definitions.

read/write request -> in_queue -> async read/write
            reply <- out_queue <- async read/write reply, refresh reply

sync read/write() not implemented.
"""

from threading import Thread
import time
from goserver.dataqueue import ClosableQueue
from goserver.constants import GoOperation, GoStatus
from goserver.common import FlowVar

__author__ = 'Yun Hua'
__email__ = 'huayunflys@126.com'
__url__ = 'https://github.com/huayunfly/goiot'
__license__ = 'Apache License, Version 2.0'
__version__ = '0.1'
__status__ = 'Beta'

GO_DEVICE_Q_MAXSIZE = 100


class DeviceVar(object):
    """
    Device variable definition.
    """
    def __init__(self, name, tag_id=-1):
        self.name = name
        self.tag_id = tag_id


class DeviceBase(object):
    """
    Base device definition.
    """
    def __init__(self, name, service):
        """

        Args:
            name: Device name.
            service: Data access service.
        """
        assert name is not None
        assert service is not None
        if name is None:
            raise ValueError('Name is None.')
        if service is None:
            raise ValueError('Service is None')

        self.name = name
        self.service = service
        self.vars = []
        self.in_queue = ClosableQueue(maxsize=GO_DEVICE_Q_MAXSIZE)
        self.out_queue = ClosableQueue(maxsize=GO_DEVICE_Q_MAXSIZE)
        self.refreshing = True
        self.threads = []

    def start(self):
        """
        Device starts to work. Device registers variables to service, starts to
        refresh the data via the variables.

        """
        for var in self.vars:
            tag_id = self.service.add_tag(var.name, self)
            var.tag_id = tag_id

        self.refreshing = True
        thread = Thread(target=self.refresh_worker)
        self.threads.append(thread)
        thread = Thread(target=self.request_dispatch_worker)
        self.threads.append(thread)
        thread = Thread(target=self.reply_dispatch_worker)
        self.threads.append(thread)
        for thread in self.threads:
            thread.start()

    def stop(self):
        """
        Device stops to work. Device stops refresh data to service, then
        un-registers variables from service.

        """
        self.refreshing = False
        self.in_queue.close()
        self.out_queue.close()
        for thread in self.threads:
            thread.join()

        for var in self.vars:
            self.service.remove_tag(var.name)
            var.tag_id = -1

    def refresh_worker(self):
        """
        Worker refreshing data into out_queue, which move to the device client.

        """
        while self.refreshing:
            time.sleep(0.05)

    def reply_dispatch_worker(self):
        """
        Worker dispatching reply data of out_queue to the device client.

        """
        for flow_var in self.out_queue:
            tag_ids = []
            for index in flow_var.indexes:
                tag_ids.append(self.vars[index].tag_id)
            if GoOperation.OP_REFRESH == flow_var.op_mode:
                self.service.refresh(tag_ids, flow_var.values, flow_var.op_results)

            elif GoOperation.OP_ASYNC_READ == flow_var.op_mode:
                self.service.read_completed(tag_ids, flow_var.values,
                                            flow_var.op_results, flow_var.trans_id)

            elif GoOperation.OP_ASYNC_WRITE == flow_var.op_mode:
                self.service.write_completed(tag_ids, flow_var.op_results, flow_var.trans_id)

            else:
                raise ValueError('GoOperation is not supported.')

    def request_dispatch_worker(self):
        """
        Worker dealing with the request: read, write, async_read, async_write.

        """
        for flow_var in self.in_queue:
            if GoOperation.OP_ASYNC_READ == flow_var.op_mode:
                self.internal_read(flow_var)

            elif GoOperation.OP_ASYNC_WRITE == flow_var.op_mode:
                self.internal_write(flow_var)

            else:
                raise ValueError('GoOperation is not supported.')

    def read(self, var_names, var_ids):
        """
        Read data by the variable name or id.
        Args:
            var_names: Variable name list.
            var_ids: Variable id list in integer.

        Returns:
            Variable data.

        """
        raise NotImplementedError

    def async_read(self, var_names, var_ids, trans_id):
        """
        Async read data by the variable name.
        Args:
            var_names: Variable name list.
            var_ids: Variable id list in integer.
            trans_id: Transaction id.

        Returns:
            Status code.

        """
        if (not isinstance(var_names, list)) or (not isinstance(var_ids, list)):
            raise ValueError('Names and ids need to be list.')

        indexes = []
        for var_id in var_ids:
            for index, item in enumerate(self.vars):
                if item.tag_id == var_id:
                    indexes.append(index)
                    break
        if not indexes:
            return
        flow_var = FlowVar(names=var_names, indexes=indexes, values=None,
                           op_mode=GoOperation.OP_ASYNC_READ, op_results=[], trans_id=trans_id)
        self.in_queue.put(flow_var)
        # TODO: return something?

    def write(self, var_names, var_ids, values):
        """
            Write data by the variable name or id.
            Args:
                var_names: Variable name list in string.
                var_ids: Variable id list in integer.
                values: Variable data list.
            Raises:
                ValueError.

            """
        raise NotImplementedError

    def async_write(self, var_names, var_ids, values, trans_id):
        """
        Async read data by the variable name.
        Args:
            var_names: Variable name list.
            var_ids: Variable id list in integer.
            values: Variable data.
            trans_id: Transaction id.

        Returns:
            Status code.

        """
        if (not isinstance(var_names, list)) or (not isinstance(var_ids, list)) \
                or (not isinstance(values, list)):
            raise ValueError('Names, ids, values need to be list.')

        indexes = []
        for var_id in var_ids:
            for index, item in enumerate(self.vars):
                if item.tag_id == var_id:
                    indexes.append(index)
                    break
        if not indexes:
            return
        flow_var = FlowVar(names=var_names, indexes=indexes, values=values,
                           op_mode=GoOperation.OP_ASYNC_WRITE, op_results=[], trans_id=trans_id)
        self.in_queue.put(flow_var)

    def internal_read(self, flow_var):
        """
        Internal implementation for read.
        Args:
            flow_var: Flow variable.

        Returns: read variables.

        """
        raise NotImplementedError

    def internal_write(self, flow_var):
        """
        Internal implementation for write.
        Args:
            flow_var: Flow variable

        Returns: write results.

        """
        raise NotImplementedError

    def query_registered_var_ids(self, var_ids):
        """
        Query the registered variable ids.
        Args:
            var_ids: Variable id list query

        Returns:
            Registered variable id list.

        """
        registered_var_ids = []
        for var_id in var_ids:
            for var in self.vars:
                if var_id == var.tag_id:
                    registered_var_ids.append(var_id)
                    break
        return registered_var_ids


class MockupDevice(DeviceBase):
    """
    Mockup device with auto generated variables
    """
    def __init__(self, name, service):
        super().__init__(name, service)

        for i in range(6):
            name = 'Sim_' + str(i)
            var = DeviceVar(name=name)
            self.vars.append(var)

        self.sim_5 = 1.11

    def refresh_worker(self):
        """
        Overrides the base method.

        """
        names = []
        indexes = []
        op_results = []
        for i, var in enumerate(self.vars):
            names.append(var.name)
            indexes.append(i)
            op_results.append(GoStatus.S_OK)

        inc = 1
        while self.refreshing:
            values = list()
            values.append(str(inc))
            if (inc // 10) % 2:
                values.append(True)
            else:
                values.append(False)
            values.append(inc * 1.0)
            values.append(inc / 2.0)
            values.append(None)
            values.append(self.sim_5)

            flow_var = FlowVar(names, indexes, values,
                               GoOperation.OP_REFRESH, op_results, -1)
            self.out_queue.put(flow_var)
            inc += 1
            if inc > 10000:
                inc = 1
            time.sleep(0.1)

    def internal_write(self, flow_var):
        results = []
        for index, value in zip(flow_var.indexes, flow_var.values):
            if 5 == index:
                self.sim_5 = value
                results.append(GoStatus.S_OK)
            else:
                results.apppend(GoStatus.S_NO_IMPLEMENTED)
        flow_var.values = None
        flow_var.op_results = results
        flow_var.op_mode = GoOperation.OP_ASYNC_WRITE
        self.out_queue.put(flow_var)



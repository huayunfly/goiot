#!/use/bin/env python

"""
Device base and derived definitions
"""

from threading import Thread
from goserver.dataqueue import ClosableQueue

__author__ = 'Yun Hua'
__email__ = 'huayunflys@126.com'
__url__ = 'https://github.com/huayunfly/goiot'
__license__ = 'Apache License, Version 2.0'
__version__ = '0.1'
__status__ = 'Beta'

GO_DEVICE_Q_MAXSIZE = 100


class FlowVar(object):
    """
    Variable definition for the data flowing in the device.
    """
    def __init__(self, name, index, value):
        """
        Initialize the variable object.
        Args:
            name: Variable name.
            index: Variable index in the parent device.
            value: Variable value.
        """
        self.name = name
        self.index = index
        self.value = value


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
        self.threads = []

    def start(self):
        """
        Device starts to work. Device registers variables to service, starts to
        refresh the data via the variables.

        """
        for var in self.vars:
            tag_entry = self.service.add_tag(var.name)
            var.tag_id = tag_entry.tag_id
        thread = Thread(target=self.refresh_service)
        self.threads.append(thread)
        thread.start()

    def stop(self):
        """
        Device stops to work. Device stops refresh data to service, then
        un-registers variables from service.

        """
        self.in_queue.close()
        for thread in self.threads:
            thread.join()
        for var in self.vars:
            self.service.remove_tag(var.name)
            var.tag_id = -1

    def refresh_service(self):
        """
        Refreshing data to the service worker

        """
        for flow_var in self.in_queue:
            self.service.refresh(self.vars[flow_var.index].tag_id, flow_var.value)


    def generate_data(self):
        """
        Generating or acquiring data worker.

        """
        pass

    def read_by_name(self, var_name):
        """
        Read data by the variable name.
        Args:
            var_name: Variable name.

        Returns:
            Variable data.

        """
        raise NotImplementedError

    def read_by_id(self, var_id):
        """
        Read data by the variable.
        Args:
            var_id: Variable id in integer.

        Returns:
            Variable data.
        """
        raise NotImplementedError

    def write_by_name(self, var_name, value):
        """
            Write data by the variable name.
            Args:
                var_name: Variable name in string.
                value: Variable data.
            Raises:
                ValueError.

            """
        raise NotImplementedError

    def write_by_id(self, var_id, value):
        """
            Write data by the variable id.
            Args:
                var_id: Variable id in integer.
                value: Variable data.
            Raises:
                ValueError.

            """
        raise NotImplementedError


class MockupDevice(DeviceBase):
    """
    Mockup device with auto generated variables
    """
    def __init__(self, name, service):
        super().__init__(name, service)

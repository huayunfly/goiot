#!/usr/bin/env python
"""
Driver base class and the derived definition for GoIoT server.
A Modbus RTU via RS-485 driver is implemented.
"""

__author__ = 'Yun Hua'
__email__ = 'huayunflys@126.com'
__url__ = 'https://github.com/huayunfly/repo'
__license__ = 'Apache License, Version 2.0'
__version__ = '0.1'
__status__ = 'Beta'


class DriverBase(object):
    """
    The driver base acting as a file object
    """
    def __init__(self, protocol_path):
        self.protocol_path = protocol_path

    @classmethod
    def open(cls):
        """
        Open the driver object
        Returns: the driver handler executing read / write
        """
        raise NotImplementedError('close method')

    @classmethod
    def close(cls):
        """
        Close the driver object and release the resources
        """
        raise NotImplementedError('close method')

    def read_tags(self, size, tag_list):
        """
        Read the driver data
        Args:
            size: tag size
            tag_list: tag list
        Returns: Read tags operation 's values, qualities, results and timestamps

        """
        raise NotImplementedError('read_tags method')

    def write_tags(self, size, tag_list, values):
        """
        Write tags into the (hardware) devices
        Args:
            size: tag size
            tag_list: tag list
            values:

        Returns: Write tags operation 's results
        """
        raise NotImplementedError('write_tags method')



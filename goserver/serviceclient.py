#!/usr/bin/env python
"""
Client consuming the service.
"""

__author__ = 'Yun Hua'
__email__ = 'huayunflys@126.com'
__url__ = 'https://github.com/huayunfly/goiot'
__license__ = 'Apache License, Version 2.0'
__version__ = '0.1'
__status__ = 'Beta'


class ServiceClient(object):
    """
    Service client base.
    """
    def __init__(self, name, service=None):
        self.name = name
        self.service = service

    def __str__(self):
        return self.name

    def data_changed(self, tag_ids, values, op_results):
        """
        On data change callback.

        Args:
            tag_ids: Tag id in list.
            values: Value in list.
            op_results: Operation result in list.

        Returns:

        """
        if __debug__:
            print('data changed... tag_id:{0}, value:{1}'.format(tag_ids, values))

    def read_completed(self, tag_ids, values, op_results, trans_id):
        """
        On read completed callback.
        Args:
            tag_ids: Tag id in list.
            values: Value in list.
            op_results: Operation result in list.
            trans_id: Transaction id

        Returns:

        """
        if __debug__:
            print('read completed... tag_id:{0}, value:{1}'.format(tag_ids, values))

    def write_completed(self, tag_ids, op_results, trans_id):
        """
        On write completed callback.

        Args:
            tag_ids: Tag id in list.
            op_results: Operation result in list.
            trans_id: Transaction id

        """
        if __debug__:
            print('write completed... tag_id:{0}, value:{1}'.format(tag_ids, op_results))

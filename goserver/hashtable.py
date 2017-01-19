#!/usr/bin/env python
"""
Hash table for the key, value pair storage.
The hash() and position probing algorithms came from Python dict's C implementation.
Refer to "http://svn.python.org/projects/python/trunk/Objects/dictobject.c"

However, Python dict insert manipulation does not return the slot index. The dict is
sizable automatically. It is difficult to track a key-value pair's index.

In some scenarios, a fixed length hash table is useful, which is non-sizable. Thus,
tracking the table slot index is meaningful for we can access the key-value pair via
the index.

"""

import time


__author__ = 'Yun Hua'
__email__ = 'huayunflys@126.com'
__url__ = 'https://github.com/huayunfly/goiot'
__license__ = 'Apache License, Version 2.0'
__version__ = '0.1'
__status__ = 'Beta'


GO_QUALITY_NORMAL = 0x00
GO_RESULT_OK = 0x00
GO_RESULT_ERROR = 0x01
GO_READABLE = 0x00
GO_WRITEABLE = 0x00


class TagState(object):
    """
    Tag state definition.
    """
    def __init__(self, timestamp=None, error_result=GO_RESULT_OK, quality=GO_QUALITY_NORMAL):
        """

        Args:
            timestamp: use time.time() to assign self.time if the variable is None.
            error_result: manipulation result.
            quality: tag quality. It is reserved for the future usage.
        """
        if timestamp is None:
            self.time = time.time()
        else:
            self.time = timestamp
        self.error = error_result
        self.quality = quality


class TagValue(object):
    """
    Tag value definition.
    """
    def __init__(self, value=None, tag_id=0, tag_state=None):
        """

        Args:
            value: assign the value.
            tag_id: tag id in integer.
            tag_state: assign an empty TagState if tag_state is None.
        """
        self.value = value
        self.tag_id = tag_id
        if tag_state is None:
            self.tag_state = TagState()
        else:
            self.tag_state = tag_state


class TagAttr(object):
    """
    Tag attribute definition.
    """
    def __init__(self, name='', tag_id=0, rights=GO_READABLE):
        """

        Args:
            name: tag name
            tag_id: tag id in integer.
            right: indicate read or write rights
        """
        self.name = name
        self.tag_id = tag_id
        self.rights = rights


class FixedLenHashTable(object):
    """
    The hash table implementation for the fixed length and non-sizable.
    Every insert manipulation will return the table slot index.
    """
    def __init__(self, size):
        pass

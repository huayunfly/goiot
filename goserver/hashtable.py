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
GO_RESULT_NULL_NAME = 0x02
GO_READABLE = 0x01
GO_WRITEABLE = 0x10
GO_EU_TYPE_ANALOG = 0x1


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


class EUnit(object):
    """
    Engineering unit definition.
    """
    def __init__(self, range_max=None, range_min=None, deadband=None):
        """

        Args:
            range_max: ANALOG variable's range max value. NO analog value is None.
            range_min: ANALOG variable's range min value. NO analog value is None.
            deadband: Deadband. NO analog value is None.
        """
        self.range_max = range_max
        self.range_min = range_min
        self.deadband = deadband


class TagAttr(EUnit):
    """
    Tag attribute definition.
    """
    def __init__(self, name=None, tag_id=0, rights=GO_READABLE,
                 range_max=None, range_min=None, deadband=None):
        """

        Args:
            name: tag name
            tag_id: tag id in integer.
            rights: indicate read or write rights
        """
        self.name = name
        self.tag_id = tag_id
        self.rights = rights
        super(TagAttr, self).__init__(range_max=range_max, range_min=range_min, deadband=deadband)


class TagEntry(TagAttr, TagState):
    """
    Tag entry definition.
    """
    def __init__(self, prim_value, active=True):
        """

        Args:
            prim_value: primary value.
            active: whether the entry is active.
        """
        super(TagEntry, self).__init__()
        self.prim_value = prim_value
        self.active = active


class FixedLenHashTable(object):
    """
    The hash table implementation for the fixed length and non-sizable.
    Every insert manipulation will return the table slot index.
    """
    def __init__(self, size):
        assert(isinstance(size, int))
        self.total = size + (size >> 1)
        if self.total & 0x1:
            self.total += 1
        self.slots = [None] * self.total

    def add_item(self, name, value):
        """
        Add a new item.
        Args:
            name: item name
            value: item value
        Raises:
            ValueError

        Returns: operation result, slot id in integer

        """
        assert(name is not None)
        if name is None:
            raise ValueError
        perturb = hash(name)
        j = perturb
        # collison resolution
        if j < self.total and self.slots[j] is None:
            entry = TagEntry(value)
            entry.name = name
            self.slots[j] = entry



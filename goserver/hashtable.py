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
import sys


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

GO_DUMMY_KEY = '<dummy_key>'


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
    def __init__(self, name=None, rights=GO_READABLE,
                 range_max=None, range_min=None, deadband=None):
        """

        Args:
            name: tag name
            rights: indicate read or write rights
        """
        self.name = name
        self.tag_hash = 0
        self.rights = rights
        super(TagAttr, self).__init__(range_max=range_max, range_min=range_min, deadband=deadband)


class TagEntry(TagAttr, TagState):
    """
    Tag entry definition.
    """
    def __init__(self, tag_id=0, prim_value=None, active=True):
        """

        Args:
            tag_id: unique id in integer
            prim_value: primary value.
            active: whether the entry is active.
        """
        super(TagEntry, self).__init__()
        self.tag_id = tag_id
        self.prim_value = prim_value
        self.active = active

    def __str__(self):
        return str(self.name) + '.id_' + str(self.tag_id)


class OutRangeError(LookupError):
    """
    Operation out of range.
    """
    pass


class RepetitionKeyError(LookupError):
    """
    Repetition key error.
    """
    pass


class FixedDict(object):
    """
    The hash table implementation for the fixed length and non-sizable dictionary.
    Every insert manipulation will return the table slot index.
    """
    def __init__(self, size):
        """
        Use a fixed size to initialize the hash table.
        However, the table allocates (3 / 2) * size slots for better hash collision
        resolution performance.
        Args:
            size: fixed hash table size.
        """
        assert(isinstance(size, int))
        if size < 1:
            raise ValueError
        self.size = size
        # Reserved size is at least larger than 3/2 size
        reserved = size + (size >> 1)
        self.reserved_size = 1
        # Force reserved size equal 2**x
        while reserved:
            self.reserved_size <<= 1
            reserved >>= 1
        self.mask = self.reserved_size - 1
        # self.filled (including dummy item) >= self.used
        self.filled = 0
        self.used = 0
        self.PERTURB_SHIFT = 5
        self.slots = [TagEntry(tag_id=i) for i in range(self.reserved_size)]

    def lookup_by_string(self, key, hash_code):
        """
        Lookup an item with a string type key.
        Args:
            key: item's key which is a string.
            hash_code: item hash value.
        Raises:
            ValueError
            TypeError
            LookupError

        Returns: found item with id attribute in integer.

        """
        assert(key is not None)
        if key is None:
            raise ValueError('Key can not be None.')
        if not isinstance(key, str):
            raise TypeError('Key shall be a string')
        i = hash_code & self.mask
        if self.slots[i].name is None:
            return self.slots[i]
        elif self.slots[i].name == GO_DUMMY_KEY:
            freeslot = self.slots[i]
        elif self.slots[i].tag_hash == hash and self.slots[i].name == key:
            return self.slots[i]
        else:
            freeslot = None

        # collison resolution
        perturb = hash_code & sys.maxsize
        while True:
            i = (i << 2) + i + 1 + perturb
            pos = i & self.mask
            if self.slots[pos].name is None:
                if freeslot is None:
                    return self.slots[pos]
                else:
                    return freeslot
            if (self.slots[pos].tag_hash == hash_code) and \
                    (self.slots[pos].name == key) and (self.slots[pos].name != GO_DUMMY_KEY):
                return self.slot[pos]
            if (self.slots[pos].name == GO_DUMMY_KEY) and (freeslot is None):
                return self.slots[pos]
            perturb >>= self.PERTURB_SHIFT

        assert False
        raise LookupError('Unexpected lookup result.')

    def add_item(self, key):
        """
        Add a new item.
        Args:
            key: item key.

        Raises:
            LookupError.
            OutRangeError.
            ValueError.
        Returns: item

        """
        assert(key is not None)
        if key is None:
            raise ValueError('Key can not be None.')
        if self.used > self.size:
            raise OutRangeError('No free slots available.')

        hash_code = hash(key)
        item = self.lookup_by_string(key, hash_code)
        if item.name is None:
            item.name = key
            item.tag_hash = hash_code
            return item
        elif GO_DUMMY_KEY == item.name:
            item.name = key
            item.tag_hash = hash_code
            return item
        elif key == item.name and hash_code == item.tag_hash:
            raise RepetitionKeyError('Entry with the same key and hash value found')

        raise LookupError('Unexpected: NO dummy entry, No empty entry and No same entry')










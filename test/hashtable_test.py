#!/usr/bin/env python
"""
Hash table test cases.
"""

import unittest
import math
from goserver import hashtable

__author__ = 'Yun Hua'
__email__ = 'huayunflys@126.com'
__url__ = 'https://github.com/huayunfly/goiot'
__license__ = 'Apache License, Version 2.0'
__version__ = '0.1'
__status__ = 'Beta'


class HashTableTest(unittest.TestCase):
    """
    Test cases for the hash table.
    """
    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_tag(self):
        """
        Test tag construction and inheritance.
        """
        entry = hashtable.TagEntry(prim_value=100.0)
        self.assertIn('name', entry.__dict__)
        self.assertIn('range_min', entry.__dict__)
        self.assertIn('range_max', entry.__dict__)
        self.assertIn('rights', entry.__dict__)
        self.assertIn('tag_id', entry.__dict__)
        self.assertIn('active', entry.__dict__)
        self.assertIn('prim_value', entry.__dict__)
        self.assertIn('deadband', entry.__dict__)

    def test_fixed_dict(self):
        """
        Test hash table construction and operation.
        """
        fixed_table = hashtable.FixedDict(101)
        self.assertEqual(fixed_table.reserved_size, 256)
        self.assertIsNone(fixed_table.slots[fixed_table.reserved_size - 1].name)
        # Lookup with type error.
        with self.assertRaises(TypeError):
            _ = fixed_table.lookup_by_string(1, hash(1))
        # Lookup an empty item
        tag_entry = fixed_table.lookup_by_string('PV1', hash('PV1'))
        self.assertIsNone(tag_entry.name)
        self.assertEqual(tag_entry.tag_id, hash('PV1') & fixed_table.mask)
        # Add items
        for i in range(101):
            tag_entry = fixed_table.add_item('{0}.UDC3300'.format(i))
        # Out of range
        with self.assertRaises(hashtable.OutRangeError):
            fixed_table.add_item('{0}.UDC3300'.format(101))
        # Lookup items
        for i in range(101):
            tag_entry = fixed_table.lookup_by_string('{0}.UDC3300'.format(i),
                                                     hash('{0}.UDC3300'.format(i)))
            self.assertEqual(tag_entry.name, '{0}.UDC3300'.format(i))
        # Remove an item
        with self.assertRaises(ValueError):
            fixed_table.remove_item('102.UDC3300')
        # Remove items
        for i in range(101):
            fixed_table.remove_item('{0}.UDC3300'.format(i))
        self.assertEqual(fixed_table.used, 0)
        self.assertEqual(fixed_table.filled, 101)
        # Update items value
        with self.assertRaises(ValueError):
            fixed_table.update_item('102.UDC3300', 1.0)
        for i in range(101):
            _ = fixed_table.add_item('{0}.UDC3300'.format(i))
        for i in range(101):
            fixed_table.update_item('{0}.UDC3300'.format(i), float(i) * math.pi)
            tag_entry = fixed_table.lookup_by_string('{0}.UDC3300'.format(i),
                                                     hash('{0}.UDC3300'.format(i)))
            self.assertAlmostEqual(tag_entry.prim_value, float(i) * math.pi, delta=1e-9)


if __name__ == '__main__':
    unittest.main()


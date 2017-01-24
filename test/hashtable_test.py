#!/usr/bin/env python
"""
Hash table test cases.
"""

import unittest
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

    def test_fixed_hashtable(self):
        """
        Test hash table construction and operation.
        """
        fixed_table = hashtable.FixedLenHashTable(101)
        self.assertEqual(fixed_table.total, 152)
        self.assertIsNone(fixed_table.slots[fixed_table.total - 1].name)

if __name__ == '__main__':
    unittest.main()


#!/usr/bin/env python
"""
Data access service test.
"""

import unittest
import time
from goserver import daservice

__author__ = 'Yun Hua'
__email__ = 'huayunflys@126.com'
__url__ = 'https://github.com/huayunfly/goiot'
__license__ = 'Apache License, Version 2.0'
__version__ = '0.1'
__status__ = 'Beta'


class DAServiceTest(unittest.TestCase):
    """
    Data access service test cases.
    """
    def setUp(self):
        self.service = daservice.DAService(1001)

    def tearDown(self):
        pass

    def test_service_ctor(self):
        """
        Test the service constructs.

        """
        self.assertEqual(len(self.service.main.slots), len(self.service.secondary))

    def test_service_update_main(self):
        """
        Test the routine updating the main cache from the secondary cache.

        """
        pass

    def test_service_run(self):
        """
        Test the service running.

        """
        self.service.run()


if __name__ == '__main__':
    unittest.main()

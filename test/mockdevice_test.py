#!/usr/bin/env python
"""
Mockup device test.
"""

import unittest
import time
from goserver import daservice
from device.deviceobj import MockupDevice

__author__ = 'Yun Hua'
__email__ = 'huayunflys@126.com'
__url__ = 'https://github.com/huayunfly/goiot'
__license__ = 'Apache License, Version 2.0'
__version__ = '0.1'
__status__ = 'Beta'


class MockDeviceTest(unittest.TestCase):
    """
    Test cases for the mockup device.
    """
    def setUp(self):
        self.service = daservice.DAService(10)
        self.device = MockupDevice('Simulation', self.service)
        self.service.register_device(self.device)

    def tearDown(self):
        self.service.unregister_device('Simulation')

    def test_start(self):
        """
        Start device, adding tags, running threads.

        """
        self.device.start()
        time.sleep(1)
        self.device.stop()



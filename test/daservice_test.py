#!/usr/bin/env python
"""
Data access service test.
"""

import unittest
import time
from goserver import daservice
from goserver.constants import GoStatus
from goserver import serviceclient
from device import deviceobj

__author__ = 'Yun Hua'
__email__ = 'huayunflys@126.com'
__url__ = 'https://github.com/huayunfly/goiot'
__license__ = 'Apache License, Version 2.0'
__version__ = '0.1'
__status__ = 'Beta'


def data_changed(tag_ids, values, op_results):
    print('data_changed() called... '
          'tag_id:{0}, value:{1}, result:{2}'.format(tag_ids, values, op_results))


def read_completed(tag_ids, values, op_results, trans_id):
    print('read_completed() called... '
          'tag_id:{0}, value:{1}, result:{2}, trans_id:{3}'.format(tag_ids, values, op_results, trans_id))


def write_completed(tag_ids, op_results, trans_id):
    print('read_completed() called... '
          'tag_id:{0}, result:{1}, trans_id:{2}'.format(tag_ids, op_results, trans_id))


class DAServiceTest(unittest.TestCase):
    """
    Data access service test cases.
    """
    TEST_TAG_1 = 'TestTag_1'
    TEST_TAG_2 = 'TestTag_2'
    TEST_TAG_3 = 'TestTag_3'

    def setUp(self):
        self.service = daservice.DAService(1001)
        self.service.add_tag(self.TEST_TAG_1)
        self.service.add_tag(self.TEST_TAG_2)
        self.service.add_tag(self.TEST_TAG_3)
        self.client = serviceclient.ServiceClient('TestClient')

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

    def test_service_add_tag(self):
        """
        Test adding a tag.

        """
        self.service.add_tag('0.UDC3300')
        with self.assertRaises(ValueError):
            self.service.add_tag('0.UDC3300')

    def test_service_refresh(self):
        """
        Test refresh method.

        """
        self.service.refresh([0], [100.0], [GoStatus.S_OK])

    def test_register_driver(self):
        """
        Test register/unregister driver.

        """
        device = deviceobj.DeviceBase('UDC3300', self.service)
        self.service.register_device(device)
        self.service.unregister_device('UDC3300')

    def test_service_run(self):
        """
        Test the service running.

        """
        print('test_service_run() -------------------------')
        device = deviceobj.DeviceBase('UDC3300', self.service)
        self.service.register_device(device)
        self.service.run()
        time.sleep(1)
        self.service.stop()
        self.service.unregister_device('UDC3300')

    def test_subscribe(self):
        """
        Test subscribing/un-subscribing client.

        """
        print('test_subscribe() -----------------------')
        with self.assertRaises(ValueError):
            self.service.subscribe('client1', self.client.data_changed, ['1.UDC3300'])
        self.service.subscribe('client1', self.client.data_changed, [self.TEST_TAG_1])
        self.service.subscribe('client2', data_changed, [self.TEST_TAG_1])

        self.service.run()
        time.sleep(1)
        self.service.stop()

        self.service.unsubscribe('client1')
        self.service.unsubscribe('client2')
        self.assertEqual(len(self.service.subscribers), 0)

    def test_mockdevice_subscribe(self):
        """
        Test subscribe data refreshed from the mock device.

        """
        print('test_mockdevice_subscribe() ------------------')
        device = deviceobj.MockupDevice('Simulation', self.service)
        self.service.register_device(device)

        self.service.run()
        tag_names = ['Sim_' + str(i) for i in range(5)]
        self.service.subscribe('client3', data_changed, tag_names)
        time.sleep(1)
        self.service.stop()

        self.service.unsubscribe('client3')
        self.service.unregister_device('Simulation')

    def test_async_read(self):
        """
        Test async read.

        """
        print('test_async_read() ------------------')
        self.service.run()
        with self.assertRaises(ValueError) as e:
            self.service.async_read(tag_names=['haha'], trans_id=-1, callback_func=read_completed)
        print(e.exception)

        self.service.async_read(tag_names=[self.TEST_TAG_1, self.TEST_TAG_2, self.TEST_TAG_3],
                                trans_id=1, callback_func=read_completed)
        with self.assertRaises(ValueError) as e:
            self.service.async_read(tag_names=[self.TEST_TAG_1, self.TEST_TAG_2, self.TEST_TAG_3],
                                    trans_id=1, callback_func=read_completed)
        print(e.exception)
        time.sleep(0.01) # Error, if no wait, async_read() is not called.
        self.service.stop()


if __name__ == '__main__':
    unittest.main()

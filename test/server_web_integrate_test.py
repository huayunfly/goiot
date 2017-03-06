#!/usr/bin/env python

"""
Go server and web access integration test.
@date 2017.03.03
"""

import unittest
import requests
import json
import time
import threading
from goserver import daservice
from goserver.constants import GoStatus
from goserver import serviceclient
from device import deviceobj, honeywelludc


REFRESH_RATE = 0.5  # in second
WRITE_RATE = 0.5  # in second


class WebApiClient(serviceclient.ServiceClient):
    """
    Class access Web APIs.
    """

    def __init__(self, name, service):
        assert service is not None
        super().__init__(name, service)
        self.tags_rw_url = 'http://127.0.0.1:8000/da/api/tags'
        self.token = 'abc'
        self.tag_names = ['udc_pv1', 'udc_sp1', 'Sim_0', 'Sim_1', 'Sim_2', 'Sim_3', 'Sim_4', 'Sim_5']
        self.tag_ids = None
        self.keys = [1, 2, 3, 4, 5, 6, 7, 8]
        self.request_keys = [2, 8]  # request API to get data and write to the device
        self.tick = time.time()
        self.keep_working = False
        self.threads = []
        self.rw_lock = threading.Lock()
        self.trans_id = 101
        # self.key = {'data': [{'key': 1}, {'key': 2}, {'key': 3}, {'key': 4}]}
        self.post_tag_ids = []
        self.post_values = []
        self.post_results = []

    def start(self):
        self.tag_ids = self.service.browse_tag_id(self.tag_names)
        subscribe_tag_names = ['udc_pv1', 'Sim_0', 'Sim_1', 'Sim_2', 'Sim_3', 'Sim_4']
        self.service.subscribe(self.name, self.data_changed, subscribe_tag_names)
        self.keep_working = True
        self.threads.append(threading.Thread(target=self.write_worker))
        self.threads.append(threading.Thread(target=self.refresh_worker))
        for thread in self.threads:
            thread.start()

    def stop(self):
        self.keep_working = False
        for thread in self.threads:
            thread.join()
        self.tag_ids = None
        self.service.unsubscribe(self.name)

    def data_changed(self, tag_ids, values, op_results):
        """
        Call back function for the service subscribe.

        """
        with self.rw_lock:
            self.post_tag_ids = tag_ids[:]
            self.post_values = values[:]
            self.post_results = op_results[:]

    def read_completed(self, tag_ids, values, op_results, trans_id):
        raise NotImplementedError

    def write_completed(self, tag_ids, op_results, trans_id):
        pass

    def refresh_worker(self):
        """
        Worker post the refreshing data to the web server.

        """
        while self.keep_working:
            data = {'data': []}
            with self.rw_lock:
                for tag_id, raw_value, op_result in zip(self.post_tag_ids,
                                                        self.post_values, self.post_results):
                    if GoStatus.S_OK != op_result:
                        continue

                    if isinstance(raw_value, float):
                        value = raw_value
                    elif isinstance(raw_value, int):
                        value = float(raw_value)
                    elif isinstance(raw_value, str):
                        value = float(hash(raw_value))
                    elif isinstance(raw_value, bool):
                        value = float(raw_value)
                    elif raw_value is None:
                        value = -27648.0
                    else:
                        assert False
                    key = self.keys[self.tag_ids.index(tag_id)]
                    data['data'].append(dict(key=key, value=value))
            payload = dict(token=self.token, data=json.dumps(data))
            try:
                r = requests.post(self.tags_rw_url, data=payload)
            except requests.exceptions.ConnectionError:
                print('data_change(): connection failed.')
            assert 200 == r.status_code
            time.sleep(REFRESH_RATE)

    def write_worker(self):
        """
        Worker getting the setpoint data from web server and writing to the service.

        """
        while self.keep_working:
            data = {'data': []}
            for key in self.request_keys:
                data['data'].append(dict(key=key))
            payload = dict(token=self.token, key=json.dumps(data))
            try:
                r = requests.get(self.tags_rw_url, params=payload)
            except requests.exceptions.ConnectionError:
                print('write_worker(): connection failed.')
                time.sleep(WRITE_RATE)
                continue
            if 200 != r.status_code:
                assert False
                return
            try:
                data = r.json()
            except TypeError:
                assert False
                return
            except json.decoder.JSONDecodeError:
                assert False
                return
            try:
                data_items = data['data']
                tag_ids = []
                tag_values = []
                for item in data_items:
                    key = item['key']
                    value = item['value']
                    tag_ids.append(self.tag_ids[self.keys.index(key)])
                    tag_values.append(value)
            except KeyError:
                return
            except IndexError:
                return
            else:
                self.service.async_write_by_id(tag_ids, tag_values, self.trans_id, self.write_completed)
                self.trans_id += 1
            time.sleep(WRITE_RATE)


class ServerIntegrateTest(unittest.TestCase):
    """
    Server and web integrate test.
    """

    def setUp(self):
        self.service = daservice.DAService(101)
        self.client = WebApiClient('WebClient', self.service)

    def tearDown(self):
        self.service.stop()
        self.service.unregister_device('Simulation')
        self.service.unregister_device('UDC3300')
        self.client.stop()

    def test_data_exchange(self):
        """
        Loop test for the data interaction between server and web apis.

        """
        device = deviceobj.MockupDevice('Simulation', self.service)
        self.service.register_device(device)
        device = honeywelludc.HoneywellUDC('UDC3300', self.service)
        self.service.register_device(device)
        self.service.run()
        self.client.start()
        time.sleep(30)

if __name__ == '__main__':
    unittest.main()

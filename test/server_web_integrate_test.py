#!/usr/bin/env python

"""
Go server and web access integration test.
"""

import unittest
import requests
import json
import time
import threading
from goserver import daservice
from goserver.constants import GoOperation, GoStatus
from goserver import serviceclient
from device import deviceobj
from goserver.common import FlowVar


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
        self.tag_names = ['Sim_0', 'Sim_1', 'Sim_2', 'Sim_3', 'Sim_4', 'Sim_5']
        self.tag_ids = None
        self.keys = [3, 4, 5, 6, 7, 8]
        self.request_keys = [8]  # request API to get data and write to the device
        self.tick = time.time()
        self.keep_working = False
        self.threads = []
        self.trans_id = 101
        # self.key = {'data': [{'key': 1}, {'key': 2}, {'key': 3}, {'key': 4}]}

    def start(self):
        self.tag_ids = self.service.browse_tag_id(self.tag_names)
        subscribe_tag_names = ['Sim_0', 'Sim_1', 'Sim_2', 'Sim_3', 'Sim_4']
        self.service.subscribe(self.name, self.data_changed, subscribe_tag_names)
        self.keep_working = True
        self.threads.append(threading.Thread(target=self.write_worker))
        for thread in self.threads:
            thread.start()

    def stop(self):
        self.keep_working = False
        for thread in self.threads:
            thread.join()
        self.tag_ids = None
        self.service.unsubscribe(self.name)

    def data_changed(self, tag_ids, values, op_results):
        if time.time() - self.tick < REFRESH_RATE:
            return

        self.tick = time.time()
        data = {'data': []}
        for tag_id, raw_value, op_result in zip(tag_ids, values, op_results):
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
        r = requests.post(self.tags_rw_url, data=payload)
        assert 200 == r.status_code

    def read_completed(self, tag_ids, values, op_results, trans_id):
        raise NotImplementedError

    def write_completed(self, tag_ids, op_results, trans_id):
        pass

    def write_worker(self):
        data = {'data': []}
        for key in self.request_keys:
            data['data'].append(dict(key=key))
        payload = dict(token=self.token, key=json.dumps(data))
        r = requests.get(self.tags_rw_url, params=payload)
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

    def test_data_exchange(self):
        """
        Loop test for the data interaction between server and web apis.

        """
        device = deviceobj.MockupDevice('Simulation', self.service)
        self.service.register_device(device)
        self.service.run()
        self.client.start()
        time.sleep(20)

if __name__ == '__main__':
    unittest.main()

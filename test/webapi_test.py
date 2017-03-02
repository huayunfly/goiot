#!/usr/bin/env python
"""
Web APIs test.
"""

import unittest
import requests
import json

class WebAPITest(unittest.TestCase):
    """
    Read and write APIs test cases.
    """

    def setUp(self):
        self.main_url = 'http://127.0.0.1:8000'
        self.tags_rw_url = 'http://127.0.0.1:8000/da/api/tags'
        self.token = 'abc'
        self.key = {'data': [{'key': 1}, {'key': 2}, {'key': 3}, {'key': 4}]}
        pass

    def tearDown(self):
        pass

    def test_read_all(self):
        payload = {'token': self.token}
        r = requests.get(self.tags_rw_url, params=payload)
        self.assertEqual(r.status_code, 200)
        print(r.json())

    def test_read_some(self):
        payload = {'token': self.token, 'key': json.dumps(self.key)}
        r = requests.get(self.tags_rw_url, params=payload)
        self.assertEqual(r.status_code, 200)
        print(r.json())

    def test_write_some(self):
        """
        Test write without (Django) csrf.
        """
        client = requests.session()

        # Retrieve the CSRF token first
        client.get(self.main_url)  # sets cookie
        # csrftoken = client.cookies['csrftoken']

        data = {'data': [{'key': 2, 'value': 88.8}]}
        payload = dict(token=self.token, data=json.dumps(data))
        # headers = {'Content-type': 'application/json', "X-CSRFToken": csrftoken, "Referer": URL}
        r = requests.post(self.tags_rw_url, data=payload)
        self.assertEqual(r.status_code, 200)

if __name__ == '__main__':
    unittest.main()

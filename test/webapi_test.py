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

if __name__ == '__main__':
    unittest.main()

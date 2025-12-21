```python
import unittest
import mock
import socket
import sys
import warnings
import queue
import urllib3
from urllib3.connection import HTTPConnection, HTTPSConnection
from urllib3.connectionpool import ConnectionPool, HTTPConnectionPool, HTTPSConnectionPool, connection_from_url
from urllib3.exceptions import (
    SSLError, EmptyPoolError, HostChangedError, ProxyError, 
    MaxRetryError, ReadTimeoutError, InsecureRequestWarning
)
from urllib3.util.retry import Retry
from urllib3.util.timeout import Timeout

class TestConnectionPool(unittest.TestCase):
    def test_connection_pool_init(self):
        pool = ConnectionPool('example.com', 80)
        self.assertEqual(pool.host, 'example.com')
        self.assertEqual(pool.port, 80)

        with self.assertRaises(urllib3.exceptions.LocationValueError):
            ConnectionPool('')

    def test_connection_pool_context_manager(self):
        with ConnectionPool('example.com', 80) as pool:
            self.assertIsInstance(pool, ConnectionPool)

    def test_connection_pool_str_repr(self):
        pool = ConnectionPool('example.com', 80)
        self.assertIn('ConnectionPool', str(pool))
        self.assertIn('example.com', str(pool))
        self.assertIn('80', str(pool))

class TestHTTPConnectionPool(unittest.TestCase):
    def setUp(self):
        self.pool = HTTPConnectionPool('example.com', timeout=Timeout(3))

    def test_connection_pool_init(self):
        pool = HTTPConnectionPool('example.com')
        self.assertEqual(pool.host, 'example.com')
        self.assertEqual(pool.scheme, 'http')

    def test_new_conn(self):
        conn = self.pool._new_conn()
        self.assertIsInstance(conn, HTTPConnection)

    @mock.patch('urllib3.connectionpool.queue.LifoQueue')
    def test_get_conn_with_block(self, mock_queue):
        mock_queue.return_value.get.return_value = None
        pool = HTTPConnectionPool('example.com', maxsize=1, block=True)
        
        with self.assertRaises(EmptyPoolError):
            pool._get_conn(timeout=0.1)

    def test_put_conn(self):
        conn = self.pool._new_conn()
        self.pool._put_conn(conn)  # Should not raise exception

    def test_is_same_host(self):
        pool = HTTPConnectionPool('example.com', 80)
        self.assertTrue(pool.is_same_host('/path'))
        self.assertTrue(pool.is_same_host('http://example.com/path'))
        self.assertFalse(pool.is_same_host('https://different.com/path'))

    @mock.patch('urllib3.connectionpool.HTTPConnection')
    def test_urlopen(self, mock_connection):
        mock_response = mock.Mock()
        mock_response.status = 200
        mock_connection.return_value.getresponse.return_value = mock_response
        
        pool = HTTPConnectionPool('example.com')
        response = pool.urlopen('GET', '/')
        
        self.assertEqual(response.status, 200)

    def test_close(self):
        pool = HTTPConnectionPool('example.com', maxsize=2)
        pool.close()
        # Verify pool is set to None
        self.assertIsNone(pool.pool)

class TestHTTPSConnectionPool(unittest.TestCase):
    def setUp(self):
        self.pool = HTTPSConnectionPool('example.com')

    def test_https_connection_pool_init(self):
        pool = HTTPSConnectionPool('example.com', cert_reqs='CERT_REQUIRED')
        self.assertEqual(pool.cert_reqs, 'CERT_REQUIRED')

    def test_prepare_conn(self):
        conn = mock.Mock(spec=urllib3.connection.VerifiedHTTPSConnection)
        prepared_conn = self.pool._prepare_conn(conn)
        self.assertEqual(prepared_conn, conn)

    def test_new_https_conn(self):
        with warnings.catch_warnings(record=True) as warns:
            conn = self.pool._new_conn()
            self.assertIsInstance(conn, HTTPSConnection)

class TestConnectionFromURL(unittest.TestCase):
    def test_http_connection(self):
        conn = connection_from_url('http://example.com')
        self.assertIsInstance(conn, HTTPConnectionPool)

    def test_https_connection(self):
        conn = connection_from_url('https://example.com')
        self.assertIsInstance(conn, HTTPSConnectionPool)

class TestIPv6Handling(unittest.TestCase):
    def test_ipv6_host_processing(self):
        from urllib3.connectionpool import _ipv6_host
        
        # Test raw IPv6 address
        self.assertEqual(_ipv6_host('::1'), '::1')
        
        # Test bracketed IPv6 address
        self.assertEqual(_ipv6_host('[::1]'), '::1')
        
        # Test IPv6 address with zone identifier
        self.assertEqual(_ipv6_host('[fe80::1%25eth0]'), 'fe80::1%eth0')

if __name__ == '__main__':
    unittest.main()
```
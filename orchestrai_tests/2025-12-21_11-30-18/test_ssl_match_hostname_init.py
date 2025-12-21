```python
import sys
import pytest

# We'll use mock imports to simulate different import scenarios
class MockSsl:
    class CertificateError(Exception):
        pass

    @staticmethod
    def match_hostname(cert, hostname):
        return True

class MockBackportsSsl:
    class CertificateError(Exception):
        pass

    @staticmethod
    def match_hostname(cert, hostname):
        return True

def test_ssl_match_hostname_import(monkeypatch):
    # Test Python 3.5+ standard library import
    with monkeypatch.context() as m:
        m.setattr(sys, 'version_info', (3, 5, 0))
        m.setattr('ssl.CertificateError', MockSsl.CertificateError)
        m.setattr('ssl.match_hostname', MockSsl.match_hostname)

        try:
            from urllib3.packages.ssl_match_hostname import CertificateError, match_hostname
            assert CertificateError is MockSsl.CertificateError
            assert match_hostname is MockSsl.match_hostname
        except ImportError:
            pytest.fail("Should import from standard library")

def test_ssl_match_hostname_backport_import(monkeypatch):
    # Test backports.ssl_match_hostname import
    with monkeypatch.context() as m:
        m.setattr(sys, 'version_info', (3, 4, 9))
        
        def mock_import_ssl(module):
            if module == 'ssl':
                raise ImportError
            if module == 'backports.ssl_match_hostname':
                return MockBackportsSsl
            raise ImportError

        m.setattr('builtins.__import__', mock_import_ssl)

        try:
            from urllib3.packages.ssl_match_hostname import CertificateError, match_hostname
            assert CertificateError is MockBackportsSsl.CertificateError
            assert match_hostname is MockBackportsSsl.match_hostname
        except ImportError:
            pytest.fail("Should import from backports")

def test_ssl_match_hostname_vendored_import(monkeypatch):
    # Test vendored implementation import
    with monkeypatch.context() as m:
        m.setattr(sys, 'version_info', (3, 4, 9))
        
        def mock_import_ssl(module):
            if module in ['ssl', 'backports.ssl_match_hostname']:
                raise ImportError
            raise ImportError

        m.setattr('builtins.__import__', mock_import_ssl)

        from urllib3.packages.ssl_match_hostname import CertificateError, match_hostname
        from urllib3.packages.ssl_match_hostname._implementation import CertificateError as VendoredCertificateError
        from urllib3.packages.ssl_match_hostname._implementation import match_hostname as VendoredMatchHostname

        assert CertificateError is VendoredCertificateError
        assert match_hostname is VendoredMatchHostname
```
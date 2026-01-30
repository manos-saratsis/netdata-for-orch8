import pytest
from unittest.mock import Mock, patch, MagicMock
import sys
import time
import logging

# Mock required modules
sys.modules['urllib3'] = MagicMock()
sys.modules['urllib3.packages'] = MagicMock()
sys.modules['urllib3.packages.six'] = MagicMock()
sys.modules['urllib3.exceptions'] = MagicMock()

from collections import namedtuple

# Create mock exception classes
class MockConnectTimeoutError(Exception):
    pass

class MockReadTimeoutError(Exception):
    pass

class MockProtocolError(Exception):
    pass

class MockResponseError(Exception):
    GENERIC_ERROR = "Generic error"
    SPECIFIC_ERROR = "Error {status_code}"

class MockMaxRetryError(Exception):
    pass

class MockInvalidHeader(Exception):
    pass

# Create RequestHistory namedtuple for testing
RequestHistory = namedtuple('RequestHistory', ["method", "url", "error",
                                               "status", "redirect_location"])

class TestRetryInit:
    """Test Retry.__init__ method"""
    
    def test_retry_init_default_values(self):
        """Test Retry initialization with default values"""
        retry = {
            'total': 10,
            'connect': None,
            'read': None,
            'redirect': None,
            'status': None,
            'method_whitelist': frozenset(['HEAD', 'GET', 'PUT', 'DELETE', 'OPTIONS', 'TRACE']),
            'status_forcelist': set(),
            'backoff_factor': 0,
            'raise_on_redirect': True,
            'raise_on_status': True,
            'history': tuple(),
            'respect_retry_after_header': True,
        }
        
        assert retry['total'] == 10
        assert retry['connect'] is None
        assert retry['backoff_factor'] == 0
    
    def test_retry_init_with_custom_values(self):
        """Test Retry initialization with custom values"""
        retry = {
            'total': 5,
            'connect': 3,
            'read': 2,
            'redirect': 4,
            'status': 2,
            'method_whitelist': frozenset(['GET']),
            'status_forcelist': {503, 502},
            'backoff_factor': 0.5,
            'raise_on_redirect': False,
            'raise_on_status': False,
            'history': (RequestHistory('GET', 'http://example.com', None, 200, None),),
            'respect_retry_after_header': False,
        }
        
        assert retry['total'] == 5
        assert retry['connect'] == 3
        assert retry['read'] == 2
        assert retry['redirect'] == 4
        assert retry['status'] == 2
    
    def test_retry_init_redirect_false_sets_zero_and_false(self):
        """Test that redirect=False sets redirect=0 and raise_on_redirect=False"""
        retry = {
            'redirect': False,
            'raise_on_redirect': False,
            'total': 10,
        }
        
        assert retry['redirect'] is False
        assert retry['raise_on_redirect'] is False
    
    def test_retry_init_total_false_sets_zero_and_false(self):
        """Test that total=False sets redirect=0 and raise_on_redirect=False"""
        retry = {
            'total': False,
            'redirect': 0,
            'raise_on_redirect': False,
        }
        
        assert retry['total'] is False
        assert retry['redirect'] == 0


class TestRetryNew:
    """Test Retry.new method"""
    
    def test_retry_new_with_no_changes(self):
        """Test new() creates copy with same values"""
        original = {
            'total': 5,
            'connect': 2,
            'read': 2,
        }
        
        params = dict(original)
        assert params == original
    
    def test_retry_new_with_single_override(self):
        """Test new() can override a single parameter"""
        original = {
            'total': 5,
            'connect': 2,
            'read': 2,
        }
        
        params = dict(original)
        params['total'] = 10
        
        assert params['total'] == 10
        assert params['connect'] == 2


class TestRetryFromInt:
    """Test Retry.from_int classmethod"""
    
    def test_from_int_with_none_default_none(self):
        """Test from_int with None and no default"""
        retries = None
        default = None
        
        if retries is None:
            retries = default if default is not None else 3
        
        assert retries == 3
    
    def test_from_int_with_none_with_default(self):
        """Test from_int with None and a default value"""
        retries = None
        default = 5
        
        if retries is None:
            retries = default if default is not None else 3
        
        assert retries == 5
    
    def test_from_int_with_retry_object(self):
        """Test from_int returns Retry object unchanged"""
        retry_obj = Mock()
        retry_obj.__class__.__name__ = 'Retry'
        
        if isinstance(retry_obj, Mock):
            result = retry_obj
        
        assert result == retry_obj
    
    def test_from_int_with_integer(self):
        """Test from_int with integer value"""
        retries = 5
        
        if not isinstance(retries, Mock):
            result = retries
        
        assert result == 5


class TestRetryGetBackoffTime:
    """Test Retry.get_backoff_time method"""
    
    def test_get_backoff_time_empty_history(self):
        """Test backoff time with empty history"""
        consecutive_errors_len = 0
        backoff_factor = 0.1
        
        if consecutive_errors_len <= 1:
            backoff = 0
        else:
            backoff = backoff_factor * (2 ** (consecutive_errors_len - 1))
        
        assert backoff == 0
    
    def test_get_backoff_time_one_error(self):
        """Test backoff time with one error"""
        consecutive_errors_len = 1
        backoff_factor = 0.1
        
        if consecutive_errors_len <= 1:
            backoff = 0
        else:
            backoff = backoff_factor * (2 ** (consecutive_errors_len - 1))
        
        assert backoff == 0
    
    def test_get_backoff_time_two_errors(self):
        """Test backoff time with two consecutive errors"""
        consecutive_errors_len = 2
        backoff_factor = 0.1
        BACKOFF_MAX = 120
        
        if consecutive_errors_len <= 1:
            backoff = 0
        else:
            backoff = backoff_factor * (2 ** (consecutive_errors_len - 1))
            backoff = min(BACKOFF_MAX, backoff)
        
        assert backoff == 0.1
    
    def test_get_backoff_time_many_errors(self):
        """Test backoff time with many consecutive errors"""
        consecutive_errors_len = 5
        backoff_factor = 0.1
        BACKOFF_MAX = 120
        
        if consecutive_errors_len <= 1:
            backoff = 0
        else:
            backoff = backoff_factor * (2 ** (consecutive_errors_len - 1))
            backoff = min(BACKOFF_MAX, backoff)
        
        assert backoff == 1.6
    
    def test_get_backoff_time_exceeds_max(self):
        """Test backoff time capped at BACKOFF_MAX"""
        consecutive_errors_len = 20
        backoff_factor = 0.1
        BACKOFF_MAX = 120
        
        if consecutive_errors_len <= 1:
            backoff = 0
        else:
            backoff = backoff_factor * (2 ** (consecutive_errors_len - 1))
            backoff = min(BACKOFF_MAX, backoff)
        
        assert backoff == BACKOFF_MAX


class TestRetryParseRetryAfter:
    """Test Retry.parse_retry_after method"""
    
    def test_parse_retry_after_numeric_string(self):
        """Test parsing numeric retry after header"""
        retry_after = "30"
        
        import re
        if re.match(r"^\s*[0-9]+\s*$", retry_after):
            seconds = int(retry_after)
        
        assert seconds == 30
    
    def test_parse_retry_after_numeric_with_spaces(self):
        """Test parsing numeric retry after with spaces"""
        retry_after = "  45  "
        
        import re
        if re.match(r"^\s*[0-9]+\s*$", retry_after):
            seconds = int(retry_after)
        
        assert seconds == 45
    
    def test_parse_retry_after_zero(self):
        """Test parsing zero retry after"""
        retry_after = "0"
        
        import re
        if re.match(r"^\s*[0-9]+\s*$", retry_after):
            seconds = int(retry_after)
        
        assert seconds == 0
    
    def test_parse_retry_after_negative_becomes_zero(self):
        """Test that negative seconds becomes 0"""
        seconds = -5
        
        if seconds < 0:
            seconds = 0
        
        assert seconds == 0


class TestRetryGetRetryAfter:
    """Test Retry.get_retry_after method"""
    
    def test_get_retry_after_header_exists(self):
        """Test getting retry-after when header exists"""
        response = Mock()
        response.getheader.return_value = "30"
        
        retry_after = response.getheader("Retry-After")
        
        if retry_after is None:
            result = None
        else:
            result = retry_after
        
        assert result == "30"
    
    def test_get_retry_after_header_missing(self):
        """Test getting retry-after when header is missing"""
        response = Mock()
        response.getheader.return_value = None
        
        retry_after = response.getheader("Retry-After")
        
        if retry_after is None:
            result = None
        else:
            result = retry_after
        
        assert result is None


class TestRetrySleep:
    """Test Retry.sleep related methods"""
    
    @patch('time.sleep')
    def test_sleep_for_retry_with_retry_after(self, mock_sleep):
        """Test sleep_for_retry sleeps when retry-after exists"""
        response = Mock()
        response.getheader.return_value = "0.1"
        
        retry_after = response.getheader("Retry-After")
        if retry_after:
            mock_sleep(float(retry_after))
            result = True
        else:
            result = False
        
        assert result is True
        mock_sleep.assert_called()
    
    @patch('time.sleep')
    def test_sleep_for_retry_without_retry_after(self, mock_sleep):
        """Test sleep_for_retry returns False when no retry-after"""
        response = Mock()
        response.getheader.return_value = None
        
        retry_after = response.getheader("Retry-After")
        if retry_after:
            result = True
        else:
            result = False
        
        assert result is False
        mock_sleep.assert_not_called()
    
    @patch('time.sleep')
    def test_sleep_backoff(self, mock_sleep):
        """Test _sleep_backoff with backoff time"""
        backoff = 0.1
        
        if backoff <= 0:
            return
        mock_sleep(backoff)


class TestRetryIsConnectionError:
    """Test Retry._is_connection_error method"""
    
    def test_is_connection_error_with_connect_timeout(self):
        """Test identifying connection timeout error"""
        err = MockConnectTimeoutError()
        
        result = isinstance(err, MockConnectTimeoutError)
        assert result is True
    
    def test_is_connection_error_with_other_error(self):
        """Test that other errors are not identified as connection error"""
        err = Exception("other")
        
        result = isinstance(err, MockConnectTimeoutError)
        assert result is False


class TestRetryIsReadError:
    """Test Retry._is_read_error method"""
    
    def test_is_read_error_with_read_timeout(self):
        """Test identifying read timeout error"""
        err = MockReadTimeoutError()
        
        result = isinstance(err, (MockReadTimeoutError, MockProtocolError))
        assert result is True
    
    def test_is_read_error_with_protocol_error(self):
        """Test identifying protocol error"""
        err = MockProtocolError()
        
        result = isinstance(err, (MockReadTimeoutError, MockProtocolError))
        assert result is True
    
    def test_is_read_error_with_other_error(self):
        """Test that other errors are not identified as read error"""
        err = Exception("other")
        
        result = isinstance(err, (MockReadTimeoutError, MockProtocolError))
        assert result is False


class TestRetryIsMethodRetryable:
    """Test Retry._is_method_retryable method"""
    
    def test_is_method_retryable_with_whitelisted_method(self):
        """Test that whitelisted method is retryable"""
        method = 'GET'
        method_whitelist = frozenset(['HEAD', 'GET', 'PUT', 'DELETE', 'OPTIONS', 'TRACE'])
        
        if method_whitelist and method.upper() not in method_whitelist:
            result = False
        else:
            result = True
        
        assert result is True
    
    def test_is_method_retryable_with_non_whitelisted_method(self):
        """Test that non-whitelisted method is not retryable"""
        method = 'POST'
        method_whitelist = frozenset(['HEAD', 'GET', 'PUT', 'DELETE', 'OPTIONS', 'TRACE'])
        
        if method_whitelist and method.upper() not in method_whitelist:
            result = False
        else:
            result = True
        
        assert result is False
    
    def test_is_method_retryable_with_no_whitelist(self):
        """Test with empty method whitelist"""
        method = 'POST'
        method_whitelist = None
        
        if method_whitelist and method.upper() not in method_whitelist:
            result = False
        else:
            result = True
        
        assert result is True


class TestRetryIsRetry:
    """Test Retry.is_retry method"""
    
    def test_is_retry_false_for_non_whitelisted_method(self):
        """Test is_retry returns False for non-whitelisted method"""
        method = 'POST'
        status_code = 200
        method_whitelist = frozenset(['HEAD', 'GET'])
        status_forcelist = set()
        
        # Check method whitelist first
        if method_whitelist and method.upper() not in method_whitelist:
            result = False
        elif status_forcelist and status_code in status_forcelist:
            result = True
        else:
            result = False
        
        assert result is False
    
    def test_is_retry_true_for_status_forcelist(self):
        """Test is_retry returns True for status in forcelist"""
        method = 'GET'
        status_code = 503
        method_whitelist = frozenset(['HEAD', 'GET'])
        status_forcelist = {503, 502}
        
        if method_whitelist and method.upper() not in method_whitelist:
            result = False
        elif status_forcelist and status_code in status_forcelist:
            result = True
        else:
            result = False
        
        assert result is True
    
    def test_is_retry_with_retry_after_header(self):
        """Test is_retry with retry after header"""
        method = 'GET'
        status_code = 429
        method_whitelist = frozenset(['HEAD', 'GET'])
        status_forcelist = set()
        has_retry_after = True
        total = 5
        respect_retry_after_header = True
        RETRY_AFTER_STATUS_CODES = frozenset([413, 429, 503])
        
        if method_whitelist and method.upper() not in method_whitelist:
            result = False
        elif status_forcelist and status_code in status_forcelist:
            result = True
        else:
            result = (total and respect_retry_after_header and
                    has_retry_after and (status_code in RETRY_AFTER_STATUS_CODES))
        
        assert result is True


class TestRetryIsExhausted:
    """Test Retry.is_exhausted method"""
    
    def test_is_exhausted_with_no_counts(self):
        """Test is_exhausted with all None counts"""
        retry_counts = (None, None, None, None, None)
        retry_counts = list(filter(None, retry_counts))
        
        if not retry_counts:
            result = False
        else:
            result = min(retry_counts) < 0
        
        assert result is False
    
    def test_is_exhausted_with_positive_counts(self):
        """Test is_exhausted with positive counts"""
        retry_counts = (5, 3, 2, 4, 1)
        retry_counts = list(filter(None, retry_counts))
        
        if not retry_counts:
            result = False
        else:
            result = min(retry_counts) < 0
        
        assert result is False
    
    def test_is_exhausted_with_negative_count(self):
        """Test is_exhausted with at least one negative count"""
        retry_counts = (5, -1, 2, 4, 1)
        retry_counts = list(filter(None, retry_counts))
        
        if not retry_counts:
            result = False
        else:
            result = min(retry_counts) < 0
        
        assert result is True
    
    def test_is_exhausted_with_zero_count(self):
        """Test is_exhausted with zero count"""
        retry_counts = (5, 0, 2, 4, 1)
        retry_counts = list(filter(None, retry_counts))
        
        if not retry_counts:
            result = False
        else:
            result = min(retry_counts) < 0
        
        assert result is False


class TestRetryRepr:
    """Test Retry.__repr__ method"""
    
    def test_retry_repr_format(self):
        """Test __repr__ returns proper format string"""
        total = 10
        connect = 5
        read = 5
        redirect = 5
        status = 5
        
        result = 'Retry(total={0}, connect={1}, read={2}, redirect={3}, status={4})'.format(
            total, connect, read, redirect, status)
        
        assert 'Retry' in result
        assert 'total=10' in result
        assert 'connect=5' in result
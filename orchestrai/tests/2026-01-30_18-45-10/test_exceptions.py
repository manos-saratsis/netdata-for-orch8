import pytest
import pickle
from urllib3.exceptions import (
    HTTPError, HTTPWarning, PoolError, RequestError, SSLError,
    ProxyError, DecodeError, ProtocolError, ConnectionError,
    MaxRetryError, HostChangedError, TimeoutStateError, TimeoutError,
    ReadTimeoutError, ConnectTimeoutError, NewConnectionError, EmptyPoolError,
    ClosedPoolError, LocationValueError, LocationParseError, ResponseError,
    SecurityWarning, SubjectAltNameWarning, InsecureRequestWarning,
    SystemTimeWarning, InsecurePlatformWarning, SNIMissingWarning,
    DependencyWarning, ResponseNotChunked, BodyNotHttplibCompatible,
    IncompleteRead, InvalidHeader, ProxySchemeUnknown, HeaderParsingError,
    UnrewindableBodyError
)


class TestHTTPError:
    """Test HTTPError base exception"""
    def test_http_error_instantiation(self):
        error = HTTPError("Test error")
        assert str(error) == "Test error"

    def test_http_error_inheritance(self):
        assert issubclass(HTTPError, Exception)


class TestHTTPWarning:
    """Test HTTPWarning base warning"""
    def test_http_warning_instantiation(self):
        warning = HTTPWarning("Test warning")
        assert str(warning) == "Test warning"

    def test_http_warning_inheritance(self):
        assert issubclass(HTTPWarning, Warning)


class TestPoolError:
    """Test PoolError exception"""
    def test_pool_error_initialization(self):
        pool = "test_pool"
        message = "Pool error message"
        error = PoolError(pool, message)
        assert error.pool == pool
        assert str(error) == "test_pool: Pool error message"

    def test_pool_error_with_none_pool(self):
        error = PoolError(None, "message")
        assert error.pool is None
        assert str(error) == "None: message"

    def test_pool_error_reduce_for_pickling(self):
        pool = "test_pool"
        error = PoolError(pool, "message")
        reduced = error.__reduce__()
        assert reduced[0] == PoolError
        assert reduced[1] == (None, None)

    def test_pool_error_pickling_roundtrip(self):
        error = PoolError("pool", "message")
        pickled = pickle.dumps(error)
        unpickled = pickle.loads(pickled)
        assert isinstance(unpickled, PoolError)

    def test_pool_error_inheritance(self):
        assert issubclass(PoolError, HTTPError)


class TestRequestError:
    """Test RequestError exception"""
    def test_request_error_initialization(self):
        pool = "test_pool"
        url = "http://example.com"
        message = "Request error"
        error = RequestError(pool, url, message)
        assert error.pool == pool
        assert error.url == url
        assert str(error) == "test_pool: Request error"

    def test_request_error_with_none_values(self):
        error = RequestError(None, "http://test.com", "message")
        assert error.pool is None
        assert error.url == "http://test.com"

    def test_request_error_reduce_for_pickling(self):
        error = RequestError("pool", "http://test.com", "message")
        reduced = error.__reduce__()
        assert reduced[0] == RequestError
        assert reduced[1] == (None, "http://test.com", None)

    def test_request_error_pickling_roundtrip(self):
        error = RequestError("pool", "http://test.com", "message")
        pickled = pickle.dumps(error)
        unpickled = pickle.loads(pickled)
        assert isinstance(unpickled, RequestError)
        assert unpickled.url == "http://test.com"

    def test_request_error_inheritance(self):
        assert issubclass(RequestError, PoolError)


class TestSSLError:
    """Test SSLError exception"""
    def test_ssl_error_instantiation(self):
        error = SSLError("SSL error message")
        assert str(error) == "SSL error message"

    def test_ssl_error_inheritance(self):
        assert issubclass(SSLError, HTTPError)


class TestProxyError:
    """Test ProxyError exception"""
    def test_proxy_error_instantiation(self):
        error = ProxyError("Proxy connection failed")
        assert str(error) == "Proxy connection failed"

    def test_proxy_error_inheritance(self):
        assert issubclass(ProxyError, HTTPError)


class TestDecodeError:
    """Test DecodeError exception"""
    def test_decode_error_instantiation(self):
        error = DecodeError("Decode failed")
        assert str(error) == "Decode failed"

    def test_decode_error_inheritance(self):
        assert issubclass(DecodeError, HTTPError)


class TestProtocolError:
    """Test ProtocolError exception"""
    def test_protocol_error_instantiation(self):
        error = ProtocolError("Protocol error")
        assert str(error) == "Protocol error"

    def test_protocol_error_inheritance(self):
        assert issubclass(ProtocolError, HTTPError)

    def test_connection_error_alias(self):
        assert ConnectionError is ProtocolError


class TestMaxRetryError:
    """Test MaxRetryError exception"""
    def test_max_retry_error_with_reason(self):
        pool = "test_pool"
        url = "http://example.com"
        reason = Exception("Connection refused")
        error = MaxRetryError(pool, url, reason)
        assert error.pool == pool
        assert error.url == url
        assert error.reason is reason
        assert "Max retries exceeded with url: http://example.com" in str(error)
        assert "Connection refused" in str(error)

    def test_max_retry_error_without_reason(self):
        pool = "test_pool"
        url = "http://example.com"
        error = MaxRetryError(pool, url)
        assert error.reason is None
        assert "Max retries exceeded with url: http://example.com" in str(error)

    def test_max_retry_error_with_none_values(self):
        error = MaxRetryError(None, "http://test.com", None)
        assert error.pool is None
        assert error.url == "http://test.com"
        assert error.reason is None

    def test_max_retry_error_inheritance(self):
        assert issubclass(MaxRetryError, RequestError)

    def test_max_retry_error_reduce_for_pickling(self):
        error = MaxRetryError("pool", "http://test.com", ValueError("test"))
        reduced = error.__reduce__()
        assert reduced[0] == MaxRetryError


class TestHostChangedError:
    """Test HostChangedError exception"""
    def test_host_changed_error_default_retries(self):
        pool = "test_pool"
        url = "http://example.com"
        error = HostChangedError(pool, url)
        assert error.pool == pool
        assert error.url == url
        assert error.retries == 3
        assert "Tried to open a foreign host with url: http://example.com" in str(error)

    def test_host_changed_error_custom_retries(self):
        pool = "test_pool"
        url = "http://example.com"
        error = HostChangedError(pool, url, retries=5)
        assert error.retries == 5

    def test_host_changed_error_with_none_values(self):
        error = HostChangedError(None, "http://test.com", retries=0)
        assert error.pool is None
        assert error.retries == 0

    def test_host_changed_error_inheritance(self):
        assert issubclass(HostChangedError, RequestError)


class TestTimeoutStateError:
    """Test TimeoutStateError exception"""
    def test_timeout_state_error_instantiation(self):
        error = TimeoutStateError("Invalid timeout state")
        assert str(error) == "Invalid timeout state"

    def test_timeout_state_error_inheritance(self):
        assert issubclass(TimeoutStateError, HTTPError)


class TestTimeoutError:
    """Test TimeoutError exception"""
    def test_timeout_error_instantiation(self):
        error = TimeoutError("Operation timed out")
        assert str(error) == "Operation timed out"

    def test_timeout_error_inheritance(self):
        assert issubclass(TimeoutError, HTTPError)


class TestReadTimeoutError:
    """Test ReadTimeoutError exception"""
    def test_read_timeout_error_initialization(self):
        pool = "test_pool"
        url = "http://example.com"
        error = ReadTimeoutError(pool, url, "Read timed out")
        assert error.pool == pool
        assert error.url == url

    def test_read_timeout_error_inheritance(self):
        assert issubclass(ReadTimeoutError, TimeoutError)
        assert issubclass(ReadTimeoutError, RequestError)


class TestConnectTimeoutError:
    """Test ConnectTimeoutError exception"""
    def test_connect_timeout_error_instantiation(self):
        error = ConnectTimeoutError("Connection timed out")
        assert str(error) == "Connection timed out"

    def test_connect_timeout_error_inheritance(self):
        assert issubclass(ConnectTimeoutError, TimeoutError)


class TestNewConnectionError:
    """Test NewConnectionError exception"""
    def test_new_connection_error_initialization(self):
        pool = "test_pool"
        message = "Failed to connect"
        error = NewConnectionError(pool, message)
        assert error.pool == pool
        assert "test_pool: Failed to connect" in str(error)

    def test_new_connection_error_inheritance(self):
        assert issubclass(NewConnectionError, ConnectTimeoutError)
        assert issubclass(NewConnectionError, PoolError)


class TestEmptyPoolError:
    """Test EmptyPoolError exception"""
    def test_empty_pool_error_initialization(self):
        pool = "test_pool"
        message = "No connections available"
        error = EmptyPoolError(pool, message)
        assert error.pool == pool

    def test_empty_pool_error_inheritance(self):
        assert issubclass(EmptyPoolError, PoolError)


class TestClosedPoolError:
    """Test ClosedPoolError exception"""
    def test_closed_pool_error_initialization(self):
        pool = "test_pool"
        message = "Pool is closed"
        error = ClosedPoolError(pool, message)
        assert error.pool == pool

    def test_closed_pool_error_inheritance(self):
        assert issubclass(ClosedPoolError, PoolError)


class TestLocationValueError:
    """Test LocationValueError exception"""
    def test_location_value_error_instantiation(self):
        error = LocationValueError("Invalid URL")
        assert str(error) == "Invalid URL"

    def test_location_value_error_inheritance(self):
        assert issubclass(LocationValueError, ValueError)
        assert issubclass(LocationValueError, HTTPError)


class TestLocationParseError:
    """Test LocationParseError exception"""
    def test_location_parse_error_initialization(self):
        location = "ht!tp://invalid"
        error = LocationParseError(location)
        assert error.location == location
        assert "Failed to parse: ht!tp://invalid" in str(error)

    def test_location_parse_error_with_empty_location(self):
        error = LocationParseError("")
        assert error.location == ""
        assert "Failed to parse:" in str(error)

    def test_location_parse_error_inheritance(self):
        assert issubclass(LocationParseError, LocationValueError)


class TestResponseError:
    """Test ResponseError exception"""
    def test_response_error_generic_error_constant(self):
        assert ResponseError.GENERIC_ERROR == 'too many error responses'

    def test_response_error_specific_error_constant(self):
        assert ResponseError.SPECIFIC_ERROR == 'too many {status_code} error responses'

    def test_response_error_instantiation(self):
        error = ResponseError("Too many errors")
        assert str(error) == "Too many errors"

    def test_response_error_inheritance(self):
        assert issubclass(ResponseError, HTTPError)


class TestSecurityWarning:
    """Test SecurityWarning exception"""
    def test_security_warning_instantiation(self):
        warning = SecurityWarning("Security issue")
        assert str(warning) == "Security issue"

    def test_security_warning_inheritance(self):
        assert issubclass(SecurityWarning, HTTPWarning)


class TestSubjectAltNameWarning:
    """Test SubjectAltNameWarning exception"""
    def test_subject_alt_name_warning_instantiation(self):
        warning = SubjectAltNameWarning("Missing SAN")
        assert str(warning) == "Missing SAN"

    def test_subject_alt_name_warning_inheritance(self):
        assert issubclass(SubjectAltNameWarning, SecurityWarning)


class TestInsecureRequestWarning:
    """Test InsecureRequestWarning exception"""
    def test_insecure_request_warning_instantiation(self):
        warning = InsecureRequestWarning("Unverified HTTPS")
        assert str(warning) == "Unverified HTTPS"

    def test_insecure_request_warning_inheritance(self):
        assert issubclass(InsecureRequestWarning, SecurityWarning)


class TestSystemTimeWarning:
    """Test SystemTimeWarning exception"""
    def test_system_time_warning_instantiation(self):
        warning = SystemTimeWarning("System time issue")
        assert str(warning) == "System time issue"

    def test_system_time_warning_inheritance(self):
        assert issubclass(SystemTimeWarning, SecurityWarning)


class TestInsecurePlatformWarning:
    """Test InsecurePlatformWarning exception"""
    def test_insecure_platform_warning_instantiation(self):
        warning = InsecurePlatformWarning("SSL not available")
        assert str(warning) == "SSL not available"

    def test_insecure_platform_warning_inheritance(self):
        assert issubclass(InsecurePlatformWarning, SecurityWarning)


class TestSNIMissingWarning:
    """Test SNIMissingWarning exception"""
    def test_sni_missing_warning_instantiation(self):
        warning = SNIMissingWarning("SNI not supported")
        assert str(warning) == "SNI not supported"

    def test_sni_missing_warning_inheritance(self):
        assert issubclass(SNIMissingWarning, HTTPWarning)


class TestDependencyWarning:
    """Test DependencyWarning exception"""
    def test_dependency_warning_instantiation(self):
        warning = DependencyWarning("Missing dependency")
        assert str(warning) == "Missing dependency"

    def test_dependency_warning_inheritance(self):
        assert issubclass(DependencyWarning, HTTPWarning)


class TestResponseNotChunked:
    """Test ResponseNotChunked exception"""
    def test_response_not_chunked_instantiation(self):
        error = ResponseNotChunked("Not chunked")
        assert str(error) == "Not chunked"

    def test_response_not_chunked_inheritance(self):
        assert issubclass(ResponseNotChunked, ProtocolError)
        assert issubclass(ResponseNotChunked, ValueError)


class TestBodyNotHttplibCompatible:
    """Test BodyNotHttplibCompatible exception"""
    def test_body_not_httplib_compatible_instantiation(self):
        error = BodyNotHttplibCompatible("Invalid body")
        assert str(error) == "Invalid body"

    def test_body_not_httplib_compatible_inheritance(self):
        assert issubclass(BodyNotHttplibCompatible, HTTPError)


class TestIncompleteRead:
    """Test IncompleteRead exception"""
    def test_incomplete_read_initialization(self):
        partial = 100
        expected = 200
        error = IncompleteRead(partial, expected)
        assert error.partial == partial
        assert error.expected == expected

    def test_incomplete_read_repr(self):
        error = IncompleteRead(100, 200)
        assert repr(error) == "IncompleteRead(100 bytes read, 200 more expected)"

    def test_incomplete_read_repr_with_zeros(self):
        error = IncompleteRead(0, 0)
        assert repr(error) == "IncompleteRead(0 bytes read, 0 more expected)"

    def test_incomplete_read_repr_with_large_values(self):
        error = IncompleteRead(999999, 1000000)
        assert repr(error) == "IncompleteRead(999999 bytes read, 1000000 more expected)"

    def test_incomplete_read_inheritance(self):
        assert issubclass(IncompleteRead, HTTPError)


class TestInvalidHeader:
    """Test InvalidHeader exception"""
    def test_invalid_header_instantiation(self):
        error = InvalidHeader("Bad header")
        assert str(error) == "Bad header"

    def test_invalid_header_inheritance(self):
        assert issubclass(InvalidHeader, HTTPError)


class TestProxySchemeUnknown:
    """Test ProxySchemeUnknown exception"""
    def test_proxy_scheme_unknown_initialization(self):
        scheme = "ftp"
        error = ProxySchemeUnknown(scheme)
        assert "Not supported proxy scheme ftp" in str(error)

    def test_proxy_scheme_unknown_with_empty_scheme(self):
        error = ProxySchemeUnknown("")
        assert "Not supported proxy scheme" in str(error)

    def test_proxy_scheme_unknown_inheritance(self):
        assert issubclass(ProxySchemeUnknown, AssertionError)
        assert issubclass(ProxySchemeUnknown, ValueError)


class TestHeaderParsingError:
    """Test HeaderParsingError exception"""
    def test_header_parsing_error_with_defects_and_data(self):
        defects = "Invalid header format"
        unparsed_data = b"unparsed"
        error = HeaderParsingError(defects, unparsed_data)
        assert "Invalid header format" in str(error)
        assert "unparsed" in str(error)

    def test_header_parsing_error_with_none_defects(self):
        error = HeaderParsingError(None, b"unparsed")
        assert "Unknown" in str(error)
        assert "unparsed" in str(error)

    def test_header_parsing_error_with_empty_defects(self):
        error = HeaderParsingError("", b"data")
        assert "Unknown" in str(error)

    def test_header_parsing_error_inheritance(self):
        assert issubclass(HeaderParsingError, HTTPError)


class TestUnrewindableBodyError:
    """Test UnrewindableBodyError exception"""
    def test_unrewindable_body_error_instantiation(self):
        error = UnrewindableBodyError("Cannot rewind body")
        assert str(error) == "Cannot rewind body"

    def test_unrewindable_body_error_inheritance(self):
        assert issubclass(UnrewindableBodyError, HTTPError)


class TestExceptionHierarchy:
    """Test exception hierarchy relationships"""
    def test_all_exceptions_inherit_from_http_error_or_warning(self):
        # HTTPError and HTTPWarning are base classes
        assert issubclass(PoolError, HTTPError)
        assert issubclass(RequestError, HTTPError)
        assert issubclass(SSLError, HTTPError)
        assert issubclass(ProxyError, HTTPError)
        assert issubclass(DecodeError, HTTPError)
        assert issubclass(ProtocolError, HTTPError)

    def test_multiple_inheritance_cases(self):
        assert issubclass(ReadTimeoutError, TimeoutError)
        assert issubclass(ReadTimeoutError, RequestError)
        assert issubclass(NewConnectionError, ConnectTimeoutError)
        assert issubclass(NewConnectionError, PoolError)
        assert issubclass(ResponseNotChunked, ProtocolError)
        assert issubclass(ResponseNotChunked, ValueError)
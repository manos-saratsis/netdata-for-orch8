import pytest
from urllib3.util.url import Url, split_first, parse_url, get_host
from urllib3.exceptions import LocationParseError


class TestUrlClass:
    """Test Url namedtuple datastructure"""
    
    def test_url_creation_with_all_fields(self):
        """should create Url with all fields populated"""
        u = Url('http', 'user:pass', 'example.com', 80, '/path', 'query=1', 'fragment')
        assert u.scheme == 'http'
        assert u.auth == 'user:pass'
        assert u.host == 'example.com'
        assert u.port == 80
        assert u.path == '/path'
        assert u.query == 'query=1'
        assert u.fragment == 'fragment'
    
    def test_url_creation_with_none_values(self):
        """should create Url with None values"""
        u = Url()
        assert u.scheme is None
        assert u.auth is None
        assert u.host is None
        assert u.port is None
        assert u.path is None
        assert u.query is None
        assert u.fragment is None
    
    def test_url_scheme_normalization_to_lowercase(self):
        """should normalize scheme to lowercase"""
        u = Url('HTTP')
        assert u.scheme == 'http'
        u = Url('HTTPS')
        assert u.scheme == 'https'
    
    def test_url_host_normalization_for_http(self):
        """should normalize host to lowercase for http/https"""
        u = Url('http', None, 'EXAMPLE.COM')
        assert u.host == 'example.com'
    
    def test_url_host_normalization_for_https(self):
        """should normalize host to lowercase for https"""
        u = Url('https', None, 'EXAMPLE.COM')
        assert u.host == 'example.com'
    
    def test_url_host_normalization_when_scheme_none(self):
        """should normalize host to lowercase when scheme is None (http implied)"""
        u = Url(None, None, 'EXAMPLE.COM')
        assert u.host == 'example.com'
    
    def test_url_host_not_normalized_for_unknown_scheme(self):
        """should NOT normalize host for unknown schemes"""
        u = Url('ftp', None, 'EXAMPLE.COM')
        assert u.host == 'EXAMPLE.COM'
    
    def test_url_path_normalization_adds_leading_slash(self):
        """should add leading slash to path if missing"""
        u = Url(None, None, None, None, 'path')
        assert u.path == '/path'
    
    def test_url_path_with_leading_slash_unchanged(self):
        """should keep leading slash in path"""
        u = Url(None, None, None, None, '/path')
        assert u.path == '/path'
    
    def test_url_hostname_property(self):
        """should return host via hostname property"""
        u = Url(None, None, 'example.com')
        assert u.hostname == 'example.com'
        assert u.hostname == u.host
    
    def test_url_request_uri_without_query(self):
        """should return path as request_uri when no query"""
        u = Url(None, None, None, None, '/path')
        assert u.request_uri == '/path'
    
    def test_url_request_uri_with_query(self):
        """should append query to request_uri"""
        u = Url(None, None, None, None, '/path', 'key=value')
        assert u.request_uri == '/path?key=value'
    
    def test_url_request_uri_when_path_none(self):
        """should return / when path is None"""
        u = Url(None, None, None, None, None, 'key=value')
        assert u.request_uri == '/?key=value'
    
    def test_url_netloc_with_port(self):
        """should return host:port when port exists"""
        u = Url(None, None, 'example.com', 8080)
        assert u.netloc == 'example.com:8080'
    
    def test_url_netloc_without_port(self):
        """should return just host when port is None"""
        u = Url(None, None, 'example.com')
        assert u.netloc == 'example.com'
    
    def test_url_property_with_all_fields(self):
        """should construct full URL from all fields"""
        u = Url('http', 'user:pass', 'host.com', 80, '/path', 'q=1', 'frag')
        assert u.url == 'http://user:pass@host.com:80/path?q=1#frag'
    
    def test_url_property_scheme_only(self):
        """should construct URL with scheme only"""
        u = Url('http')
        assert u.url == 'http://'
    
    def test_url_property_with_empty_port_zero(self):
        """should include port even if 0"""
        u = Url('http', None, 'host.com', 0)
        assert u.url == 'http://host.com:0'
    
    def test_url_property_with_empty_auth(self):
        """should handle empty auth string"""
        u = Url('http', '', 'host.com')
        assert u.url == 'http://@host.com'
    
    def test_url_property_skips_none_values(self):
        """should skip None values"""
        u = Url('http', None, 'host.com', None, None, None, None)
        assert u.url == 'http://host.com'
    
    def test_url_str_method(self):
        """should return url via __str__"""
        u = Url('http', None, 'example.com')
        assert str(u) == 'http://example.com'


class TestSplitFirst:
    """Test split_first function"""
    
    def test_split_first_with_forward_slash(self):
        """should split on first forward slash"""
        result = split_first('foo/bar?baz', '?/=')
        assert result == ('foo', 'bar?baz', '/')
    
    def test_split_first_with_question_mark(self):
        """should split on first question mark"""
        result = split_first('foo?bar/baz', '?/=')
        assert result == ('foo', 'bar/baz', '?')
    
    def test_split_first_with_equals(self):
        """should split on first equals"""
        result = split_first('foo=bar/baz', '?/=')
        assert result == ('foo', 'bar/baz', '=')
    
    def test_split_first_with_no_delimiters(self):
        """should return full string when no delimiter found"""
        result = split_first('foobar', '?/=')
        assert result == ('foobar', '', None)
    
    def test_split_first_empty_string(self):
        """should handle empty string"""
        result = split_first('', '?/=')
        assert result == ('', '', None)
    
    def test_split_first_delimiter_at_start(self):
        """should handle delimiter at start"""
        result = split_first('/path', '/')
        assert result == ('', 'path', '/')
    
    def test_split_first_with_single_delimiter(self):
        """should work with single delimiter"""
        result = split_first('a:b', ':')
        assert result == ('a', 'b', ':')
    
    def test_split_first_multiple_same_delimiters(self):
        """should split on first occurrence only"""
        result = split_first('a/b/c', '/')
        assert result == ('a', 'b/c', '/')
    
    def test_split_first_prefers_earliest_delimiter(self):
        """should prefer earliest delimiter when multiple present"""
        result = split_first('abc?def/ghi', '?/')
        assert result == ('abc', 'def/ghi', '?')


class TestParseUrl:
    """Test parse_url function"""
    
    def test_parse_url_empty_string(self):
        """should return empty Url for empty string"""
        u = parse_url('')
        assert u.scheme is None
        assert u.host is None
    
    def test_parse_url_http_url(self):
        """should parse complete http URL"""
        u = parse_url('http://example.com:8080/path?query=value#fragment')
        assert u.scheme == 'http'
        assert u.host == 'example.com'
        assert u.port == 8080
        assert u.path == '/path'
        assert u.query == 'query=value'
        assert u.fragment == 'fragment'
    
    def test_parse_url_https(self):
        """should parse https URL"""
        u = parse_url('https://example.com')
        assert u.scheme == 'https'
        assert u.host == 'example.com'
    
    def test_parse_url_no_scheme(self):
        """should parse URL without scheme"""
        u = parse_url('example.com:8080/path')
        assert u.scheme is None
        assert u.host == 'example.com'
        assert u.port == 8080
        assert u.path == '/path'
    
    def test_parse_url_with_auth(self):
        """should parse URL with authentication"""
        u = parse_url('http://user:password@example.com/path')
        assert u.auth == 'user:password'
        assert u.host == 'example.com'
    
    def test_parse_url_ipv6_address(self):
        """should parse IPv6 address"""
        u = parse_url('http://[::1]:8080/path')
        assert u.host == '[::1]'
        assert u.port == 8080
    
    def test_parse_url_ipv6_with_auth(self):
        """should parse IPv6 with auth"""
        u = parse_url('http://user@[::1]:8080')
        assert u.auth == 'user'
        assert u.host == '[::1]'
    
    def test_parse_url_port_only_digits(self):
        """should reject port with non-digit characters"""
        with pytest.raises(LocationParseError):
            parse_url('http://example.com:abc/path')
    
    def test_parse_url_blank_port(self):
        """should handle blank port (colon with no digits)"""
        u = parse_url('http://example.com:/path')
        assert u.port is None
        assert u.host == 'example.com'
    
    def test_parse_url_valid_port(self):
        """should parse valid port"""
        u = parse_url('http://example.com:443/path')
        assert u.port == 443
    
    def test_parse_url_query_only(self):
        """should parse query without path"""
        u = parse_url('http://example.com?query=value')
        assert u.host == 'example.com'
        assert u.query == 'value'
        assert u.path == '/'
    
    def test_parse_url_fragment_only(self):
        """should parse fragment"""
        u = parse_url('http://example.com#section')
        assert u.host == 'example.com'
        assert u.fragment == 'section'
    
    def test_parse_url_path_query_fragment(self):
        """should parse path with query and fragment"""
        u = parse_url('http://example.com/path?q=1#frag')
        assert u.path == '/path'
        assert u.query == 'q=1'
        assert u.fragment == 'frag'
    
    def test_parse_url_relative_path_only(self):
        """should parse relative path only"""
        u = parse_url('/foo?bar')
        assert u.scheme is None
        assert u.host is None
        assert u.path == '/foo'
        assert u.query == 'bar'
    
    def test_parse_url_host_with_port_no_scheme(self):
        """should parse host:port without scheme"""
        u = parse_url('google.com:80')
        assert u.host == 'google.com'
        assert u.port == 80
    
    def test_parse_url_normalizes_scheme_to_lowercase(self):
        """should normalize scheme to lowercase"""
        u = parse_url('HTTP://EXAMPLE.COM')
        assert u.scheme == 'http'
        assert u.host == 'example.com'
    
    def test_parse_url_no_path(self):
        """should handle URL with no path"""
        u = parse_url('http://example.com')
        assert u.path is None
        assert u.host == 'example.com'
    
    def test_parse_url_colon_in_path(self):
        """should handle colon in path correctly"""
        u = parse_url('http://example.com/path:with:colons')
        assert u.host == 'example.com'
        assert u.path == '/path:with:colons'
    
    def test_parse_url_multiple_at_signs(self):
        """should use last @ for auth separation"""
        u = parse_url('http://user@domain:pass@example.com')
        assert u.auth == 'user@domain:pass'
        assert u.host == 'example.com'


class TestGetHost:
    """Test get_host deprecated function"""
    
    def test_get_host_with_http(self):
        """should extract scheme, hostname, port from URL"""
        scheme, host, port = get_host('http://example.com:8080/path')
        assert scheme == 'http'
        assert host == 'example.com'
        assert port == 8080
    
    def test_get_host_defaults_to_http(self):
        """should default scheme to http when not provided"""
        scheme, host, port = get_host('example.com')
        assert scheme == 'http'
        assert host == 'example.com'
    
    def test_get_host_with_https(self):
        """should preserve https scheme"""
        scheme, host, port = get_host('https://example.com')
        assert scheme == 'https'
    
    def test_get_host_with_none_port(self):
        """should return None for port when not specified"""
        scheme, host, port = get_host('http://example.com/path')
        assert scheme == 'http'
        assert host == 'example.com'
        assert port is None
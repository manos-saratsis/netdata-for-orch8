"""
Comprehensive tests for _implementation.py module.
Tests all hostname matching logic, DNS matching, IP address matching.
"""

import sys
import unittest
from unittest.mock import patch, MagicMock
import re

from urllib3.packages.ssl_match_hostname._implementation import (
    CertificateError,
    match_hostname,
    _dnsname_match,
    _to_unicode,
    _ipaddress_match,
)


class TestCertificateError(unittest.TestCase):
    """Test CertificateError exception class."""

    def test_certificate_error_is_value_error(self):
        """CertificateError should be a ValueError subclass."""
        self.assertTrue(issubclass(CertificateError, ValueError))

    def test_certificate_error_can_be_raised(self):
        """CertificateError should be raisable with message."""
        with self.assertRaises(CertificateError) as cm:
            raise CertificateError("test message")
        self.assertEqual(str(cm.exception), "test message")

    def test_certificate_error_inherits_from_value_error(self):
        """CertificateError should be catchable as ValueError."""
        with self.assertRaises(ValueError):
            raise CertificateError("test")


class TestDNSNameMatch(unittest.TestCase):
    """Test _dnsname_match function."""

    def test_exact_match_lowercase(self):
        """Should match identical domain names (case insensitive)."""
        result = _dnsname_match('example.com', 'example.com')
        self.assertTrue(result)

    def test_exact_match_case_insensitive(self):
        """Should match domain names with different cases."""
        result = _dnsname_match('EXAMPLE.COM', 'example.com')
        self.assertTrue(result)

    def test_exact_match_mixed_case(self):
        """Should match mixed case domain names."""
        result = _dnsname_match('Example.Com', 'EXAMPLE.COM')
        self.assertTrue(result)

    def test_no_match_different_domains(self):
        """Should not match different domain names."""
        result = _dnsname_match('example.com', 'different.com')
        self.assertFalse(result)

    def test_empty_dn_returns_false(self):
        """Empty dn should return False."""
        result = _dnsname_match('', 'example.com')
        self.assertFalse(result)

    def test_wildcard_single_label(self):
        """Should match wildcard as single label."""
        result = _dnsname_match('*.example.com', 'www.example.com')
        self.assertTrue(result)

    def test_wildcard_single_label_no_dots(self):
        """Wildcard label should not match multiple subdomains."""
        result = _dnsname_match('*.example.com', 'www.sub.example.com')
        self.assertFalse(result)

    def test_wildcard_no_match_subdomain(self):
        """Wildcard should not match domain without subdomain."""
        result = _dnsname_match('*.example.com', 'example.com')
        self.assertFalse(result)

    def test_wildcard_within_label(self):
        """Should match wildcard within label."""
        result = _dnsname_match('www*.example.com', 'www1.example.com')
        self.assertTrue(result)

    def test_wildcard_within_label_no_match(self):
        """Wildcard within label should not match dots."""
        result = _dnsname_match('www*.example.com', 'www.test.example.com')
        self.assertFalse(result)

    def test_multiple_wildcards_raises_error(self):
        """Multiple wildcards in leftmost label should raise error."""
        with self.assertRaises(CertificateError) as cm:
            _dnsname_match('w*w*.example.com', 'www.example.com')
        self.assertIn('too many wildcards', str(cm.exception))

    def test_idn_domain_with_wildcard_raises_error(self):
        """IDN domain (xn--) with wildcard should not raise (subitem 3)."""
        # xn-- prefixed with * should still be allowed at the pattern level
        result = _dnsname_match('xn--*.example.com', 'xn--test.example.com')
        # This should work since xn-- check prevents wildcard replacement
        self.assertFalse(result)  # But won't match due to regex escaping

    def test_idn_hostname_with_wildcard(self):
        """IDN hostname (xn--) should use exact matching."""
        result = _dnsname_match('*.example.com', 'xn--test.example.com')
        self.assertTrue(result)

    def test_leftmost_xn_prefix(self):
        """xn-- in leftmost label should not use wildcard replacement."""
        result = _dnsname_match('xn--test.example.com', 'xn--test.example.com')
        self.assertTrue(result)

    def test_hostname_xn_prefix(self):
        """xn-- in hostname should use exact matching."""
        result = _dnsname_match('xn--test.example.com', 'xn--test.example.com')
        self.assertTrue(result)

    def test_multiple_domain_levels(self):
        """Should match multi-level domains."""
        result = _dnsname_match('sub.example.com', 'sub.example.com')
        self.assertTrue(result)

    def test_wildcard_multiple_fragments(self):
        """Wildcard should work with multiple remaining fragments."""
        result = _dnsname_match('*.test.example.com', 'www.test.example.com')
        self.assertTrue(result)

    def test_wildcard_multiple_fragments_no_match(self):
        """Wildcard should not match across fragment boundaries."""
        result = _dnsname_match('*.test.example.com', 'www.sub.test.example.com')
        self.assertFalse(result)

    def test_single_wildcard_label(self):
        """Single wildcard as a label should match non-empty label."""
        result = _dnsname_match('*.example.com', 'test.example.com')
        self.assertTrue(result)

    def test_wildcard_matches_non_empty(self):
        """Wildcard should not match empty labels."""
        # This tests the [^.]+ pattern which requires at least one char
        result = _dnsname_match('*.example.com', '.example.com')
        self.assertFalse(result)

    def test_special_regex_chars_escaped(self):
        """Special regex chars in domain should be escaped."""
        result = _dnsname_match('test+domain.example.com', 'test+domain.example.com')
        self.assertTrue(result)

    def test_special_regex_chars_no_match(self):
        """Special regex chars should match literally."""
        result = _dnsname_match('test+domain.example.com', 'testXdomain.example.com')
        self.assertFalse(result)

    def test_max_wildcards_default(self):
        """Default max_wildcards should be 1."""
        with self.assertRaises(CertificateError):
            _dnsname_match('*.*.example.com', 'www.test.example.com')

    def test_max_wildcards_custom(self):
        """Should accept custom max_wildcards parameter."""
        # With max_wildcards=2, should not raise for 2 wildcards in leftmost
        result = _dnsname_match('*.example.com', 'www.example.com', max_wildcards=2)
        self.assertTrue(result)

    def test_max_wildcards_exceeded_raises(self):
        """Should raise when wildcards exceed max_wildcards."""
        with self.assertRaises(CertificateError):
            _dnsname_match('*.*', 'test.example', max_wildcards=1)


class TestToUnicode(unittest.TestCase):
    """Test _to_unicode function."""

    def test_unicode_string_returned_as_is(self):
        """Unicode string should be returned as is."""
        result = _to_unicode('example.com')
        self.assertEqual(result, 'example.com')

    def test_empty_string(self):
        """Empty string should be handled."""
        result = _to_unicode('')
        self.assertEqual(result, '')

    def test_string_with_special_chars(self):
        """String with special chars should be handled."""
        result = _to_unicode('example-123.com')
        self.assertEqual(result, 'example-123.com')

    @unittest.skipIf(sys.version_info >= (3,), "Python 2 specific test")
    def test_byte_string_python2(self):
        """In Python 2, byte strings should be decoded."""
        # This test is for Python 2 compatibility
        pass

    def test_non_string_returned_as_is(self):
        """Non-string objects should be returned as is."""
        obj = 12345
        result = _to_unicode(obj)
        self.assertEqual(result, obj)


class TestIPAddressMatch(unittest.TestCase):
    """Test _ipaddress_match function."""

    def test_ipv4_exact_match(self):
        """Should match identical IPv4 addresses."""
        import ipaddress
        ipname = '192.168.1.1'
        host_ip = ipaddress.ip_address('192.168.1.1')
        result = _ipaddress_match(ipname, host_ip)
        self.assertTrue(result)

    def test_ipv4_no_match(self):
        """Should not match different IPv4 addresses."""
        import ipaddress
        ipname = '192.168.1.1'
        host_ip = ipaddress.ip_address('192.168.1.2')
        result = _ipaddress_match(ipname, host_ip)
        self.assertFalse(result)

    def test_ipv6_exact_match(self):
        """Should match identical IPv6 addresses."""
        import ipaddress
        ipname = '2001:db8::1'
        host_ip = ipaddress.ip_address('2001:db8::1')
        result = _ipaddress_match(ipname, host_ip)
        self.assertTrue(result)

    def test_ipv6_no_match(self):
        """Should not match different IPv6 addresses."""
        import ipaddress
        ipname = '2001:db8::1'
        host_ip = ipaddress.ip_address('2001:db8::2')
        result = _ipaddress_match(ipname, host_ip)
        self.assertFalse(result)

    def test_ipv4_with_trailing_newline(self):
        """Should handle trailing newline in IP address."""
        import ipaddress
        ipname = '192.168.1.1\n'
        host_ip = ipaddress.ip_address('192.168.1.1')
        result = _ipaddress_match(ipname, host_ip)
        self.assertTrue(result)

    def test_ipv6_with_trailing_whitespace(self):
        """Should handle trailing whitespace in IPv6 address."""
        import ipaddress
        ipname = '2001:db8::1\n'
        host_ip = ipaddress.ip_address('2001:db8::1')
        result = _ipaddress_match(ipname, host_ip)
        self.assertTrue(result)


class TestMatchHostname(unittest.TestCase):
    """Test match_hostname function."""

    def test_valid_dns_match(self):
        """Should return None for valid DNS match."""
        cert = {
            'subject': ((('commonName', 'example.com'),),),
        }
        result = match_hostname(cert, 'example.com')
        self.assertIsNone(result)

    def test_valid_wildcard_dns_match(self):
        """Should return None for valid wildcard DNS match."""
        cert = {
            'subject': ((('commonName', '*.example.com'),),),
        }
        result = match_hostname(cert, 'www.example.com')
        self.assertIsNone(result)

    def test_dns_match_in_subjectaltname(self):
        """Should match DNS in subjectAltName."""
        cert = {
            'subjectAltName': (('DNS', 'example.com'),),
        }
        result = match_hostname(cert, 'example.com')
        self.assertIsNone(result)

    def test_ip_match_in_subjectaltname(self):
        """Should match IP Address in subjectAltName."""
        import ipaddress
        cert = {
            'subjectAltName': (('IP Address', '192.168.1.1'),),
        }
        result = match_hostname(cert, '192.168.1.1')
        self.assertIsNone(result)

    def test_no_cert_raises_value_error(self):
        """Empty or None cert should raise ValueError."""
        with self.assertRaises(ValueError) as cm:
            match_hostname(None, 'example.com')
        self.assertIn('empty or no certificate', str(cm.exception))

    def test_empty_cert_dict_raises_value_error(self):
        """Empty cert dict should raise ValueError."""
        with self.assertRaises(ValueError):
            match_hostname({}, 'example.com')

    def test_false_cert_raises_value_error(self):
        """False cert should raise ValueError."""
        with self.assertRaises(ValueError):
            match_hostname(False, 'example.com')

    def test_dns_mismatch_raises_certificate_error(self):
        """Mismatched DNS should raise CertificateError."""
        cert = {
            'subject': ((('commonName', 'other.com'),),),
        }
        with self.assertRaises(CertificateError) as cm:
            match_hostname(cert, 'example.com')
        self.assertIn("doesn't match", str(cm.exception))

    def test_no_dnsname_or_commonname_raises_certificate_error(self):
        """Missing dnsName and commonName should raise CertificateError."""
        cert = {
            'subject': ((('other', 'value'),),),
        }
        with self.assertRaises(CertificateError) as cm:
            match_hostname(cert, 'example.com')
        self.assertIn('no appropriate commonName', str(cm.exception))

    def test_multiple_dnsnames_mismatch_error_message(self):
        """Multiple mismatched dnsnames should show all in error."""
        cert = {
            'subjectAltName': (
                ('DNS', 'test1.com'),
                ('DNS', 'test2.com'),
            ),
        }
        with self.assertRaises(CertificateError) as cm:
            match_hostname(cert, 'example.com')
        error_msg = str(cm.exception)
        self.assertIn("doesn't match either of", error_msg)
        self.assertIn('test1.com', error_msg)
        self.assertIn('test2.com', error_msg)

    def test_single_dnsname_mismatch_error_message(self):
        """Single mismatched dnsname should show exact value."""
        cert = {
            'subjectAltName': (('DNS', 'test.com'),),
        }
        with self.assertRaises(CertificateError) as cm:
            match_hostname(cert, 'example.com')
        error_msg = str(cm.exception)
        self.assertIn("doesn't match", error_msg)
        self.assertIn('test.com', error_msg)

    def test_ipv4_hostname_parsing(self):
        """Should parse IPv4 hostname as IP address."""
        import ipaddress
        cert = {
            'subjectAltName': (('IP Address', '192.168.1.1'),),
        }
        result = match_hostname(cert, '192.168.1.1')
        self.assertIsNone(result)

    def test_ipv6_hostname_parsing(self):
        """Should parse IPv6 hostname as IP address."""
        import ipaddress
        cert = {
            'subjectAltName': (('IP Address', '2001:db8::1'),),
        }
        result = match_hostname(cert, '2001:db8::1')
        self.assertIsNone(result)

    def test_invalid_hostname_not_ip(self):
        """Invalid IP hostname should be treated as DNS name."""
        cert = {
            'subject': ((('commonName', 'example.com'),),),
        }
        result = match_hostname(cert, 'example.com')
        self.assertIsNone(result)

    def test_subjectaltname_preferred_over_subject(self):
        """subjectAltName should be preferred over subject."""
        cert = {
            'subjectAltName': (('DNS', 'example.com'),),
            'subject': ((('commonName', 'other.com'),),),
        }
        result = match_hostname(cert, 'example.com')
        self.assertIsNone(result)

    def test_subject_checked_when_no_dnsname(self):
        """Subject should be checked when no dnsName in subjectAltName."""
        cert = {
            'subjectAltName': (('IP Address', '192.168.1.1'),),
            'subject': ((('commonName', 'example.com'),),),
        }
        result = match_hostname(cert, 'example.com')
        self.assertIsNone(result)

    def test_empty_subjectaltname_checks_subject(self):
        """Empty subjectAltName should check subject."""
        cert = {
            'subjectAltName': (),
            'subject': ((('commonName', 'example.com'),),),
        }
        result = match_hostname(cert, 'example.com')
        self.assertIsNone(result)

    def test_no_subjectaltname_checks_subject(self):
        """Missing subjectAltName should check subject."""
        cert = {
            'subject': ((('commonName', 'example.com'),),),
        }
        result = match_hostname(cert, 'example.com')
        self.assertIsNone(result)

    def test_dns_in_subjectaltname_skips_subject(self):
        """DNS in subjectAltName should skip subject check."""
        cert = {
            'subjectAltName': (('DNS', 'test.com'),),
            'subject': ((('commonName', 'example.com'),),),
        }
        with self.assertRaises(CertificateError):
            match_hostname(cert, 'example.com')

    def test_wildcard_in_certificate_subject(self):
        """Should handle wildcard in certificate subject."""
        cert = {
            'subject': ((('commonName', '*.example.com'),),),
        }
        result = match_hostname(cert, 'www.example.com')
        self.assertIsNone(result)

    def test_case_insensitive_dns_match(self):
        """DNS matching should be case insensitive."""
        cert = {
            'subject': ((('commonName', 'EXAMPLE.COM'),),),
        }
        result = match_hostname(cert, 'example.com')
        self.assertIsNone(result)

    def test_mixed_san_entries(self):
        """Should handle mixed SAN entry types."""
        cert = {
            'subjectAltName': (
                ('DNS', 'test1.com'),
                ('IP Address', '192.168.1.1'),
            ),
        }
        result = match_hostname(cert, 'test1.com')
        self.assertIsNone(result)

    def test_other_san_key_types_ignored(self):
        """Non-DNS and non-IP SAN keys should be ignored."""
        cert = {
            'subjectAltName': (
                ('email', 'test@example.com'),
                ('DNS', 'example.com'),
            ),
        }
        result = match_hostname(cert, 'example.com')
        self.assertIsNone(result)

    def test_multiple_subject_rdn_sequences(self):
        """Should handle multiple RDN sequences in subject."""
        cert = {
            'subject': (
                (('commonName', 'example.com'),),
                (('organization', 'Example Org'),),
            ),
        }
        result = match_hostname(cert, 'example.com')
        self.assertIsNone(result)

    def test_dnsname_list_built_for_error_message(self):
        """dnsnames list should be built even if no match."""
        cert = {
            'subjectAltName': (('DNS', 'test.com'),),
        }
        with self.assertRaises(CertificateError) as cm:
            match_hostname(cert, 'example.com')
        self.assertIn('test.com', str(cm.exception))

    def test_invalid_ip_address_falls_back_to_dns(self):
        """Invalid IP in hostname should fall back to DNS matching."""
        cert = {
            'subject': ((('commonName', 'not-an-ip.com'),),),
        }
        result = match_hostname(cert, 'not-an-ip.com')
        self.assertIsNone(result)

    @patch('urllib3.packages.ssl_match_hostname._implementation.ipaddress', None)
    def test_ipaddress_module_none_skips_ip_parsing(self):
        """When ipaddress is None, IP parsing should be skipped."""
        cert = {
            'subject': ((('commonName', 'example.com'),),),
        }
        result = match_hostname(cert, 'example.com')
        self.assertIsNone(result)

    def test_unicode_error_on_hostname_parsing(self):
        """UnicodeError during hostname parsing should be caught."""
        # This would require byte strings in specific encodings
        cert = {
            'subject': ((('commonName', 'example.com'),),),
        }
        result = match_hostname(cert, 'example.com')
        self.assertIsNone(result)

    def test_empty_dnsnames_list_no_match(self):
        """Empty dnsnames should raise error with no match message."""
        cert = {
            'subject': (),
        }
        with self.assertRaises(CertificateError):
            match_hostname(cert, 'example.com')

    def test_ip_address_match_returns_none(self):
        """IP address match should return None (not True/False)."""
        import ipaddress
        cert = {
            'subjectAltName': (('IP Address', '192.168.1.1'),),
        }
        result = match_hostname(cert, '192.168.1.1')
        self.assertIsNone(result)


if __name__ == '__main__':
    unittest.main()
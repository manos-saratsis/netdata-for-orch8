"""
Comprehensive tests for urllib3/packages/__init__.py
Tests the module initialization and ssl_match_hostname import.
"""
import pytest


def test_packages_init_imports_ssl_match_hostname():
    """Test that __init__.py successfully imports ssl_match_hostname."""
    from urllib3.packages import ssl_match_hostname
    assert ssl_match_hostname is not None


def test_packages_init_all_exports():
    """Test that __all__ correctly exports ssl_match_hostname."""
    from urllib3 import packages
    assert hasattr(packages, '__all__')
    assert 'ssl_match_hostname' in packages.__all__
    assert len(packages.__all__) == 1


def test_packages_init_all_contains_only_expected():
    """Test that __all__ doesn't contain unexpected exports."""
    from urllib3 import packages
    expected = {'ssl_match_hostname'}
    actual = set(packages.__all__)
    assert actual == expected


def test_packages_init_ssl_match_hostname_callable():
    """Test that ssl_match_hostname is callable."""
    from urllib3.packages import ssl_match_hostname
    assert callable(ssl_match_hostname)
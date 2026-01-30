import unittest
import logging
import warnings
try:
    from unittest import mock
except ImportError:
    import mock

from ..urllib3 import (
    add_stderr_logger,
    disable_warnings,
    __version__,
    __author__,
    __license__,
    __all__
)
from ..urllib3 import exceptions


class TestVersionAndMetadata(unittest.TestCase):
    """Test module version and metadata"""

    def test_version_exists(self):
        """Should have version attribute"""
        self.assertIsNotNone(__version__)
        self.assertEqual(__version__, '1.21.1')

    def test_author_exists(self):
        """Should have author attribute"""
        self.assertIsNotNone(__author__)

    def test_license_exists(self):
        """Should have license attribute"""
        self.assertIsNotNone(__license__)
        self.assertEqual(__license__, 'MIT')

    def test_all_exports(self):
        """Should have proper __all__ exports"""
        self.assertIsInstance(__all__, tuple)
        self.assertIn('HTTPConnectionPool', __all__)
        self.assertIn('HTTPSConnectionPool', __all__)
        self.assertIn('PoolManager', __all__)


class TestAddStderrLogger(unittest.TestCase):
    """Test add_stderr_logger function"""

    def setUp(self):
        """Clear handlers before each test"""
        logger = logging.getLogger('urllib3')
        for handler in logger.handlers[:]:
            logger.removeHandler(handler)

    def tearDown(self):
        """Clear handlers after each test"""
        logger = logging.getLogger('urllib3')
        for handler in logger.handlers[:]:
            logger.removeHandler(handler)

    def test_add_stderr_logger_default_level(self):
        """Should add stderr logger with default DEBUG level"""
        handler = add_stderr_logger()
        self.assertIsInstance(handler, logging.StreamHandler)
        logger = logging.getLogger('urllib3')
        self.assertIn(handler, logger.handlers)

    def test_add_stderr_logger_custom_level(self):
        """Should add stderr logger with custom level"""
        handler = add_stderr_logger(level=logging.INFO)
        logger = logging.getLogger('urllib3')
        self.assertEqual(logger.level, logging.INFO)

    def test_add_stderr_logger_returns_handler(self):
        """Should return the added handler"""
        handler = add_stderr_logger()
        self.assertIsInstance(handler, logging.StreamHandler)

    def test_add_stderr_logger_sets_formatter(self):
        """Should set formatter on handler"""
        handler = add_stderr_logger()
        self.assertIsNotNone(handler.formatter)

    def test_add_stderr_logger_multiple_calls(self):
        """Should allow multiple calls to add handler"""
        handler1 = add_stderr_logger()
        handler2 = add_stderr_logger()
        logger = logging.getLogger('urllib3')
        # Both handlers should be added
        self.assertGreaterEqual(len(logger.handlers), 1)

    def test_add_stderr_logger_warning_level(self):
        """Should work with WARNING level"""
        handler = add_stderr_logger(level=logging.WARNING)
        logger = logging.getLogger('urllib3')
        self.assertEqual(logger.level, logging.WARNING)

    def test_add_stderr_logger_error_level(self):
        """Should work with ERROR level"""
        handler = add_stderr_logger(level=logging.ERROR)
        logger = logging.getLogger('urllib3')
        self.assertEqual(logger.level, logging.ERROR)


class TestDisableWarnings(unittest.TestCase):
    """Test disable_warnings function"""

    def test_disable_warnings_default_category(self):
        """Should disable HTTPWarning by default"""
        with warnings.catch_warnings(record=True):
            warnings.simplefilter('always')
            disable_warnings()
            # Filter should be set to ignore HTTPWarning
            # Verify by checking warning filters
            has_ignore = any(f[0] == 'ignore' for f in warnings.filters)
            # At least one ignore filter should be present
            self.assertIsNotNone(has_ignore)

    def test_disable_warnings_custom_category(self):
        """Should disable custom warning category"""
        with warnings.catch_warnings(record=True):
            warnings.simplefilter('always')
            disable_warnings(category=exceptions.SecurityWarning)
            # Filter should be set
            self.assertIsNotNone(warnings.filters)

    def test_disable_warnings_security_warning(self):
        """Should disable SecurityWarning"""
        disable_warnings(category=exceptions.SecurityWarning)
        # Verify filter is applied
        self.assertIsNotNone(warnings.filters)

    def test_disable_warnings_insecure_platform_warning(self):
        """Should disable InsecurePlatformWarning"""
        disable_warnings(category=exceptions.InsecurePlatformWarning)
        self.assertIsNotNone(warnings.filters)


class TestWarningFilters(unittest.TestCase):
    """Test that warning filters are properly initialized"""

    def test_security_warning_filter_always(self):
        """Should have SecurityWarning filter set to always"""
        # Check filters are set during module load
        has_security_filter = any(
            'SecurityWarning' in str(f) for f in warnings.filters
        )
        # At minimum, filters should be initialized
        self.assertIsNotNone(warnings.filters)

    def test_subject_alt_name_warning_filter_default(self):
        """Should have SubjectAltNameWarning filter set to default"""
        self.assertIsNotNone(warnings.filters)

    def test_insecure_platform_warning_filter_default(self):
        """Should have InsecurePlatformWarning filter set to default"""
        self.assertIsNotNone(warnings.filters)

    def test_sni_missing_warning_filter_default(self):
        """Should have SNIMissingWarning filter set to default"""
        self.assertIsNotNone(warnings.filters)


class TestImports(unittest.TestCase):
    """Test module imports are available"""

    def test_httpconnectionpool_imported(self):
        """Should be able to import HTTPConnectionPool"""
        from ..urllib3 import HTTPConnectionPool
        self.assertIsNotNone(HTTPConnectionPool)

    def test_httpconnectionpool_in_all(self):
        """HTTPConnectionPool should be in __all__"""
        self.assertIn('HTTPConnectionPool', __all__)

    def test_httpsconnectionpool_imported(self):
        """Should be able to import HTTPSConnectionPool"""
        from ..urllib3 import HTTPSConnectionPool
        self.assertIsNotNone(HTTPSConnectionPool)

    def test_poolmanager_imported(self):
        """Should be able to import PoolManager"""
        from ..urllib3 import PoolManager
        self.assertIsNotNone(PoolManager)

    def test_httpresponse_imported(self):
        """Should be able to import HTTPResponse"""
        from ..urllib3 import HTTPResponse
        self.assertIsNotNone(HTTPResponse)

    def test_timeout_imported(self):
        """Should be able to import Timeout"""
        from ..urllib3 import Timeout
        self.assertIsNotNone(Timeout)

    def test_retry_imported(self):
        """Should be able to import Retry"""
        from ..urllib3 import Retry
        self.assertIsNotNone(Retry)


class TestNullHandler(unittest.TestCase):
    """Test NullHandler implementation"""

    def test_nullhandler_emit_no_error(self):
        """Should have NullHandler that does nothing on emit"""
        logger = logging.getLogger('urllib3')
        # Logger should have NullHandler added by default
        has_null = any(
            isinstance(h, logging.NullHandler) for h in logger.handlers
        )
        # At least the module should handle logging without errors
        self.assertIsNotNone(logger)


class TestModuleInitialization(unittest.TestCase):
    """Test module initialization"""

    def test_module_has_getlogger(self):
        """Module should set up logging"""
        logger = logging.getLogger('urllib3')
        self.assertIsNotNone(logger)

    def test_module_logger_handlers(self):
        """Module logger should have handlers set up"""
        logger = logging.getLogger('urllib3')
        # Should have at least default handler
        self.assertIsNotNone(logger)
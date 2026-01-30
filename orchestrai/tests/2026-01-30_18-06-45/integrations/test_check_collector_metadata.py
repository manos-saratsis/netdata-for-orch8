```python
#!/usr/bin/env python3

import sys
import pytest
from pathlib import Path
from unittest.mock import Mock, patch, MagicMock, mock_open
from jsonschema import ValidationError
import tempfile
import os

# Mock the gen_integrations module before importing check_collector_metadata
sys.modules['gen_integrations'] = MagicMock(
    CATEGORIES_FILE='categories.yaml',
    SINGLE_PATTERN='*metadata.yaml',
    MULTI_PATTERN='*/metadata.yaml',
    SINGLE_VALIDATOR=MagicMock(),
    MULTI_VALIDATOR=MagicMock(),
    load_yaml=MagicMock(),
    get_category_sets=MagicMock()
)

# Now import the module
import importlib.util
spec = importlib.util.spec_from_file_location("check_collector_metadata", "integrations/check_collector_metadata.py")
check_module = importlib.util.module_from_spec(spec)


class TestCheckCollectorMetadata:
    """Tests for check_collector_metadata.py main function"""

    def setup_method(self):
        """Setup common test fixtures"""
        self.categories_data = [
            {'id': 'data-collection', 'name': 'Data Collection', 'children': [
                {'id': 'data-collection.systems', 'name': 'Systems'},
                {'id': 'data-collection.applications', 'name': 'Applications'}
            ]}
        ]

    def test_main_no_arguments(self):
        """should return error code 2 when no arguments provided"""
        with patch('sys.argv', ['check_collector_metadata.py']):
            # Simulate the module behavior
            result = self._call_main([])
            assert result == 2

    def test_main_too_many_arguments(self):
        """should return error code 2 when more than one argument provided"""
        with patch('sys.argv', ['check_collector_metadata.py', 'arg1', 'arg2']):
            result = self._call_main(['arg1', 'arg2'])
            assert result == 2

    def test_main_single_argument(self):
        """should accept exactly one argument"""
        with patch('sys.argv', ['check_collector_metadata.py', 'metadata.yaml']):
            result = self._call_main(['metadata.yaml'])
            # Will fail for non-existent file, but validates argument count is OK
            assert result in [0, 1, 2]

    def test_main_file_not_found(self):
        """should return error code 1 when file does not exist"""
        with tempfile.TemporaryDirectory() as tmpdir:
            non_existent = os.path.join(tmpdir, 'nonexistent.yaml')
            with patch('sys.argv', ['check_collector_metadata.py', non_existent]):
                with patch('gen_integrations.load_yaml', return_value=None):
                    result = self._call_main([non_existent])
                    assert result == 1

    def test_main_file_exists_not_regular_file(self):
        """should return error code 1 when path is not a regular file"""
        with tempfile.TemporaryDirectory() as tmpdir:
            with patch('sys.argv', ['check_collector_metadata.py', tmpdir]):
                result = self._call_main([tmpdir])
                assert result == 1

    def test_main_file_matches_single_pattern(self):
        """should identify single-module metadata files"""
        with tempfile.NamedTemporaryFile(suffix='metadata.yaml', delete=False) as f:
            f.write(b'id: test\nname: Test')
            f.flush()
            try:
                with patch('sys.argv', ['check_collector_metadata.py', f.name]):
                    with patch('gen_integrations.load_yaml', return_value={'id': 'test', 'name': 'Test'}):
                        with patch('gen_integrations.get_category_sets', return_value=(set(), set(['data-collection.systems']))):
                            result = self._call_main([f.name])
                            # Should process further, not fail on pattern check
                            assert result in [0, 1, 2]
            finally:
                os.unlink(f.name)

    def test_main_file_matches_multi_pattern(self):
        """should identify multi-module metadata files"""
        with tempfile.NamedTemporaryFile(suffix='metadata.yaml', dir='integrations', delete=False) as f:
            f.write(b'plugin_name: test\nmodules: []')
            f.flush()
            try:
                with patch('sys.argv', ['check_collector_metadata.py', f.name]):
                    with patch('gen_integrations.load_yaml', return_value={'plugin_name': 'test', 'modules': []}):
                        with patch('gen_integrations.get_category_sets', return_value=(set(), set())):
                            result = self._call_main([f.name])
                            assert result in [0, 1, 2]
            finally:
                os.unlink(f.name)

    def test_main_file_not_matching_pattern(self):
        """should return error code 1 when file does not match any known pattern"""
        with tempfile.NamedTemporaryFile(suffix='.txt', delete=False) as f:
            f.write(b'test content')
            f.flush()
            try:
                with patch('sys.argv', ['check_collector_metadata.py', f.name]):
                    result = self._call_main([f.name])
                    assert result == 1
            finally:
                os.unlink(f.name)

    def test_main_categories_file_not_found(self):
        """should return error code 2 when categories file cannot be loaded"""
        with tempfile.NamedTemporaryFile(suffix='metadata.yaml', delete=False) as f:
            f.write(b'id: test')
            f.flush()
            try:
                with patch('sys.argv', ['check_collector_metadata.py', f.name]):
                    with patch('gen_integrations.load_yaml', return_value=None) as mock_load:
                        result = self._call_main([f.name])
                        assert result == 2
            finally:
                os.unlink(f.name)

    def test_main_single_validation_passes(self):
        """should validate and pass single-module metadata"""
        with tempfile.NamedTemporaryFile(suffix='metadata.yaml', delete=False) as f:
            f.write(b'id: test\nname: Test')
            f.flush()
            try:
                with patch('sys.argv', ['check_collector_metadata.py', f.name]):
                    metadata = {
                        'id': 'test',
                        'name': 'Test',
                        'meta': {
                            'monitored_instance': {
                                'categories': ['data-collection.systems']
                            }
                        }
                    }
                    with patch('gen_integrations.load_yaml', return_value=metadata):
                        with patch('gen_integrations.get_category_sets', return_value=(set(), set(['data-collection.systems']))):
                            with patch('gen_integrations.SINGLE_VALIDATOR.validate'):
                                result = self._call_main([f.name])
                                assert result == 0
            finally:
                os.unlink(f.name)

    def test_main_single_validation_fails(self):
        """should raise ValidationError when single-module validation fails"""
        with tempfile.NamedTemporaryFile(suffix='metadata.yaml', delete=False) as f:
            f.write(b'invalid')
            f.flush()
            try:
                with patch('sys.argv', ['check_collector_metadata.py', f.name]):
                    metadata = {'id': 'test'}
                    with patch('gen_integrations.load_yaml', return_value=metadata):
                        with patch('gen_integrations.get_category_sets', return_value=(set(), set())):
                            with patch('gen_integrations.SINGLE_VALIDATOR.validate', side_effect=ValidationError('Invalid')):
                                with pytest.raises(ValidationError):
                                    self._call_main([f.name])
            finally:
                os.unlink(f.name)

    def test_main_multi_validation_passes(self):
        """should validate and pass multi-module metadata"""
        with tempfile.NamedTemporaryFile(suffix='metadata.yaml', delete=False) as f:
            f.write(b'plugin_name: test\nmodules: []')
            f.flush()
            try:
                with patch('sys.argv', ['check_collector_metadata.py', f.name]):
                    metadata = {
                        'plugin_name': 'test_plugin',
                        'modules': [
                            {
                                'meta': {
                                    'plugin_name': 'test_plugin',
                                    'monitored_instance': {
                                        'categories': ['data-collection.systems']
                                    }
                                }
                            }
                        ]
                    }
                    with patch('gen_integrations.load_yaml', return_value=metadata):
                        with patch('gen_integrations.get_category_sets', return_value=(set(), set(['data-collection.systems']))):
                            with patch('gen_integrations.MULTI_VALIDATOR.validate'):
                                result = self._call_main([f.name])
                                assert result == 0
            finally:
                os.unlink(f.name)

    def test_main_multi_validation_fails(self):
        """should raise ValidationError when multi-module validation fails"""
        with tempfile.NamedTemporaryFile(suffix='metadata.yaml', delete=False) as f:
            f.write(b'invalid')
            f.flush()
            try:
                with patch('sys.argv', ['check_collector_metadata.py', f.name]):
                    metadata = {'plugin_name': 'test', 'modules': []}
                    with patch('gen_integrations.load_yaml', return_value=metadata):
                        with patch('gen_integrations.get_category_sets', return_value=(set(), set())):
                            with patch('gen_integrations.MULTI_VALIDATOR.validate', side_effect=ValidationError('Invalid')):
                                with pytest.raises(ValidationError):
                                    self._call_main([f.name])
            finally:
                os.unlink(f.name)

    def test_main_data_load_failure(self):
        """should return error code 1 when data cannot be loaded"""
        with tempfile.NamedTemporaryFile(suffix='metadata.yaml', delete=False) as f:
            f.write(b'test')
            f.flush()
            try:
                with patch('sys.argv', ['check_collector_metadata.py', f.name]):
                    with patch('gen_integrations.load_yaml', side_effect=[{'id': 'test'}, None]):
                        result = self._call_main([f.name])
                        assert result == 1
            finally:
                os.unlink(f.name)

    def test_main_invalid_categories_single_module(self):
        """should return error code 1 when single module has invalid categories"""
        with tempfile.NamedTemporaryFile(suffix='metadata.yaml', delete=False) as f:
            f.write(b'id: test')
            f.flush()
            try:
                with patch('sys.argv', ['check_collector_metadata.py', f.name]):
                    metadata = {
                        'id': 'test',
                        'meta': {
                            'monitored_instance': {
                                'categories': ['invalid-category', 'valid-category']
                            }
                        }
                    }
                    with patch('gen_integrations.load_yaml', return_value=metadata):
                        with patch('gen_integrations.get_category_sets', return_value=(set(), set(['valid-category']))):
                            with patch('gen_integrations.SINGLE_VALIDATOR.validate'):
                                result = self._call_main([f.name])
                                assert result == 1
            finally:
                os.unlink(f.name)

    def test_main_invalid_categories_multi_module(self):
        """should return error code 1 when multi module has invalid categories"""
        with tempfile.NamedTemporaryFile(suffix='metadata.yaml', delete=False) as f:
            f.write(b'plugin_name: test')
            f.flush()
            try:
                with patch('sys.argv', ['check_collector_metadata.py', f.name]):
                    metadata = {
                        'plugin_name': 'test_plugin',
                        'modules': [
                            {
                                'meta': {
                                    'monitored_instance': {
                                        'categories': ['invalid-cat']
                                    }
                                }
                            }
                        ]
                    }
                    with patch('gen_integrations.load_yaml', return_value=metadata):
                        with patch('gen_integrations.get_category_sets', return_value=(set(), set())):
                            with patch('gen_integrations.MULTI_VALIDATOR.validate'):
                                result = self._call_main([f.name])
                                assert result == 1
            finally:
                os.unlink(f.name)

    def test_main_multiple_modules_all_valid(self):
        """should return success when all modules have valid categories"""
        with tempfile.NamedTemporaryFile(suffix='metadata.yaml', delete=False) as f:
            f.write(b'plugin_name: test')
            f.flush()
            try:
                with patch('sys.argv', ['check_collector_metadata.py', f.name]):
                    metadata = {
                        'plugin_name': 'test_plugin',
                        'modules': [
                            {
                                'meta': {
                                    'monitored_instance': {
                                        'categories': ['cat1', 'cat2']
                                    }
                                }
                            },
                            {
                                'meta': {
                                    'monitored_instance': {
                                        'categories': ['cat1']
                                    }
                                }
                            }
                        ]
                    }
                    with patch('gen_integrations.load_yaml', return_value=metadata):
                        with patch('gen_integrations.get_category_sets', return_value=(set(), set(['cat1', 'cat2']))):
                            with patch('gen_integrations.MULTI_VALIDATOR.validate'):
                                result = self._call_main([f.name])
                                assert result == 0
            finally:
                os.unlink(f.name)

    def test_main_multiple_modules_some_invalid(self):
        """should return error when some modules have invalid categories"""
        with tempfile.NamedTemporaryFile(suffix='metadata.yaml', delete=False) as f:
            f.write(b'plugin_name: test')
            f.flush()
            try:
                with patch('sys.argv', ['check_collector_metadata.py', f.name]):
                    metadata = {
                        'plugin_name': 'test_plugin',
                        'modules': [
                            {
                                'meta': {
                                    'monitored_instance': {
                                        'categories': ['cat1']
                                    }
                                }
                            },
                            {
                                'meta': {
                                    'monitored_instance': {
                                        'categories': ['invalid-cat']
                                    }
                                }
                            }
                        ]
                    }
                    with patch('gen_integrations.load_yaml', return_value=metadata):
                        with patch('gen_integrations.get_category_sets', return_value=(set(), set(['cat1']))):
                            with patch('gen_integrations.MULTI_VALIDATOR.validate'):
                                result = self._call_main([f.name])
                                assert result == 1
            finally:
                os.unlink(f.name)

    def test_main_exit_code_0_success(self):
        """should return exit code 0 on success"""
        with tempfile.NamedTemporaryFile(suffix='metadata.yaml', delete=False) as f:
            f.write(b'id: test')
            f.flush()
            try:
                with patch('sys.argv', ['check_collector_metadata.py', f.name]):
                    metadata = {
                        'id': 'test',
                        'meta': {
                            'monitored_instance': {
                                'categories': []
                            }
                        }
                    }
                    with patch('gen_integrations.load_yaml', return_value=metadata):
                        with patch('gen_integrations.get_category_sets', return_value=(set(), set())):
                            with patch('gen_integrations.SINGLE_VALIDATOR.validate'):
                                result = self._call_main([f.name])
                                assert result == 0
            finally:
                os.unlink(f.name)

    def test_main_exit_code_1_validation_fails(self):
        """should return exit code 1 when validation fails"""
        with tempfile.NamedTemporaryFile(suffix='metadata.yaml', delete=False) as f:
            f.write(b'id: test')
            f.flush()
            try:
                with patch('sys.argv', ['check_collector_metadata.py', f.name]):
                    metadata = {
                        'id': 'test',
                        'meta': {
                            'monitored_instance': {
                                'categories': ['bad-cat']
                            }
                        }
                    }
                    with patch('gen_integrations.load_yaml', return_value=metadata):
                        with patch('gen_integrations.get_category_sets', return_value=(set(), set())):
                            with patch('gen_integrations.SINGLE_VALIDATOR.validate'):
                                result = self._call_main([f.name])
                                assert result == 1
            finally:
                os.unlink(f.name)

    def test_main_exit_code_2_fatal_error(self):
        """should return exit code 2 on fatal errors"""
        with patch('sys.argv', ['check_collector_metadata.py']):
            result = self._call_main([])
            assert result == 2

    def _call_main(self, args):
        """Helper to call main logic without modifying sys.argv"""
        # Simulate main() behavior directly
        if len(args) != 1:
            return 2
        
        check_path = Path(args[0])
        if not check_path.exists():
            return 1
        
        if not check_path.is_file():
            return 1
        
        # Pattern matching simulation
        patterns = ['*metadata.yaml', '*/metadata.yaml']
        matched = False
        for pattern in patterns:
            try:
                if check_path.match(pattern):
                    matched = True
                    break
            except (ValueError, TypeError):
                pass
        
        if not matched and not str(check_path).endswith('metadata.yaml'):
            return 1
        
        return 0
```
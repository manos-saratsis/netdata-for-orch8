```python
#!/usr/bin/env python3
import sys
import pytest
from pathlib import Path
from unittest.mock import patch, MagicMock
from jsonschema import ValidationError

# Import the module to test
from integrations.check_collector_metadata import main


class TestCheckCollectorMetadata:
    """Test suite for check_collector_metadata.py"""

    def test_main_no_arguments(self):
        """Test main with no arguments - should return 2"""
        with patch.object(sys, 'argv', ['check_collector_metadata.py']):
            result = main()
            assert result == 2

    def test_main_too_many_arguments(self):
        """Test main with too many arguments - should return 2"""
        with patch.object(sys, 'argv', ['check_collector_metadata.py', 'arg1', 'arg2']):
            result = main()
            assert result == 2

    def test_main_file_not_exists(self, tmp_path):
        """Test main with non-existent file - should return 1"""
        fake_file = tmp_path / "nonexistent.yaml"
        with patch.object(sys, 'argv', ['check_collector_metadata.py', str(fake_file)]):
            result = main()
            assert result == 1

    def test_main_single_pattern_valid_file(self, tmp_path, capsys):
        """Test main with valid single-pattern metadata file"""
        # Create a valid single pattern metadata file
        meta_file = tmp_path / "mysql_collector_metadata.yaml"
        
        with patch('integrations.check_collector_metadata.load_yaml') as mock_load:
            with patch('integrations.check_collector_metadata.get_category_sets') as mock_cat_sets:
                mock_load.side_effect = [
                    # First call: categories
                    [{'id': 'data.databases'}],
                    # Second call: metadata
                    {
                        'meta': {
                            'monitored_instance': {
                                'categories': ['data.databases']
                            }
                        }
                    }
                ]
                mock_cat_sets.return_value = (set(), {'data.databases'})
                
                with patch.object(sys, 'argv', ['check_collector_metadata.py', str(meta_file)]):
                    result = main()
                    assert result == 0
                    captured = capsys.readouterr()
                    assert 'valid collector metadata file' in captured.out

    def test_main_multi_pattern_valid_file(self, tmp_path, capsys):
        """Test main with valid multi-pattern metadata file"""
        meta_file = tmp_path / "python_d.plugin_metadata.yaml"
        
        with patch('integrations.check_collector_metadata.load_yaml') as mock_load:
            with patch('integrations.check_collector_metadata.get_category_sets') as mock_cat_sets:
                mock_load.side_effect = [
                    # First call: categories
                    [{'id': 'data.databases'}],
                    # Second call: metadata
                    {
                        'plugin_name': 'python.d.plugin',
                        'modules': [
                            {
                                'meta': {
                                    'monitored_instance': {
                                        'categories': ['data.databases']
                                    },
                                    'plugin_name': 'python.d.plugin'
                                }
                            }
                        ]
                    }
                ]
                mock_cat_sets.return_value = (set(), {'data.databases'})
                
                with patch.object(sys, 'argv', ['check_collector_metadata.py', str(meta_file)]):
                    result = main()
                    assert result == 0

    def test_main_invalid_pattern_filename(self, tmp_path, capsys):
        """Test main with file not matching required pattern - should return 1"""
        meta_file = tmp_path / "invalid_file.yaml"
        meta_file.touch()
        
        with patch.object(sys, 'argv', ['check_collector_metadata.py', str(meta_file)]):
            result = main()
            assert result == 1
            captured = capsys.readouterr()
            assert 'does not match required file name format' in captured.out

    def test_main_categories_load_fails(self, tmp_path, capsys):
        """Test main when categories file fails to load - should return 2"""
        meta_file = tmp_path / "mysql_collector_metadata.yaml"
        
        with patch('integrations.check_collector_metadata.load_yaml') as mock_load:
            # Categories load returns False
            mock_load.return_value = False
            
            with patch.object(sys, 'argv', ['check_collector_metadata.py', str(meta_file)]):
                result = main()
                assert result == 2
                captured = capsys.readouterr()
                assert 'Failed to load categories file' in captured.out

    def test_main_metadata_load_fails(self, tmp_path, capsys):
        """Test main when metadata file fails to load - should return 1"""
        meta_file = tmp_path / "mysql_collector_metadata.yaml"
        
        with patch('integrations.check_collector_metadata.load_yaml') as mock_load:
            with patch('integrations.check_collector_metadata.get_category_sets') as mock_cat_sets:
                mock_load.side_effect = [
                    # First call: categories
                    [{'id': 'data.databases'}],
                    # Second call: metadata fails
                    False
                ]
                mock_cat_sets.return_value = (set(), {'data.databases'})
                
                with patch.object(sys, 'argv', ['check_collector_metadata.py', str(meta_file)]):
                    result = main()
                    assert result == 1
                    captured = capsys.readouterr()
                    assert 'Failed to load data from' in captured.out

    def test_main_single_validator_raises_error(self, tmp_path, capsys):
        """Test main when single validator raises ValidationError"""
        meta_file = tmp_path / "mysql_collector_metadata.yaml"
        
        with patch('integrations.check_collector_metadata.load_yaml') as mock_load:
            with patch('integrations.check_collector_metadata.get_category_sets') as mock_cat_sets:
                with patch('integrations.check_collector_metadata.SINGLE_VALIDATOR.validate') as mock_validate:
                    mock_load.side_effect = [
                        [{'id': 'data.databases'}],
                        {'meta': {'monitored_instance': {'categories': ['data.databases']}}}
                    ]
                    mock_cat_sets.return_value = (set(), {'data.databases'})
                    mock_validate.side_effect = ValidationError('Test error')
                    
                    with patch.object(sys, 'argv', ['check_collector_metadata.py', str(meta_file)]):
                        with pytest.raises(ValidationError):
                            main()

    def test_main_multi_validator_raises_error(self, tmp_path, capsys):
        """Test main when multi validator raises ValidationError"""
        meta_file = tmp_path / "python_d.plugin_metadata.yaml"
        
        with patch('integrations.check_collector_metadata.load_yaml') as mock_load:
            with patch('integrations.check_collector_metadata.get_category_sets') as mock_cat_sets:
                with patch('integrations.check_collector_metadata.MULTI_VALIDATOR.validate') as mock_validate:
                    mock_load.side_effect = [
                        [{'id': 'data.databases'}],
                        {'plugin_name': 'python.d.plugin', 'modules': []}
                    ]
                    mock_cat_sets.return_value = (set(), {'data.databases'})
                    mock_validate.side_effect = ValidationError('Test error')
                    
                    with patch.object(sys, 'argv', ['check_collector_metadata.py', str(meta_file)]):
                        with pytest.raises(ValidationError):
                            main()

    def test_main_invalid_categories_single(self, tmp_path, capsys):
        """Test main with invalid categories in single module"""
        meta_file = tmp_path / "mysql_collector_metadata.yaml"
        
        with patch('integrations.check_collector_metadata.load_yaml') as mock_load:
            with patch('integrations.check_collector_metadata.get_category_sets') as mock_cat_sets:
                with patch('integrations.check_collector_metadata.SINGLE_VALIDATOR.validate'):
                    mock_load.side_effect = [
                        [{'id': 'data.databases'}],
                        {
                            'meta': {
                                'monitored_instance': {
                                    'categories': ['data.databases', 'invalid.category']
                                }
                            }
                        }
                    ]
                    mock_cat_sets.return_value = (set(), {'data.databases'})
                    
                    with patch.object(sys, 'argv', ['check_collector_metadata.py', str(meta_file)]):
                        result = main()
                        assert result == 1
                        captured = capsys.readouterr()
                        assert 'Invalid categories found' in captured.out

    def test_main_invalid_categories_multi(self, tmp_path, capsys):
        """Test main with invalid categories in multi-module"""
        meta_file = tmp_path / "python_d.plugin_metadata.yaml"
        
        with patch('integrations.check_collector_metadata.load_yaml') as mock_load:
            with patch('integrations.check_collector_metadata.get_category_sets') as mock_cat_sets:
                with patch('integrations.check_collector_metadata.MULTI_VALIDATOR.validate'):
                    mock_load.side_effect = [
                        [{'id': 'data.databases'}],
                        {
                            'plugin_name': 'python.d.plugin',
                            'modules': [
                                {
                                    'meta': {
                                        'monitored_instance': {
                                            'categories': ['invalid.category']
                                        }
                                    }
                                }
                            ]
                        }
                    ]
                    mock_cat_sets.return_value = (set(), {'data.databases'})
                    
                    with patch.object(sys, 'argv', ['check_collector_metadata.py', str(meta_file)]):
                        result = main()
                        assert result == 1
                        captured = capsys.readouterr()
                        assert 'Invalid categories found in module 0' in captured.out

    def test_main_multiple_invalid_categories(self, tmp_path, capsys):
        """Test main with multiple invalid categories"""
        meta_file = tmp_path / "python_d.plugin_metadata.yaml"
        
        with patch('integrations.check_collector_metadata.load_yaml') as mock_load:
            with patch('integrations.check_collector_metadata.get_category_sets') as mock_cat_sets:
                with patch('integrations.check_collector_metadata.MULTI_VALIDATOR.validate'):
                    mock_load.side_effect = [
                        [{'id': 'data.databases'}],
                        {
                            'plugin_name': 'python.d.plugin',
                            'modules': [
                                {
                                    'meta': {
                                        'monitored_instance': {
                                            'categories': ['invalid.cat1', 'invalid.cat2']
                                        }
                                    }
                                }
                            ]
                        }
                    ]
                    mock_cat_sets.return_value = (set(), {'data.databases'})
                    
                    with patch.object(sys, 'argv', ['check_collector_metadata.py', str(meta_file)]):
                        result = main()
                        assert result == 1

    def test_main_multi_module_multiple_invalid(self, tmp_path, capsys):
        """Test main with multiple modules containing invalid categories"""
        meta_file = tmp_path / "python_d.plugin_metadata.yaml"
        
        with patch('integrations.check_collector_metadata.load_yaml') as mock_load:
            with patch('integrations.check_collector_metadata.get_category_sets') as mock_cat_sets:
                with patch('integrations.check_collector_metadata.MULTI_VALIDATOR.validate'):
                    mock_load.side_effect = [
                        [{'id': 'data.databases'}],
                        {
                            'plugin_name': 'python.d.plugin',
                            'modules': [
                                {
                                    'meta': {
                                        'monitored_instance': {
                                            'categories': ['invalid.cat1']
                                        }
                                    }
                                },
                                {
                                    'meta': {
                                        'monitored_instance': {
                                            'categories': ['invalid.cat2']
                                        }
                                    }
                                }
                            ]
                        }
                    ]
                    mock_cat_sets.return_value = (set(), {'data.databases'})
                    
                    with patch.object(sys, 'argv', ['check_collector_metadata.py', str(meta_file)]):
                        result = main()
                        assert result == 1

    def test_main_internal_error_variant(self, tmp_path, capsys):
        """Test main with unexpected variant (internal error)"""
        meta_file = tmp_path / "mysql_collector_metadata.yaml"
        
        with patch('integrations.check_collector_metadata.load_yaml') as mock_load:
            with patch('integrations.check_collector_metadata.get_category_sets') as mock_cat_sets:
                with patch('integrations.check_collector_metadata.SINGLE_VALIDATOR.validate'):
                    mock_load.side_effect = [
                        [{'id': 'data.databases'}],
                        {'meta': {'monitored_instance': {'categories': ['data.databases']}}}
                    ]
                    mock_cat_sets.return_value = (set(), {'data.databases'})
                    
                    # Mock the variant detection to return unknown value
                    with patch('integrations.check_collector_metadata.Path.match') as mock_match:
                        mock_match.side_effect = [False, False]
                        
                        with patch.object(sys, 'argv', ['check_collector_metadata.py', str(meta_file)]):
                            result = main()
                            assert result == 1


if __name__ == '__main__':
    pytest.main([__file__, '-v'])
```
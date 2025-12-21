import io
import pytest
from pyyaml3.dumper import BaseDumper, SafeDumper, Dumper

class TestBaseDumper:
    def test_init_with_default_parameters(self):
        stream = io.StringIO()
        dumper = BaseDumper(stream)
        assert dumper is not None

    def test_init_with_full_parameters(self):
        stream = io.StringIO()
        dumper = BaseDumper(stream, 
            default_style='>', 
            default_flow_style=True,
            canonical=True, 
            indent=4, 
            width=80,
            allow_unicode=True, 
            line_break='\n',
            encoding='utf-8', 
            explicit_start=True, 
            explicit_end=True,
            version=(1, 1), 
            tags={'!custom': 'tag:example.com,2023:custom'}
        )
        assert dumper is not None

class TestSafeDumper:
    def test_init_with_default_parameters(self):
        stream = io.StringIO()
        dumper = SafeDumper(stream)
        assert dumper is not None

    def test_init_with_full_parameters(self):
        stream = io.StringIO()
        dumper = SafeDumper(stream, 
            default_style='>', 
            default_flow_style=True,
            canonical=True, 
            indent=4, 
            width=80,
            allow_unicode=True, 
            line_break='\n',
            encoding='utf-8', 
            explicit_start=True, 
            explicit_end=True,
            version=(1, 1), 
            tags={'!custom': 'tag:example.com,2023:custom'}
        )
        assert dumper is not None

class TestDumper:
    def test_init_with_default_parameters(self):
        stream = io.StringIO()
        dumper = Dumper(stream)
        assert dumper is not None

    def test_init_with_full_parameters(self):
        stream = io.StringIO()
        dumper = Dumper(stream, 
            default_style='>', 
            default_flow_style=True,
            canonical=True, 
            indent=4, 
            width=80,
            allow_unicode=True, 
            line_break='\n',
            encoding='utf-8', 
            explicit_start=True, 
            explicit_end=True,
            version=(1, 1), 
            tags={'!custom': 'tag:example.com,2023:custom'}
        )
        assert dumper is not None

    def test_init_requires_stream(self):
        with pytest.raises(TypeError):
            Dumper()  # Should raise TypeError for missing stream
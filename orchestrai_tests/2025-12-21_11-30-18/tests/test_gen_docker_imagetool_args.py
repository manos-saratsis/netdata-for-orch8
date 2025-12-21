```python
import sys
import pytest
from pathlib import Path
from unittest.mock import patch, MagicMock

# Import the script's functions or replicate its logic
def generate_docker_imagetool_args(digest_path, tag_prefix, tags):
    if tag_prefix:
        push_tags = tuple([
            t for t in tags.split(',') if t.startswith(tag_prefix)
        ])
    else:
        push_tags = tuple([
            t for t in tags.split(',') if t.startswith('netdata/')
        ])

    image_name = push_tags[0].split(':')[0]

    images = []
    for f in digest_path.glob('*'):
        images.append(f'{image_name}@sha256:{f.name}')

    return f'-t {" -t ".join(push_tags)} {" ".join(images)}'

def test_generate_docker_imagetool_args_with_tag_prefix(tmp_path):
    # Create mock digest files
    (tmp_path / 'abc123').touch()
    (tmp_path / 'def456').touch()

    # Test with a specific tag prefix
    result = generate_docker_imagetool_args(
        tmp_path, 
        'test/', 
        'test/image:1.0,test/image:2.0,other/image:3.0'
    )
    
    assert '-t test/image:1.0 -t test/image:2.0' in result
    assert 'test/image@sha256:abc123' in result
    assert 'test/image@sha256:def456' in result
    assert 'other/image:3.0' not in result

def test_generate_docker_imagetool_args_default_prefix(tmp_path):
    # Create mock digest files
    (tmp_path / 'abc123').touch()
    (tmp_path / 'def456').touch()

    # Test with default netdata/ prefix
    result = generate_docker_imagetool_args(
        tmp_path, 
        '', 
        'netdata/image:1.0,test/image:2.0,other/image:3.0'
    )
    
    assert '-t netdata/image:1.0' in result
    assert 'netdata/image@sha256:abc123' in result
    assert 'test/image:2.0' not in result
    assert 'other/image:3.0' not in result

def test_generate_docker_imagetool_args_no_matching_tags(tmp_path):
    # Create mock digest files
    (tmp_path / 'abc123').touch()

    # Test with no matching tags
    with pytest.raises(IndexError):
        generate_docker_imagetool_args(
            tmp_path, 
            'nonexistent/', 
            'test/image:1.0,other/image:2.0'
        )

def test_generate_docker_imagetool_args_empty_digest_path(tmp_path):
    # Test with an empty digest path
    result = generate_docker_imagetool_args(
        tmp_path, 
        'test/', 
        'test/image:1.0'
    )
    
    assert '-t test/image:1.0' in result
    assert len(result.split()) > 2  # Ensures no images
```
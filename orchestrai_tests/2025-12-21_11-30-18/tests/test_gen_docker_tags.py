```python
import os
import sys
import pytest
from unittest.mock import patch, mock_open

def generate_docker_tags(github_event, version):
    REPO = 'netdata/netdata'
    REPOS = {
        'docker': REPO,
        'quay': f'quay.io/{REPO}',
        'ghcr': f'ghcr.io/{REPO}',
    }
    NIGHTLY_TAG = 'edge'

    tags = {k: [] for k in REPOS.keys()}
    nightly = False

    if not version:
        for h in REPOS.keys():
            tags[h] = [f'{REPOS[h]}:test']
    elif version == 'nightly':
        for h in REPOS.keys():
            tags[h] = [f'{REPOS[h]}:{t}' for t in (NIGHTLY_TAG, 'latest')]
        nightly = True
    else:
        v = f'v{version}'.split('.')
        versions = (
            v[0],
            '.'.join(v[0:2]),
            '.'.join(v[0:3]),
            'stable',
        )
        for h in REPOS.keys():
            tags[h] = [f'{REPOS[h]}:{t}' for t in versions]

    return {
        'all_tags': [x for y in tags.values() for x in y],
        'nightly': nightly,
        'nightly_tag': NIGHTLY_TAG,
        'tags_by_repo': tags,
        'repos': REPOS
    }

def test_generate_docker_tags_empty_version():
    result = generate_docker_tags('', '')
    assert len(result['all_tags']) == 3
    assert all('test' in tag for tag in result['all_tags'])
    assert not result['nightly']
    assert result['nightly_tag'] == 'edge'

def test_generate_docker_tags_nightly_version():
    result = generate_docker_tags('', 'nightly')
    assert len(result['all_tags']) == 6
    assert all(('edge' in tag or 'latest' in tag) for tag in result['all_tags'])
    assert result['nightly']
    assert result['nightly_tag'] == 'edge'

def test_generate_docker_tags_specific_version():
    result = generate_docker_tags('', '1.2.3')
    assert len(result['all_tags']) == 12
    
    expected_tag_patterns = ['1', '1.2', '1.2.3', 'stable']
    for repo_tags in result['tags_by_repo'].values():
        for tag_pattern in expected_tag_patterns:
            assert any(tag_pattern in tag for tag in repo_tags)
    
    assert not result['nightly']
    assert result['nightly_tag'] == 'edge'

def test_generate_docker_tags_repos():
    result = generate_docker_tags('', '1.0.0')
    expected_repos = {
        'docker': 'netdata/netdata',
        'quay': 'quay.io/netdata/netdata',
        'ghcr': 'ghcr.io/netdata/netdata'
    }
    assert result['repos'] == expected_repos

@patch.dict(os.environ, {'GITHUB_OUTPUT': '/tmp/github_output'})
def test_write_output_tags():
    with patch('builtins.open', mock_open()) as mock_file:
        tags_output = 'tag1,tag2,tag3'
        
        def write_output(name, value):
            with open(os.getenv('GITHUB_OUTPUT'), 'a') as f:
                f.write(f'{name}={value}\n')
        
        write_output('tags', tags_output)
        
        mock_file.assert_called_once_with('/tmp/github_output', 'a')
        handle = mock_file()
        handle.write.assert_called_once_with('tags=tag1,tag2,tag3\n')
```
```python
import pytest
import io
from socket import socket, SocketIO
from unittest.mock import Mock, patch

# Import the function to test
from urllib3.packages.backports.makefile import backport_makefile

class MockSocket:
    def __init__(self):
        self._makefile_refs = 0

    def __call__(self, family, type):
        return self

def test_backport_makefile_invalid_mode():
    mock_socket = MockSocket()
    with pytest.raises(ValueError, match="invalid mode"):
        backport_makefile(mock_socket, mode="x")

def test_backport_makefile_binary_unbuffered():
    mock_socket = MockSocket()
    result = backport_makefile(mock_socket, mode="rb", buffering=0)
    assert isinstance(result, SocketIO)
    assert mock_socket._makefile_refs == 1

def test_backport_makefile_binary_buffered():
    mock_socket = MockSocket()
    result = backport_makefile(mock_socket, mode="rb")
    assert isinstance(result, io.BufferedReader)
    assert mock_socket._makefile_refs == 1

def test_backport_makefile_text_reading():
    mock_socket = MockSocket()
    result = backport_makefile(mock_socket, mode="r", encoding="utf-8")
    assert isinstance(result, io.TextIOWrapper)
    assert result.mode == "r"
    assert mock_socket._makefile_refs == 1

def test_backport_makefile_text_writing():
    mock_socket = MockSocket()
    result = backport_makefile(mock_socket, mode="w", encoding="utf-8")
    assert isinstance(result, io.TextIOWrapper)
    assert result.mode == "w"
    assert mock_socket._makefile_refs == 1

def test_backport_makefile_rw_mode():
    mock_socket = MockSocket()
    result = backport_makefile(mock_socket, mode="r+b")
    assert isinstance(result, io.BufferedRWPair)
    assert mock_socket._makefile_refs == 1

def test_backport_makefile_unbuffered_text_mode_raises_error():
    mock_socket = MockSocket()
    with pytest.raises(ValueError, match="unbuffered streams must be binary"):
        backport_makefile(mock_socket, mode="r", buffering=0)

def test_backport_makefile_custom_buffering():
    mock_socket = MockSocket()
    result = backport_makefile(mock_socket, mode="rb", buffering=1024)
    assert isinstance(result, io.BufferedReader)
    assert mock_socket._makefile_refs == 1
```
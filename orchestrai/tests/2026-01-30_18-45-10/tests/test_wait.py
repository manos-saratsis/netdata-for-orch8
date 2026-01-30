import pytest
from unittest.mock import Mock, patch, MagicMock
from urllib3.util.wait import _wait_for_io_events, wait_for_read, wait_for_write


class TestWaitForIoEvents:
    """Test _wait_for_io_events function"""
    
    @patch('urllib3.util.wait.HAS_SELECT', False)
    def test_wait_for_io_events_no_select_raises_error(self):
        """should raise ValueError when HAS_SELECT is False"""
        with pytest.raises(ValueError, match='Platform does not have a selector'):
            _wait_for_io_events([], 0)
    
    @patch('urllib3.util.wait.DefaultSelector')
    @patch('urllib3.util.wait.HAS_SELECT', True)
    def test_wait_for_io_events_with_single_socket(self, mock_selector_class):
        """should handle single socket passed directly"""
        mock_selector = MagicMock()
        mock_selector_class.return_value.__enter__.return_value = mock_selector
        
        mock_sock = Mock()
        mock_sock.fileno = Mock(return_value=5)
        
        mock_selector.select.return_value = [
            (Mock(fileobj=mock_sock), 1)
        ]
        
        result = _wait_for_io_events(mock_sock, 1, timeout=None)
        
        assert result == [mock_sock]
        mock_selector.register.assert_called_once()
    
    @patch('urllib3.util.wait.DefaultSelector')
    @patch('urllib3.util.wait.HAS_SELECT', True)
    def test_wait_for_io_events_with_list_of_sockets(self, mock_selector_class):
        """should handle list of sockets"""
        mock_selector = MagicMock()
        mock_selector_class.return_value.__enter__.return_value = mock_selector
        
        mock_sock1 = Mock()
        mock_sock1.fileno = Mock(return_value=5)
        mock_sock2 = Mock()
        mock_sock2.fileno = Mock(return_value=6)
        
        mock_selector.select.return_value = [
            (Mock(fileobj=mock_sock1), 1),
            (Mock(fileobj=mock_sock2), 1)
        ]
        
        result = _wait_for_io_events([mock_sock1, mock_sock2], 1)
        
        assert len(result) == 2
        assert mock_sock1 in result
        assert mock_sock2 in result
    
    @patch('urllib3.util.wait.DefaultSelector')
    @patch('urllib3.util.wait.HAS_SELECT', True)
    def test_wait_for_io_events_with_iterable_of_sockets(self, mock_selector_class):
        """should handle iterable of sockets"""
        mock_selector = MagicMock()
        mock_selector_class.return_value.__enter__.return_value = mock_selector
        
        mock_sock1 = Mock()
        mock_sock2 = Mock()
        
        mock_selector.select.return_value = [
            (Mock(fileobj=mock_sock1), 1),
            (Mock(fileobj=mock_sock2), 1)
        ]
        
        sockets_generator = (s for s in [mock_sock1, mock_sock2])
        result = _wait_for_io_events(sockets_generator, 1)
        
        assert len(result) == 2
    
    @patch('urllib3.util.wait.DefaultSelector')
    @patch('urllib3.util.wait.HAS_SELECT', True)
    def test_wait_for_io_events_filters_by_event_mask(self, mock_selector_class):
        """should filter results by event mask"""
        mock_selector = MagicMock()
        mock_selector_class.return_value.__enter__.return_value = mock_selector
        
        mock_sock1 = Mock()
        mock_sock2 = Mock()
        
        # Only sock1 has the event bit set
        mock_selector.select.return_value = [
            (Mock(fileobj=mock_sock1), 1),  # Event bit 1 matches mask 1
            (Mock(fileobj=mock_sock2), 2)   # Event bit 2 does NOT match mask 1
        ]
        
        result = _wait_for_io_events([mock_sock1, mock_sock2], 1)
        
        assert len(result) == 1
        assert mock_sock1 in result
    
    @patch('urllib3.util.wait.DefaultSelector')
    @patch('urllib3.util.wait.HAS_SELECT', True)
    def test_wait_for_io_events_with_timeout(self, mock_selector_class):
        """should pass timeout to selector"""
        mock_selector = MagicMock()
        mock_selector_class.return_value.__enter__.return_value = mock_selector
        mock_selector.select.return_value = []
        
        mock_sock = Mock()
        _wait_for_io_events([mock_sock], 1, timeout=5.0)
        
        mock_selector.select.assert_called_once_with(5.0)
    
    @patch('urllib3.util.wait.DefaultSelector')
    @patch('urllib3.util.wait.HAS_SELECT', True)
    def test_wait_for_io_events_empty_result(self, mock_selector_class):
        """should return empty list when no sockets ready"""
        mock_selector = MagicMock()
        mock_selector_class.return_value.__enter__.return_value = mock_selector
        mock_selector.select.return_value = []
        
        mock_sock = Mock()
        result = _wait_for_io_events([mock_sock], 1)
        
        assert result == []


class TestWaitForRead:
    """Test wait_for_read function"""
    
    @patch('urllib3.util.wait._wait_for_io_events')
    @patch('urllib3.util.wait.EVENT_READ', 1)
    def test_wait_for_read_calls_wait_for_io_events_with_read_event(self, mock_wait):
        """should call _wait_for_io_events with READ event"""
        mock_sock = Mock()
        mock_wait.return_value = [mock_sock]
        
        result = wait_for_read([mock_sock])
        
        mock_wait.assert_called_once_with([mock_sock], 1, None)
        assert result == [mock_sock]
    
    @patch('urllib3.util.wait._wait_for_io_events')
    @patch('urllib3.util.wait.EVENT_READ', 1)
    def test_wait_for_read_with_timeout(self, mock_wait):
        """should pass timeout to _wait_for_io_events"""
        mock_sock = Mock()
        mock_wait.return_value = []
        
        wait_for_read([mock_sock], timeout=2.5)
        
        mock_wait.assert_called_once_with([mock_sock], 1, 2.5)
    
    @patch('urllib3.util.wait._wait_for_io_events')
    @patch('urllib3.util.wait.EVENT_READ', 1)
    def test_wait_for_read_with_single_socket(self, mock_wait):
        """should handle single socket"""
        mock_sock = Mock()
        mock_wait.return_value = [mock_sock]
        
        result = wait_for_read(mock_sock)
        
        mock_wait.assert_called_once()
        assert result == [mock_sock]


class TestWaitForWrite:
    """Test wait_for_write function"""
    
    @patch('urllib3.util.wait._wait_for_io_events')
    @patch('urllib3.util.wait.EVENT_WRITE', 2)
    def test_wait_for_write_calls_wait_for_io_events_with_write_event(self, mock_wait):
        """should call _wait_for_io_events with WRITE event"""
        mock_sock = Mock()
        mock_wait.return_value = [mock_sock]
        
        result = wait_for_write([mock_sock])
        
        mock_wait.assert_called_once_with([mock_sock], 2, None)
        assert result == [mock_sock]
    
    @patch('urllib3.util.wait._wait_for_io_events')
    @patch('urllib3.util.wait.EVENT_WRITE', 2)
    def test_wait_for_write_with_timeout(self, mock_wait):
        """should pass timeout to _wait_for_io_events"""
        mock_sock = Mock()
        mock_wait.return_value = []
        
        wait_for_write([mock_sock], timeout=3.0)
        
        mock_wait.assert_called_once_with([mock_sock], 2, 3.0)
    
    @patch('urllib3.util.wait._wait_for_io_events')
    @patch('urllib3.util.wait.EVENT_WRITE', 2)
    def test_wait_for_write_with_single_socket(self, mock_wait):
        """should handle single socket"""
        mock_sock = Mock()
        mock_wait.return_value = [mock_sock]
        
        result = wait_for_write(mock_sock)
        
        mock_wait.assert_called_once()
        assert result == [mock_sock]
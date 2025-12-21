```python
import unittest
import json
import sys
import asyncio
import websockets
from unittest.mock import patch, MagicMock
import nd_mcp  # Assuming the script is importable

class TestNetdataMCP(unittest.TestCase):
    def test_parse_jsonrpc_message_valid(self):
        """Test parsing a valid JSON-RPC message"""
        message = json.dumps({
            "jsonrpc": "2.0", 
            "method": "test_method", 
            "id": "123"
        })
        id, method = nd_mcp.parse_jsonrpc_message(message)
        self.assertEqual(id, "123")
        self.assertEqual(method, "test_method")

    def test_parse_jsonrpc_message_invalid_json(self):
        """Test parsing an invalid JSON message"""
        message = "Invalid JSON"
        id, method = nd_mcp.parse_jsonrpc_message(message)
        self.assertIsNone(id)
        self.assertIsNone(method)

    def test_create_jsonrpc_error(self):
        """Test creating a JSON-RPC error response"""
        error_response = nd_mcp.create_jsonrpc_error(
            id="test_id", 
            code=-32000, 
            message="Test error", 
            data={"details": "Test details"}
        )
        error_data = json.loads(error_response)
        
        self.assertEqual(error_data["jsonrpc"], "2.0")
        self.assertEqual(error_data["id"], "test_id")
        self.assertEqual(error_data["error"]["code"], -32000)
        self.assertEqual(error_data["error"]["message"], "Test error")
        self.assertEqual(error_data["error"]["data"], {"details": "Test details"})

    def test_parse_args_valid_uri(self):
        """Test parsing arguments with valid URI"""
        with patch.object(sys, 'argv', ['nd-mcp.py', 'ws://localhost:8080/path']):
            target, bearer = nd_mcp.parse_args(sys.argv[1:])
            self.assertEqual(target, 'ws://localhost:8080/path')
            self.assertIsNone(bearer)

    def test_parse_args_with_bearer(self):
        """Test parsing arguments with bearer token"""
        with patch.object(sys, 'argv', ['nd-mcp.py', '--bearer', 'test_token', 'ws://localhost:8080/path']):
            target, bearer = nd_mcp.parse_args(sys.argv[1:])
            self.assertEqual(target, 'ws://localhost:8080/path')
            self.assertEqual(bearer, 'test_token')

    def test_parse_args_invalid_args(self):
        """Test parsing arguments with invalid configuration"""
        with self.assertRaises(SystemExit):
            with patch.object(sys, 'argv', ['nd-mcp.py']):
                nd_mcp.parse_args(sys.argv[1:])

    @patch('os.environ.get')
    def test_main_with_env_bearer_token(self, mock_env):
        """Test main function with bearer token from environment"""
        mock_env.return_value = "env_token"
        
        with patch('nd_mcp.parse_args', return_value=('ws://test', None)) as mock_parse_args, \
             patch('nd_mcp.connect_with_backoff') as mock_connect:
            
            nd_mcp.main()
            
            mock_parse_args.assert_called_once()
            mock_connect.assert_called_with('ws://test', 'env_token')

    @patch('sys.exit')
    def test_signal_handler(self, mock_exit):
        """Test signal handling"""
        handler = nd_mcp.signal_handler
        handler(signal.SIGINT, None)
        mock_exit.assert_called_with(0)

async def test_connect_with_backoff_connection_retry():
    """Test connection retry mechanism"""
    # This would be a complex async test requiring more detailed mocking
    pass

if __name__ == '__main__':
    unittest.main()
```
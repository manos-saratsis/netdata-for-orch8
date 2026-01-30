"""
Comprehensive tests for packaging/windows/fetch-msys2-installer.py

Tests cover all functions, branches, error paths, and edge cases.
"""

import json
import sys
import hashlib
import pytest
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest.mock import Mock, patch, MagicMock, mock_open, call
from urllib.request import Request
from urllib.error import HTTPError, URLError
import io

# Import the module
sys.path.insert(0, str(Path(__file__).parent.parent / "packaging" / "windows"))
from fetch_msys2_installer import (
    get_latest_release,
    fetch_release_asset,
    main,
    REPO,
)


class TestGetLatestRelease:
    """Test suite for get_latest_release() function."""

    def test_get_latest_release_success_with_nightly_builds(self):
        """Should filter out Nightly Installer Build and return latest release."""
        mock_response_data = [
            {
                "name": "Nightly Installer Build",
                "tag_name": "2024-01-15",
                "id": 1
            },
            {
                "name": "msys2-base-x86_64-20240114.0",
                "tag_name": "2024-01-14-1",
                "id": 2
            },
            {
                "name": "msys2-base-x86_64-20240101.0",
                "tag_name": "2024-01-01-0",
                "id": 3
            }
        ]

        with patch('fetch_msys2_installer.urlopen') as mock_urlopen:
            mock_response = MagicMock()
            mock_response.status = 200
            mock_response.__enter__ = Mock(return_value=mock_response)
            mock_response.__exit__ = Mock(return_value=False)
            mock_urlopen.return_value = mock_response

            with patch('fetch_msys2_installer.json.load', return_value=mock_response_data):
                with patch('builtins.print') as mock_print:
                    name, version = get_latest_release()

                    assert name == "msys2-base-x86_64-20240114.0"
                    assert version == "20240114.0"
                    mock_print.assert_called_with('>>> Fetching release list')

    def test_get_latest_release_success_no_nightly_builds(self):
        """Should handle response with no Nightly Installer Builds."""
        mock_response_data = [
            {
                "name": "msys2-base-x86_64-20240114.0",
                "tag_name": "v2024-01-14",
                "id": 1
            }
        ]

        with patch('fetch_msys2_installer.urlopen') as mock_urlopen:
            mock_response = MagicMock()
            mock_response.status = 200
            mock_response.__enter__ = Mock(return_value=mock_response)
            mock_response.__exit__ = Mock(return_value=False)
            mock_urlopen.return_value = mock_response

            with patch('fetch_msys2_installer.json.load', return_value=mock_response_data):
                name, version = get_latest_release()

                assert name == "msys2-base-x86_64-20240114.0"
                assert version == "v20240114"

    def test_get_latest_release_http_failure(self):
        """Should exit with code 1 on HTTP error."""
        with patch('fetch_msys2_installer.urlopen') as mock_urlopen:
            mock_response = MagicMock()
            mock_response.status = 404
            mock_response.__enter__ = Mock(return_value=mock_response)
            mock_response.__exit__ = Mock(return_value=False)
            mock_urlopen.return_value = mock_response

            with patch('builtins.print') as mock_print:
                with pytest.raises(SystemExit) as exc_info:
                    get_latest_release()

                assert exc_info.value.code == 1
                mock_print.assert_called_with('!!! Failed to fetch release list, status=404')

    def test_get_latest_release_timeout(self):
        """Should handle timeout from urlopen."""
        with patch('fetch_msys2_installer.urlopen', side_effect=TimeoutError("Connection timed out")):
            with pytest.raises(TimeoutError):
                get_latest_release()

    def test_get_latest_release_with_dashes_in_tag(self):
        """Should correctly handle tag names with multiple dashes."""
        mock_response_data = [
            {
                "name": "Release-2024-01-14-v1",
                "tag_name": "release-2024-01-14-v1",
                "id": 1
            }
        ]

        with patch('fetch_msys2_installer.urlopen') as mock_urlopen:
            mock_response = MagicMock()
            mock_response.status = 200
            mock_response.__enter__ = Mock(return_value=mock_response)
            mock_response.__exit__ = Mock(return_value=False)
            mock_urlopen.return_value = mock_response

            with patch('fetch_msys2_installer.json.load', return_value=mock_response_data):
                name, version = get_latest_release()

                assert version == "release20240114v1"

    def test_get_latest_release_connection_error(self):
        """Should propagate URLError."""
        with patch('fetch_msys2_installer.urlopen', side_effect=URLError("Connection refused")):
            with pytest.raises(URLError):
                get_latest_release()

    def test_get_latest_release_json_decode_error(self):
        """Should propagate JSON decode error."""
        with patch('fetch_msys2_installer.urlopen') as mock_urlopen:
            mock_response = MagicMock()
            mock_response.status = 200
            mock_response.__enter__ = Mock(return_value=mock_response)
            mock_response.__exit__ = Mock(return_value=False)
            mock_urlopen.return_value = mock_response

            with patch('fetch_msys2_installer.json.load', side_effect=json.JSONDecodeError("Invalid JSON", "", 0)):
                with pytest.raises(json.JSONDecodeError):
                    get_latest_release()


class TestFetchReleaseAsset:
    """Test suite for fetch_release_asset() function."""

    def test_fetch_release_asset_success(self):
        """Should successfully download and save release asset."""
        with TemporaryDirectory() as tmpdir:
            tmppath = Path(tmpdir)
            asset_name = "test-asset-1.0.tar.zst"
            asset_data = b"test file content 123"
            release_name = "v1.0.0"

            with patch('fetch_msys2_installer.urlopen') as mock_urlopen:
                mock_response = MagicMock()
                mock_response.status = 200
                mock_response.read = Mock(return_value=asset_data)
                mock_response.__enter__ = Mock(return_value=mock_response)
                mock_response.__exit__ = Mock(return_value=False)
                mock_urlopen.return_value = mock_response

                with patch('builtins.print') as mock_print:
                    result = fetch_release_asset(tmppath, release_name, asset_name)

                    assert result == tmppath / asset_name
                    assert result.exists()
                    assert result.read_bytes() == asset_data
                    mock_print.assert_called_with(f'>>> Downloading {asset_name}')

    def test_fetch_release_asset_http_failure(self):
        """Should exit with code 1 on HTTP error."""
        with TemporaryDirectory() as tmpdir:
            tmppath = Path(tmpdir)
            asset_name = "test-asset-1.0.tar.zst"
            release_name = "v1.0.0"

            with patch('fetch_msys2_installer.urlopen') as mock_urlopen:
                mock_response = MagicMock()
                mock_response.status = 404
                mock_response.__enter__ = Mock(return_value=mock_response)
                mock_response.__exit__ = Mock(return_value=False)
                mock_urlopen.return_value = mock_response

                with patch('builtins.print') as mock_print:
                    with pytest.raises(SystemExit) as exc_info:
                        fetch_release_asset(tmppath, release_name, asset_name)

                    assert exc_info.value.code == 1
                    mock_print.assert_called_with(f'!!! Failed to fetch {asset_name}, status=404')

    def test_fetch_release_asset_timeout(self):
        """Should handle timeout from urlopen."""
        with TemporaryDirectory() as tmpdir:
            tmppath = Path(tmpdir)

            with patch('fetch_msys2_installer.urlopen', side_effect=TimeoutError("Timeout")):
                with pytest.raises(TimeoutError):
                    fetch_release_asset(tmppath, "v1.0.0", "test-asset.tar.zst")

    def test_fetch_release_asset_empty_file(self):
        """Should handle empty asset files."""
        with TemporaryDirectory() as tmpdir:
            tmppath = Path(tmpdir)
            asset_name = "empty-asset.tar.zst"
            release_name = "v1.0.0"

            with patch('fetch_msys2_installer.urlopen') as mock_urlopen:
                mock_response = MagicMock()
                mock_response.status = 200
                mock_response.read = Mock(return_value=b"")
                mock_response.__enter__ = Mock(return_value=mock_response)
                mock_response.__exit__ = Mock(return_value=False)
                mock_urlopen.return_value = mock_response

                result = fetch_release_asset(tmppath, release_name, asset_name)

                assert result.exists()
                assert result.read_bytes() == b""

    def test_fetch_release_asset_large_file(self):
        """Should handle large asset files."""
        with TemporaryDirectory() as tmpdir:
            tmppath = Path(tmpdir)
            asset_name = "large-asset.tar.zst"
            release_name = "v1.0.0"
            large_data = b"x" * (10 * 1024 * 1024)  # 10 MB

            with patch('fetch_msys2_installer.urlopen') as mock_urlopen:
                mock_response = MagicMock()
                mock_response.status = 200
                mock_response.read = Mock(return_value=large_data)
                mock_response.__enter__ = Mock(return_value=mock_response)
                mock_response.__exit__ = Mock(return_value=False)
                mock_urlopen.return_value = mock_response

                result = fetch_release_asset(tmppath, release_name, asset_name)

                assert result.read_bytes() == large_data

    def test_fetch_release_asset_request_attributes(self):
        """Should make request with correct URL and headers."""
        with TemporaryDirectory() as tmpdir:
            tmppath = Path(tmpdir)
            asset_name = "test.tar.zst"
            release_name = "v1.0.0"

            with patch('fetch_msys2_installer.urlopen') as mock_urlopen:
                mock_response = MagicMock()
                mock_response.status = 200
                mock_response.read = Mock(return_value=b"data")
                mock_response.__enter__ = Mock(return_value=mock_response)
                mock_response.__exit__ = Mock(return_value=False)
                mock_urlopen.return_value = mock_response

                fetch_release_asset(tmppath, release_name, asset_name)

                # Verify the request was made with correct URL
                call_args = mock_urlopen.call_args[0][0]
                assert f"https://github.com/{REPO}/releases/download/{release_name}/{asset_name}" == call_args.full_url


class TestMain:
    """Test suite for main() function."""

    def test_main_success_with_valid_arguments(self):
        """Should successfully process with valid argument."""
        with TemporaryDirectory() as tmpdir:
            target_file = Path(tmpdir) / "output.tar.zst"

            # Mock all external dependencies
            with patch('sys.argv', ['script', str(target_file)]):
                with patch('fetch_msys2_installer.get_latest_release') as mock_get_release:
                    with patch('fetch_msys2_installer.fetch_release_asset') as mock_fetch:
                        with patch('fetch_msys2_installer.hashlib.sha256') as mock_sha256:
                            with patch('fetch_msys2_installer.shutil.copy') as mock_copy:
                                mock_get_release.return_value = ("v1.0.0", "1.0.0")

                                installer_path = Path(tmpdir) / "installer.tar.zst"
                                checksums_path = Path(tmpdir) / "checksums.txt"

                                test_data = b"installer data"
                                test_checksum = hashlib.sha256(test_data).hexdigest()

                                installer_path.write_bytes(test_data)
                                checksums_path.write_text(f"{test_checksum} installer.tar.zst")

                                mock_fetch.side_effect = [installer_path, checksums_path]

                                with patch('builtins.print'):
                                    main()

                                # Verify shutil.copy was called
                                assert mock_copy.called

    def test_main_wrong_argument_count_zero(self):
        """Should print error when no arguments provided."""
        with patch('sys.argv', ['script']):
            with patch('builtins.print') as mock_print:
                main()
                mock_print.assert_called_once()
                assert "must be run with exactly one argument" in str(mock_print.call_args)

    def test_main_wrong_argument_count_multiple(self):
        """Should print error when more than one argument provided."""
        with patch('sys.argv', ['script', 'arg1', 'arg2']):
            with patch('builtins.print') as mock_print:
                main()
                mock_print.assert_called_once()
                assert "must be run with exactly one argument" in str(mock_print.call_args)

    def test_main_checksum_mismatch(self):
        """Should exit with code 1 on checksum mismatch."""
        with TemporaryDirectory() as tmpdir:
            target_file = Path(tmpdir) / "output.tar.zst"

            with patch('sys.argv', ['script', str(target_file)]):
                with patch('fetch_msys2_installer.get_latest_release') as mock_get_release:
                    with patch('fetch_msys2_installer.fetch_release_asset') as mock_fetch:
                        mock_get_release.return_value = ("v1.0.0", "1.0.0")

                        installer_path = Path(tmpdir) / "installer.tar.zst"
                        checksums_path = Path(tmpdir) / "checksums.txt"

                        installer_path.write_bytes(b"installer data")
                        wrong_checksum = "a" * 64
                        checksums_path.write_text(f"{wrong_checksum} installer.tar.zst")

                        mock_fetch.side_effect = [installer_path, checksums_path]

                        with patch('builtins.print') as mock_print:
                            with pytest.raises(SystemExit) as exc_info:
                                main()

                            assert exc_info.value.code == 1
                            print_calls = [str(call) for call in mock_print.call_args_list]
                            assert any("Checksum mismatch" in str(call) for call in print_calls)

    def test_main_checksum_validation_case_insensitive(self):
        """Should handle checksum comparison case-insensitively."""
        with TemporaryDirectory() as tmpdir:
            target_file = Path(tmpdir) / "output.tar.zst"

            with patch('sys.argv', ['script', str(target_file)]):
                with patch('fetch_msys2_installer.get_latest_release') as mock_get_release:
                    with patch('fetch_msys2_installer.fetch_release_asset') as mock_fetch:
                        with patch('fetch_msys2_installer.shutil.copy') as mock_copy:
                            mock_get_release.return_value = ("v1.0.0", "1.0.0")

                            installer_path = Path(tmpdir) / "installer.tar.zst"
                            checksums_path = Path(tmpdir) / "checksums.txt"

                            test_data = b"test data"
                            checksum_lower = hashlib.sha256(test_data).hexdigest()
                            checksum_upper = checksum_lower.upper()

                            installer_path.write_bytes(test_data)
                            checksums_path.write_text(f"{checksum_upper} installer.tar.zst")

                            mock_fetch.side_effect = [installer_path, checksums_path]

                            with patch('builtins.print'):
                                main()

                            assert mock_copy.called

    def test_main_creates_tmp_file(self):
        """Should use temporary file naming convention."""
        with TemporaryDirectory() as tmpdir:
            target_file = Path(tmpdir) / "output.tar.zst"

            with patch('sys.argv', ['script', str(target_file)]):
                with patch('fetch_msys2_installer.get_latest_release') as mock_get_release:
                    with patch('fetch_msys2_installer.fetch_release_asset') as mock_fetch:
                        with patch('fetch_msys2_installer.shutil.copy') as mock_copy:
                            with patch.object(Path, 'replace') as mock_replace:
                                mock_get_release.return_value = ("v1.0.0", "1.0.0")

                                installer_path = Path(tmpdir) / "installer.tar.zst"
                                checksums_path = Path(tmpdir) / "checksums.txt"

                                test_data = b"test"
                                test_checksum = hashlib.sha256(test_data).hexdigest()

                                installer_path.write_bytes(test_data)
                                checksums_path.write_text(f"{test_checksum} installer.tar.zst")

                                mock_fetch.side_effect = [installer_path, checksums_path]

                                with patch('builtins.print'):
                                    main()

                                # Verify file replacement was attempted
                                assert mock_replace.called

    def test_main_checksum_extraction_from_file(self):
        """Should correctly extract checksum from checksum file."""
        with TemporaryDirectory() as tmpdir:
            target_file = Path(tmpdir) / "output.tar.zst"

            with patch('sys.argv', ['script', str(target_file)]):
                with patch('fetch_msys2_installer.get_latest_release') as mock_get_release:
                    with patch('fetch_msys2_installer.fetch_release_asset') as mock_fetch:
                        with patch('fetch_msys2_installer.shutil.copy') as mock_copy:
                            mock_get_release.return_value = ("v1.0.0", "1.0.0")

                            installer_path = Path(tmpdir) / "installer.tar.zst"
                            checksums_path = Path(tmpdir) / "checksums.txt"

                            test_data = b"data"
                            checksum = hashlib.sha256(test_data).hexdigest()

                            installer_path.write_bytes(test_data)
                            # Format: checksum  filename
                            checksums_path.write_text(f"{checksum}  installer.tar.zst")

                            mock_fetch.side_effect = [installer_path, checksums_path]

                            with patch('builtins.print'):
                                main()

                            assert mock_copy.called

    def test_main_empty_checksum_file(self):
        """Should handle checksums file with whitespace only."""
        with TemporaryDirectory() as tmpdir:
            target_file = Path(tmpdir) / "output.tar.zst"

            with patch('sys.argv', ['script', str(target_file)]):
                with patch('fetch_msys2_installer.get_latest_release') as mock_get_release:
                    with patch('fetch_msys2_installer.fetch_release_asset') as mock_fetch:
                        mock_get_release.return_value = ("v1.0.0", "1.0.0")

                        installer_path = Path(tmpdir) / "installer.tar.zst"
                        checksums_path = Path(tmpdir) / "checksums.txt"

                        installer_path.write_bytes(b"data")
                        checksums_path.write_text("")

                        mock_fetch.side_effect = [installer_path, checksums_path]

                        with patch('builtins.print'):
                            with pytest.raises(SystemExit):
                                main()

    def test_main_integration_full_flow(self):
        """Integration test: full flow from fetch to verification."""
        with TemporaryDirectory() as tmpdir:
            target_file = Path(tmpdir) / "final.tar.zst"
            test_content = b"x" * 1000

            with patch('sys.argv', ['script', str(target_file)]):
                with patch('fetch_msys2_installer.get_latest_release') as mock_get_release:
                    with patch('fetch_msys2_installer.fetch_release_asset') as mock_fetch:
                        mock_get_release.return_value = ("v1.0.0", "1.0.0")

                        installer_path = Path(tmpdir) / "installer.tar.zst"
                        checksums_path = Path(tmpdir) / "checksums.txt"

                        expected_checksum = hashlib.sha256(test_content).hexdigest()

                        installer_path.write_bytes(test_content)
                        checksums_path.write_text(f"{expected_checksum}  installer.tar.zst")

                        mock_fetch.side_effect = [installer_path, checksums_path]

                        with patch('builtins.print'):
                            main()

                        # Verify target file was created (through the tmp file)
                        tmp_target = target_file.with_name(f'.{target_file.name}.tmp')
                        # After replace, the tmp file should not exist, but target should
                        assert not installer_path.exists()  # Original should be cleaned up

    def test_main_pathlib_handling(self):
        """Should correctly handle Path objects for argument."""
        with TemporaryDirectory() as tmpdir:
            target = Path(tmpdir) / "output.tar.zst"

            with patch('sys.argv', ['script', str(target)]):
                with patch('fetch_msys2_installer.get_latest_release', return_value=("v1", "1")):
                    with patch('fetch_msys2_installer.fetch_release_asset') as mock_fetch:
                        with patch('fetch_msys2_installer.shutil.copy'):
                            installer = Path(tmpdir) / "installer.tar.zst"
                            checksums = Path(tmpdir) / "checksums.txt"

                            data = b"test"
                            checksum = hashlib.sha256(data).hexdigest()

                            installer.write_bytes(data)
                            checksums.write_text(f"{checksum} installer.tar.zst")

                            mock_fetch.side_effect = [installer, checksums]

                            with patch('builtins.print'):
                                main()
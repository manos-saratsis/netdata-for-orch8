import pytest
from unittest.mock import Mock, MagicMock, patch, call
import os
from pathlib import Path
import dagger

from imageutils import (
    _ALPINE_COMMON_PACKAGES,
    build_alpine_3_18,
    build_alpine_3_19,
    static_build_openssl,
    static_build_bash,
    static_build_curl,
    static_build_ioping,
    static_build_libnetfilter_acct,
    static_build_netdata,
    static_build,
    _CENTOS_COMMON_PACKAGES,
    build_amazon_linux_2,
    build_centos_7,
    _ROCKY_LINUX_COMMON_PACKAGES,
    build_rocky_linux_8,
    build_rocky_linux_9,
    _CENTOS_STREAM_COMMON_PACKAGES,
    build_centos_stream_8,
    build_centos_stream_9,
    _ORACLE_LINUX_COMMON_PACKAGES,
    build_oracle_linux_9,
    build_oracle_linux_8,
    _OPENSUSE_COMMON_PACKAGES,
    build_opensuse_tumbleweed,
    build_opensuse_15_5,
    build_opensuse_15_4,
    _FEDORA_COMMON_PACKAGES,
    build_fedora_37,
    build_fedora_38,
    build_fedora_39,
    _DEBIAN_COMMON_PACKAGES,
    build_debian_10,
    build_debian_11,
    build_debian_12,
    _UBUNTU_COMMON_PACKAGES,
    build_ubuntu_20_04,
    build_ubuntu_22_04,
    build_ubuntu_23_04,
    build_ubuntu_23_10,
    install_cargo,
)


class TestAlpineCommonPackages:
    """Test Alpine common packages list."""

    def test_alpine_common_packages_exists(self):
        """Should have alpine common packages list defined."""
        assert isinstance(_ALPINE_COMMON_PACKAGES, list)
        assert len(_ALPINE_COMMON_PACKAGES) > 0

    def test_alpine_common_packages_contains_expected_items(self):
        """Should contain expected Alpine packages."""
        expected = ["alpine-sdk", "bash", "gcc", "git", "make"]
        for pkg in expected:
            assert pkg in _ALPINE_COMMON_PACKAGES


class TestBuildAlpine318:
    """Test build_alpine_3_18 function."""

    def test_build_alpine_3_18_basic(self):
        """Should build Alpine 3.18 container."""
        mock_client = Mock(spec=dagger.Client)
        mock_container = Mock(spec=dagger.Container)
        mock_platform = Mock(spec=dagger.Platform)

        mock_client.container.return_value = mock_container
        mock_container.from_.return_value = mock_container
        mock_container.with_exec.return_value = mock_container

        result = build_alpine_3_18(mock_client, mock_platform)

        assert result is not None
        mock_client.container.assert_called_once_with(platform=mock_platform)
        mock_container.from_.assert_called_once_with("alpine:3.18")

    def test_build_alpine_3_18_installs_packages(self):
        """Should install all Alpine packages."""
        mock_client = Mock(spec=dagger.Client)
        mock_container = Mock(spec=dagger.Container)
        mock_platform = Mock(spec=dagger.Platform)

        mock_client.container.return_value = mock_container
        mock_container.from_.return_value = mock_container
        mock_container.with_exec.return_value = mock_container

        result = build_alpine_3_18(mock_client, mock_platform)

        # Verify with_exec was called with apk add command
        calls = mock_container.with_exec.call_args_list
        apk_call = calls[-1]
        assert apk_call[0][0][0] == "apk"
        assert apk_call[0][0][1] == "add"
        assert apk_call[0][0][2] == "--no-cache"


class TestBuildAlpine319:
    """Test build_alpine_3_19 function."""

    def test_build_alpine_3_19_basic(self):
        """Should build Alpine 3.19 container."""
        mock_client = Mock(spec=dagger.Client)
        mock_container = Mock(spec=dagger.Container)
        mock_platform = Mock(spec=dagger.Platform)

        mock_client.container.return_value = mock_container
        mock_container.from_.return_value = mock_container
        mock_container.with_exec.return_value = mock_container

        result = build_alpine_3_19(mock_client, mock_platform)

        assert result is not None
        mock_client.container.assert_called_once_with(platform=mock_platform)
        mock_container.from_.assert_called_once_with("alpine:3.19")

    def test_build_alpine_3_19_installs_packages(self):
        """Should install all Alpine packages."""
        mock_client = Mock(spec=dagger.Client)
        mock_container = Mock(spec=dagger.Container)
        mock_platform = Mock(spec=dagger.Platform)

        mock_client.container.return_value = mock_container
        mock_container.from_.return_value = mock_container
        mock_container.with_exec.return_value = mock_container

        result = build_alpine_3_19(mock_client, mock_platform)

        calls = mock_container.with_exec.call_args_list
        apk_call = calls[-1]
        assert apk_call[0][0][0] == "apk"


class TestStaticBuildOpenssl:
    """Test static_build_openssl function."""

    def test_static_build_openssl_basic(self):
        """Should build OpenSSL statically."""
        mock_client = Mock(spec=dagger.Client)
        mock_ctr = Mock(spec=dagger.Container)
        mock_git = Mock()
        mock_tree = Mock()

        mock_client.git.return_value = mock_git
        mock_git.tag.return_value = mock_git
        mock_git.tree.return_value = mock_tree

        mock_ctr.with_directory.return_value = mock_ctr
        mock_ctr.with_workdir.return_value = mock_ctr
        mock_ctr.with_env_variable.return_value = mock_ctr
        mock_ctr.with_exec.return_value = mock_ctr

        result = static_build_openssl(mock_client, mock_ctr)

        assert result is not None
        mock_client.git.assert_called_once_with(
            url="https://github.com/openssl/openssl", keep_git_dir=True
        )
        mock_git.tag.assert_called_once_with("openssl-3.1.4")

    def test_static_build_openssl_environment_variables(self):
        """Should set correct environment variables."""
        mock_client = Mock(spec=dagger.Client)
        mock_ctr = Mock(spec=dagger.Container)
        mock_git = Mock()
        mock_tree = Mock()

        mock_client.git.return_value = mock_git
        mock_git.tag.return_value = mock_git
        mock_git.tree.return_value = mock_tree

        mock_ctr.with_directory.return_value = mock_ctr
        mock_ctr.with_workdir.return_value = mock_ctr
        mock_ctr.with_env_variable.return_value = mock_ctr
        mock_ctr.with_exec.return_value = mock_ctr

        result = static_build_openssl(mock_client, mock_ctr)

        env_calls = [
            call for call in mock_ctr.with_env_variable.call_args_list
        ]
        env_dict = {call[0][0]: call[0][1] for call in env_calls}

        assert "CFLAGS" in env_dict
        assert "LDFLAGS" in env_dict
        assert "PKG_CONFIG" in env_dict

    def test_static_build_openssl_commands(self):
        """Should execute correct build commands."""
        mock_client = Mock(spec=dagger.Client)
        mock_ctr = Mock(spec=dagger.Container)
        mock_git = Mock()
        mock_tree = Mock()

        mock_client.git.return_value = mock_git
        mock_git.tag.return_value = mock_git
        mock_git.tree.return_value = mock_tree

        mock_ctr.with_directory.return_value = mock_ctr
        mock_ctr.with_workdir.return_value = mock_ctr
        mock_ctr.with_env_variable.return_value = mock_ctr
        mock_ctr.with_exec.return_value = mock_ctr

        result = static_build_openssl(mock_client, mock_ctr)

        exec_calls = [call[0][0] for call in mock_ctr.with_exec.call_args_list]
        # Check for key commands
        command_strs = [" ".join(map(str, cmd)) for cmd in exec_calls]
        assert any("./config" in cmd for cmd in command_strs)
        assert any("make" in cmd for cmd in command_strs)


class TestStaticBuildBash:
    """Test static_build_bash function."""

    def test_static_build_bash_basic(self):
        """Should build Bash statically."""
        mock_client = Mock(spec=dagger.Client)
        mock_ctr = Mock(spec=dagger.Container)
        mock_git = Mock()
        mock_tree = Mock()

        mock_client.git.return_value = mock_git
        mock_git.tag.return_value = mock_git
        mock_git.tree.return_value = mock_tree

        mock_ctr.with_directory.return_value = mock_ctr
        mock_ctr.with_workdir.return_value = mock_ctr
        mock_ctr.with_env_variable.return_value = mock_ctr
        mock_ctr.with_exec.return_value = mock_ctr

        result = static_build_bash(mock_client, mock_ctr)

        assert result is not None
        mock_client.git.assert_called_once()
        assert "bash.git" in mock_client.git.call_args[1]["url"]

    def test_static_build_bash_environment_variables(self):
        """Should set environment variables for bash build."""
        mock_client = Mock(spec=dagger.Client)
        mock_ctr = Mock(spec=dagger.Container)
        mock_git = Mock()
        mock_tree = Mock()

        mock_client.git.return_value = mock_git
        mock_git.tag.return_value = mock_git
        mock_git.tree.return_value = mock_tree

        mock_ctr.with_directory.return_value = mock_ctr
        mock_ctr.with_workdir.return_value = mock_ctr
        mock_ctr.with_env_variable.return_value = mock_ctr
        mock_ctr.with_exec.return_value = mock_ctr

        result = static_build_bash(mock_client, mock_ctr)

        env_calls = [call[0][0] for call in mock_ctr.with_env_variable.call_args_list]
        assert "CFLAGS" in env_calls or "PKG_CONFIG" in env_calls


class TestStaticBuildCurl:
    """Test static_build_curl function."""

    def test_static_build_curl_basic(self):
        """Should build curl statically."""
        mock_client = Mock(spec=dagger.Client)
        mock_ctr = Mock(spec=dagger.Container)
        mock_git = Mock()
        mock_tree = Mock()

        mock_client.git.return_value = mock_git
        mock_git.tag.return_value = mock_git
        mock_git.tree.return_value = mock_tree

        mock_ctr.with_directory.return_value = mock_ctr
        mock_ctr.with_workdir.return_value = mock_ctr
        mock_ctr.with_env_variable.return_value = mock_ctr
        mock_ctr.with_exec.return_value = mock_ctr

        result = static_build_curl(mock_client, mock_ctr)

        assert result is not None
        mock_client.git.assert_called_once()
        assert "curl" in mock_client.git.call_args[1]["url"]

    def test_static_build_curl_git_repo(self):
        """Should use correct Git repository URL."""
        mock_client = Mock(spec=dagger.Client)
        mock_ctr = Mock(spec=dagger.Container)
        mock_git = Mock()
        mock_tree = Mock()

        mock_client.git.return_value = mock_git
        mock_git.tag.return_value = mock_git
        mock_git.tree.return_value = mock_tree

        mock_ctr.with_directory.return_value = mock_ctr
        mock_ctr.with_workdir.return_value = mock_ctr
        mock_ctr.with_env_variable.return_value = mock_ctr
        mock_ctr.with_exec.return_value = mock_ctr

        result = static_build_curl(mock_client, mock_ctr)

        git_url = mock_client.git.call_args[1]["url"]
        assert "github.com/curl/curl" in git_url


class TestStaticBuildIoping:
    """Test static_build_ioping function."""

    def test_static_build_ioping_basic(self):
        """Should build ioping tool."""
        mock_client = Mock(spec=dagger.Client)
        mock_ctr = Mock(spec=dagger.Container)
        mock_git = Mock()
        mock_tree = Mock()

        mock_client.git.return_value = mock_git
        mock_git.tag.return_value = mock_git
        mock_git.tree.return_value = mock_tree

        mock_ctr.with_directory.return_value = mock_ctr
        mock_ctr.with_workdir.return_value = mock_ctr
        mock_ctr.with_env_variable.return_value = mock_ctr
        mock_ctr.with_exec.return_value = mock_ctr

        result = static_build_ioping(mock_client, mock_ctr)

        assert result is not None

    def test_static_build_ioping_repo_url(self):
        """Should use correct ioping repository."""
        mock_client = Mock(spec=dagger.Client)
        mock_ctr = Mock(spec=dagger.Container)
        mock_git = Mock()
        mock_tree = Mock()

        mock_client.git.return_value = mock_git
        mock_git.tag.return_value = mock_git
        mock_git.tree.return_value = mock_tree

        mock_ctr.with_directory.return_value = mock_ctr
        mock_ctr.with_workdir.return_value = mock_ctr
        mock_ctr.with_env_variable.return_value = mock_ctr
        mock_ctr.with_exec.return_value = mock_ctr

        result = static_build_ioping(mock_client, mock_ctr)

        git_url = mock_client.git.call_args[1]["url"]
        assert "ioping" in git_url


class TestStaticBuildLibnetfilterAcct:
    """Test static_build_libnetfilter_acct function."""

    def test_static_build_libnetfilter_acct_basic(self):
        """Should build libnetfilter_acct."""
        mock_client = Mock(spec=dagger.Client)
        mock_ctr = Mock(spec=dagger.Container)
        mock_git = Mock()
        mock_tree = Mock()

        mock_client.git.return_value = mock_git
        mock_git.tag.return_value = mock_git
        mock_git.tree.return_value = mock_tree

        mock_ctr.with_directory.return_value = mock_ctr
        mock_ctr.with_workdir.return_value = mock_ctr
        mock_ctr.with_env_variable.return_value = mock_ctr
        mock_ctr.with_exec.return_value = mock_ctr

        result = static_build_libnetfilter_acct(mock_client, mock_ctr)

        assert result is not None

    def test_static_build_libnetfilter_acct_repo(self):
        """Should use netfilter.org repository."""
        mock_client = Mock(spec=dagger.Client)
        mock_ctr = Mock(spec=dagger.Container)
        mock_git = Mock()
        mock_tree = Mock()

        mock_client.git.return_value = mock_git
        mock_git.tag.return_value = mock_git
        mock_git.tree.return_value = mock_tree

        mock_ctr.with_directory.return_value = mock_ctr
        mock_ctr.with_workdir.return_value = mock_ctr
        mock_ctr.with_env_variable.return_value = mock_ctr
        mock_ctr.with_exec.return_value = mock_ctr

        result = static_build_libnetfilter_acct(mock_client, mock_ctr)

        git_url = mock_client.git.call_args[1]["url"]
        assert "netfilter" in git_url


class TestStaticBuildNetdata:
    """Test static_build_netdata function."""

    def test_static_build_netdata_basic(self):
        """Should build netdata."""
        mock_client = Mock(spec=dagger.Client)
        mock_ctr = Mock(spec=dagger.Container)

        mock_ctr.with_workdir.return_value = mock_ctr
        mock_ctr.with_env_variable.return_value = mock_ctr
        mock_ctr.with_exec.return_value = mock_ctr

        result = static_build_netdata(mock_client, mock_ctr)

        assert result is not None

    def test_static_build_netdata_environment_variables(self):
        """Should set required environment variables."""
        mock_client = Mock(spec=dagger.Client)
        mock_ctr = Mock(spec=dagger.Container)

        mock_ctr.with_workdir.return_value = mock_ctr
        mock_ctr.with_env_variable.return_value = mock_ctr
        mock_ctr.with_exec.return_value = mock_ctr

        result = static_build_netdata(mock_client, mock_ctr)

        env_calls = [call[0][0] for call in mock_ctr.with_env_variable.call_args_list]
        expected_vars = ["CFLAGS", "LDFLAGS", "PKG_CONFIG", "PKG_CONFIG_PATH"]
        for var in expected_vars:
            assert var in env_calls

    def test_static_build_netdata_special_flags(self):
        """Should set special netdata build flags."""
        mock_client = Mock(spec=dagger.Client)
        mock_ctr = Mock(spec=dagger.Container)

        mock_ctr.with_workdir.return_value = mock_ctr
        mock_ctr.with_env_variable.return_value = mock_ctr
        mock_ctr.with_exec.return_value = mock_ctr

        result = static_build_netdata(mock_client, mock_ctr)

        env_dict = {}
        for call in mock_ctr.with_env_variable.call_args_list:
            env_dict[call[0][0]] = call[0][1]

        assert "CMAKE_FLAGS" in env_dict
        assert "EBPF_LIBC" in env_dict
        assert env_dict.get("EBPF_LIBC") == "static"
        assert "IS_NETDATA_STATIC_BINARY" in env_dict
        assert env_dict.get("IS_NETDATA_STATIC_BINARY") == "yes"


class TestStaticBuild:
    """Test static_build function."""

    @patch("imageutils.Path.home")
    @patch("imageutils.os.path.join")
    def test_static_build_basic(self, mock_join, mock_home):
        """Should orchestrate full static build."""
        mock_home.return_value = Path("/home/user")
        mock_join.side_effect = lambda *args: "/".join(args)

        mock_client = Mock(spec=dagger.Client)
        mock_container = Mock(spec=dagger.Container)
        mock_directory = Mock()

        # Mock all build function returns
        with patch("imageutils.build_alpine_3_18") as mock_build_alpine:
            with patch(
                "imageutils.static_build_openssl"
            ) as mock_build_openssl:
                with patch("imageutils.static_build_bash") as mock_build_bash:
                    with patch("imageutils.static_build_curl") as mock_build_curl:
                        with patch(
                            "imageutils.static_build_ioping"
                        ) as mock_build_ioping:
                            with patch(
                                "imageutils.static_build_libnetfilter_acct"
                            ) as mock_build_libnetfilter:
                                with patch(
                                    "imageutils.static_build_netdata"
                                ) as mock_build_netdata:
                                    mock_build_alpine.return_value = mock_container
                                    mock_build_openssl.return_value = mock_container
                                    mock_build_bash.return_value = mock_container
                                    mock_build_curl.return_value = mock_container
                                    mock_build_ioping.return_value = mock_container
                                    mock_build_libnetfilter.return_value = (
                                        mock_container
                                    )
                                    mock_build_netdata.return_value = mock_container

                                    mock_container.with_directory.return_value = (
                                        mock_container
                                    )
                                    mock_container.directory.return_value = (
                                        mock_directory
                                    )
                                    mock_directory.export.return_value = "export_task"

                                    mock_client.host.return_value.directory.return_value = (
                                        Mock()
                                    )

                                    result = static_build(mock_client, "/repo/path")

                                    assert result is not None

    @patch("imageutils.Path.home")
    @patch("imageutils.os.path.join")
    def test_static_build_calls_all_builders(self, mock_join, mock_home):
        """Should call all build functions in correct order."""
        mock_home.return_value = Path("/home/user")
        mock_join.side_effect = lambda *args: "/".join(args)

        mock_client = Mock(spec=dagger.Client)
        mock_container = Mock(spec=dagger.Container)
        mock_directory = Mock()

        with patch("imageutils.build_alpine_3_18") as mock_build_alpine:
            with patch(
                "imageutils.static_build_openssl"
            ) as mock_build_openssl:
                with patch("imageutils.static_build_bash") as mock_build_bash:
                    with patch("imageutils.static_build_curl") as mock_build_curl:
                        with patch(
                            "imageutils.static_build_ioping"
                        ) as mock_build_ioping:
                            with patch(
                                "imageutils.static_build_libnetfilter_acct"
                            ) as mock_build_libnetfilter:
                                with patch(
                                    "imageutils.static_build_netdata"
                                ) as mock_build_netdata:
                                    mock_build_alpine.return_value = mock_container
                                    mock_build_openssl.return_value = mock_container
                                    mock_build_bash.return_value = mock_container
                                    mock_build_curl.return_value = mock_container
                                    mock_build_ioping.return_value = mock_container
                                    mock_build_libnetfilter.return_value = (
                                        mock_container
                                    )
                                    mock_build_netdata.return_value = mock_container

                                    mock_container.with_directory.return_value = (
                                        mock_container
                                    )
                                    mock_container.directory.return_value = (
                                        mock_directory
                                    )
                                    mock_directory.export.return_value = "export_task"

                                    mock_client.host.return_value.directory.return_value = (
                                        Mock()
                                    )

                                    result = static_build(mock_client, "/repo/path")

                                    mock_build_alpine.assert_called_once()
                                    mock_build_openssl.assert_called_once()
                                    mock_build_bash.assert_called_once()
                                    mock_build_curl.assert_called_once()
                                    mock_build_ioping.assert_called_once()
                                    mock_build_libnetfilter.assert_called_once()
                                    mock_build_netdata.assert_called_once()


class TestCentosCommonPackages:
    """Test CentOS common packages list."""

    def test_centos_common_packages_exists(self):
        """Should have CentOS common packages list."""
        assert isinstance(_CENTOS_COMMON_PACKAGES, list)
        assert len(_CENTOS_COMMON_PACKAGES) > 0

    def test_centos_common_packages_contains_build_tools(self):
        """Should contain build tools."""
        expected = ["gcc", "cmake", "make"]
        for pkg in expected:
            assert pkg in _CENTOS_COMMON_PACKAGES


class TestBuildAmazonLinux2:
    """Test build_amazon_linux_2 function."""

    def test_build_amazon_linux_2_x86_64(self):
        """Should build Amazon Linux 2 for x86_64."""
        mock_client = Mock(spec=dagger.Client)
        mock_container = Mock(spec=dagger.Container)
        mock_platform = dagger.Platform("linux/x86_64")

        mock_client.container.return_value = mock_container
        mock_container.from_.return_value = mock_container
        mock_container.with_exec.return_value = mock_container
        mock_container.with_file.return_value = mock_container

        with patch("imageutils.Path") as mock_path_class:
            mock_path_instance = Mock()
            mock_path_class.return_value = mock_path_instance
            mock_path_instance.parent = Mock()
            mock_path_instance.parent.__truediv__ = Mock(return_value="path")

            result = build_amazon_linux_2(mock_client, mock_platform)

            assert result is not None

    def test_build_amazon_linux_2_arm64(self):
        """Should build Amazon Linux 2 for ARM64."""
        mock_client = Mock(spec=dagger.Client)
        mock_container = Mock(spec=dagger.Container)
        mock_platform = dagger.Platform("linux/arm64")

        mock_client.container.return_value = mock_container
        mock_container.from_.return_value = mock_container
        mock_container.with_exec.return_value = mock_container
        mock_container.with_file.return_value = mock_container

        with patch("imageutils.Path") as mock_path_class:
            mock_path_instance = Mock()
            mock_path_class.return_value = mock_path_instance
            mock_path_instance.parent = Mock()
            mock_path_instance.parent.__truediv__ = Mock(return_value="path")

            result = build_amazon_linux_2(mock_client, mock_platform)

            assert result is not None

    def test_build_amazon_linux_2_invalid_platform(self):
        """Should raise error for unsupported platform."""
        mock_client = Mock(spec=dagger.Client)
        mock_container = Mock(spec=dagger.Container)
        mock_platform = dagger.Platform("linux/ppc64le")

        mock_client.container.return_value = mock_container
        mock_container.from_.return_value = mock_container
        mock_container.with_exec.return_value = mock_container

        with pytest.raises(Exception) as exc_info:
            build_amazon_linux_2(mock_client, mock_platform)
        assert "supports only" in str(exc_info.value)


class TestBuildCentos7:
    """Test build_centos_7 function."""

    def test_build_centos_7_x86_64(self):
        """Should build CentOS 7 for x86_64."""
        mock_client = Mock(spec=dagger.Client)
        mock_container = Mock(spec=dagger.Container)
        mock_platform = dagger.Platform("linux/x86_64")

        mock_client.container.return_value = mock_container
        mock_container.from_.return_value = mock_container
        mock_container.with_exec.return_value = mock_container
        mock_container.with_file.return_value = mock_container

        with patch("imageutils.Path") as mock_path_class:
            mock_path_instance = Mock()
            mock_path_class.return_value = mock_path_instance
            mock_path_instance.parent = Mock()
            mock_path_instance.parent.__truediv__ = Mock(return_value="path")

            result = build_centos_7(mock_client, mock_platform)

            assert result is not None

    def test_build_centos_7_arm64(self):
        """Should build CentOS 7 for ARM64."""
        mock_client = Mock(spec=dagger.Client)
        mock_container = Mock(spec=dagger.Container)
        mock_platform = dagger.Platform("linux/arm64")

        mock_client.container.return_value = mock_container
        mock_container.from_.return_value = mock_container
        mock_container.with_exec.return_value = mock_container
        mock_container.with_file.return_value = mock_container

        with patch("imageutils.Path") as mock_path_class:
            mock_path_instance = Mock()
            mock_path_class.return_value = mock_path_instance
            mock_path_instance.parent = Mock()
            mock_path_instance.parent.__truediv__ = Mock(return_value="path")

            result = build_centos_7(mock_client, mock_platform)

            assert result is not None

    def test_build_centos_7_invalid_platform(self):
        """Should raise error for unsupported platform."""
        mock_client = Mock(spec=dagger.Client)
        mock_container = Mock(spec=dagger.Container)
        mock_platform = dagger.Platform("linux/mips")

        mock_client.container.return_value = mock_container
        mock_container.from_.return_value = mock_container
        mock_container.with_exec.return_value = mock_container

        with pytest.raises(Exception) as exc_info:
            build_centos_7(mock_client, mock_platform)
        assert "supports only" in str(exc_info.value)


class TestRockyLinuxCommonPackages:
    """Test Rocky Linux common packages list."""

    def test_rocky_linux_common_packages_exists(self):
        """Should have Rocky Linux common packages."""
        assert isinstance(_ROCKY_LINUX
import unittest
import uuid
import tempfile
import pathlib
import os
from unittest.mock import Mock, MagicMock, patch, mock_open
import sys

# Mock dagger and jinja2 before importing the module
sys.modules['dagger'] = MagicMock()
sys.modules['jinja2'] = MagicMock()
sys.modules['imageutils'] = MagicMock()

import dagger
import jinja2
import imageutils

# Now import the module to test
from packaging.dag.nd import (
    Platform,
    SUPPORTED_PLATFORMS,
    SUPPORTED_DISTRIBUTIONS,
    Distribution,
    FeatureFlags,
    NetdataInstaller,
    Endpoint,
    ChildStreamConf,
    ParentStreamConf,
    StreamConf,
    AgentContext,
)


class TestPlatform(unittest.TestCase):
    """Test cases for Platform class"""

    def test_platform_init(self):
        """Test Platform initialization"""
        platform = Platform("linux/x86_64")
        self.assertIsNotNone(platform.platform)

    def test_platform_escaped_removes_linux_prefix(self):
        """Test that escaped() removes linux/ prefix"""
        platform = Platform("linux/x86_64")
        result = platform.escaped()
        self.assertEqual(result, "x86_64")

    def test_platform_escaped_replaces_slashes(self):
        """Test that escaped() replaces / with _"""
        platform = Platform("linux/arm/v7")
        result = platform.escaped()
        self.assertEqual(result, "arm_v7")

    def test_platform_escaped_handles_multiple_slashes(self):
        """Test that escaped() handles multiple slashes"""
        platform = Platform("linux/arm/v6")
        result = platform.escaped()
        self.assertEqual(result, "arm_v6")

    def test_platform_equality_with_platform_instance(self):
        """Test Platform equality with another Platform instance"""
        platform1 = Platform("linux/x86_64")
        platform2 = Platform("linux/x86_64")
        self.assertEqual(platform1, platform2)

    def test_platform_inequality_with_different_platforms(self):
        """Test Platform inequality with different platforms"""
        platform1 = Platform("linux/x86_64")
        platform2 = Platform("linux/arm64")
        self.assertNotEqual(platform1, platform2)

    def test_platform_equality_with_dagger_platform(self):
        """Test Platform equality with dagger.Platform"""
        platform1 = Platform("linux/x86_64")
        mock_dagger_platform = Mock()
        platform1.platform = mock_dagger_platform
        self.assertEqual(platform1, mock_dagger_platform)

    def test_platform_equality_returns_not_implemented_for_other_types(self):
        """Test Platform equality returns NotImplemented for unsupported types"""
        platform = Platform("linux/x86_64")
        result = platform.__eq__("not_a_platform")
        self.assertEqual(result, NotImplemented)

    def test_platform_inequality_with_different_platforms(self):
        """Test Platform __ne__ operator"""
        platform1 = Platform("linux/x86_64")
        platform2 = Platform("linux/arm64")
        self.assertTrue(platform1 != platform2)

    def test_platform_inequality_returns_false_for_equal(self):
        """Test Platform __ne__ returns False for equal platforms"""
        platform1 = Platform("linux/x86_64")
        platform2 = Platform("linux/x86_64")
        self.assertFalse(platform1 != platform2)

    def test_platform_hash(self):
        """Test Platform hash function"""
        platform1 = Platform("linux/x86_64")
        platform2 = Platform("linux/x86_64")
        self.assertEqual(hash(platform1), hash(platform2))

    def test_platform_hash_different_for_different_platforms(self):
        """Test Platform hash is different for different platforms"""
        platform1 = Platform("linux/x86_64")
        platform2 = Platform("linux/arm64")
        self.assertNotEqual(hash(platform1), hash(platform2))

    def test_platform_str(self):
        """Test Platform string representation"""
        platform = Platform("linux/x86_64")
        self.assertEqual(str(platform), str(platform.platform))

    def test_platform_can_be_used_in_set(self):
        """Test that Platform can be used in a set"""
        platform1 = Platform("linux/x86_64")
        platform2 = Platform("linux/x86_64")
        platform3 = Platform("linux/arm64")
        s = {platform1, platform2, platform3}
        # Should have 2 unique platforms since platform1 and platform2 are equal
        self.assertEqual(len(s), 2)

    def test_supported_platforms_contains_expected_platforms(self):
        """Test SUPPORTED_PLATFORMS contains expected values"""
        self.assertGreater(len(SUPPORTED_PLATFORMS), 0)
        platform_strings = [str(p) for p in SUPPORTED_PLATFORMS]
        self.assertIn("linux/x86_64", platform_strings)
        self.assertIn("linux/arm64", platform_strings)

    def test_supported_distributions_contains_expected_distributions(self):
        """Test SUPPORTED_DISTRIBUTIONS contains expected values"""
        self.assertIn("alpine_3_18", SUPPORTED_DISTRIBUTIONS)
        self.assertIn("ubuntu22.04", SUPPORTED_DISTRIBUTIONS)
        self.assertEqual(len(SUPPORTED_DISTRIBUTIONS), 24)


class TestDistribution(unittest.TestCase):
    """Test cases for Distribution class"""

    def test_distribution_alpine_3_18(self):
        """Test Distribution initialization for Alpine 3.18"""
        dist = Distribution("alpine_3_18")
        self.assertEqual(dist.display_name, "alpine_3_18")
        self.assertEqual(dist.docker_tag, "alpine:3.18")
        self.assertEqual(dist.builder, imageutils.build_alpine_3_18)
        self.assertEqual(dist.platforms, SUPPORTED_PLATFORMS)

    def test_distribution_alpine_3_19(self):
        """Test Distribution initialization for Alpine 3.19"""
        dist = Distribution("alpine_3_19")
        self.assertEqual(dist.display_name, "alpine_3_19")
        self.assertEqual(dist.docker_tag, "alpine:3.19")

    def test_distribution_amazonlinux2(self):
        """Test Distribution initialization for Amazon Linux 2"""
        dist = Distribution("amazonlinux2")
        self.assertEqual(dist.display_name, "amazonlinux2")
        self.assertEqual(dist.docker_tag, "amazonlinux:2")
        self.assertEqual(dist.builder, imageutils.build_amazon_linux_2)

    def test_distribution_centos7(self):
        """Test Distribution initialization for CentOS 7"""
        dist = Distribution("centos7")
        self.assertEqual(dist.docker_tag, "centos:7")
        self.assertEqual(dist.builder, imageutils.build_centos_7)

    def test_distribution_centos_stream8(self):
        """Test Distribution initialization for CentOS Stream 8"""
        dist = Distribution("centos-stream8")
        self.assertEqual(dist.docker_tag, "quay.io/centos/centos:stream8")
        self.assertEqual(dist.builder, imageutils.build_centos_stream_8)

    def test_distribution_centos_stream9(self):
        """Test Distribution initialization for CentOS Stream 9"""
        dist = Distribution("centos-stream9")
        self.assertEqual(dist.docker_tag, "quay.io/centos/centos:stream9")
        self.assertEqual(dist.builder, imageutils.build_centos_stream_9)

    def test_distribution_debian10(self):
        """Test Distribution initialization for Debian 10"""
        dist = Distribution("debian10")
        self.assertEqual(dist.docker_tag, "debian:10")
        self.assertEqual(dist.builder, imageutils.build_debian_10)

    def test_distribution_debian11(self):
        """Test Distribution initialization for Debian 11"""
        dist = Distribution("debian11")
        self.assertEqual(dist.docker_tag, "debian:11")
        self.assertEqual(dist.builder, imageutils.build_debian_11)

    def test_distribution_debian12(self):
        """Test Distribution initialization for Debian 12"""
        dist = Distribution("debian12")
        self.assertEqual(dist.docker_tag, "debian:12")
        self.assertEqual(dist.builder, imageutils.build_debian_12)

    def test_distribution_fedora37(self):
        """Test Distribution initialization for Fedora 37"""
        dist = Distribution("fedora37")
        self.assertEqual(dist.docker_tag, "fedora:37")
        self.assertEqual(dist.builder, imageutils.build_fedora_37)

    def test_distribution_fedora38(self):
        """Test Distribution initialization for Fedora 38"""
        dist = Distribution("fedora38")
        self.assertEqual(dist.docker_tag, "fedora:38")
        self.assertEqual(dist.builder, imageutils.build_fedora_38)

    def test_distribution_fedora39(self):
        """Test Distribution initialization for Fedora 39"""
        dist = Distribution("fedora39")
        self.assertEqual(dist.docker_tag, "fedora:39")
        self.assertEqual(dist.builder, imageutils.build_fedora_39)

    def test_distribution_opensuse_15_4(self):
        """Test Distribution initialization for openSUSE 15.4"""
        dist = Distribution("opensuse15.4")
        self.assertEqual(dist.docker_tag, "opensuse/leap:15.4")
        self.assertEqual(dist.builder, imageutils.build_opensuse_15_4)

    def test_distribution_opensuse_15_5(self):
        """Test Distribution initialization for openSUSE 15.5"""
        dist = Distribution("opensuse15.5")
        self.assertEqual(dist.docker_tag, "opensuse/leap:15.5")
        self.assertEqual(dist.builder, imageutils.build_opensuse_15_5)

    def test_distribution_opensuse_tumbleweed(self):
        """Test Distribution initialization for openSUSE Tumbleweed"""
        dist = Distribution("opensusetumbleweed")
        self.assertEqual(dist.docker_tag, "opensuse/tumbleweed:latest")
        self.assertEqual(dist.builder, imageutils.build_opensuse_tumbleweed)

    def test_distribution_oraclelinux8(self):
        """Test Distribution initialization for Oracle Linux 8"""
        dist = Distribution("oraclelinux8")
        self.assertEqual(dist.docker_tag, "oraclelinux:8")
        self.assertEqual(dist.builder, imageutils.build_oracle_linux_8)

    def test_distribution_oraclelinux9(self):
        """Test Distribution initialization for Oracle Linux 9"""
        dist = Distribution("oraclelinux9")
        self.assertEqual(dist.docker_tag, "oraclelinux:9")
        self.assertEqual(dist.builder, imageutils.build_oracle_linux_9)

    def test_distribution_rockylinux8(self):
        """Test Distribution initialization for Rocky Linux 8"""
        dist = Distribution("rockylinux8")
        self.assertEqual(dist.docker_tag, "rockylinux:8")
        self.assertEqual(dist.builder, imageutils.build_rocky_linux_8)

    def test_distribution_rockylinux9(self):
        """Test Distribution initialization for Rocky Linux 9"""
        dist = Distribution("rockylinux9")
        self.assertEqual(dist.docker_tag, "rockylinux:9")
        self.assertEqual(dist.builder, imageutils.build_rocky_linux_9)

    def test_distribution_ubuntu20_04(self):
        """Test Distribution initialization for Ubuntu 20.04"""
        dist = Distribution("ubuntu20.04")
        self.assertEqual(dist.docker_tag, "ubuntu:20.04")
        self.assertEqual(dist.builder, imageutils.build_ubuntu_20_04)

    def test_distribution_ubuntu22_04(self):
        """Test Distribution initialization for Ubuntu 22.04"""
        dist = Distribution("ubuntu22.04")
        self.assertEqual(dist.docker_tag, "ubuntu:22.04")
        self.assertEqual(dist.builder, imageutils.build_ubuntu_22_04)

    def test_distribution_ubuntu23_04(self):
        """Test Distribution initialization for Ubuntu 23.04"""
        dist = Distribution("ubuntu23.04")
        self.assertEqual(dist.docker_tag, "ubuntu:23.04")
        self.assertEqual(dist.builder, imageutils.build_ubuntu_23_04)

    def test_distribution_ubuntu23_10(self):
        """Test Distribution initialization for Ubuntu 23.10"""
        dist = Distribution("ubuntu23.10")
        self.assertEqual(dist.docker_tag, "ubuntu:23.10")
        self.assertEqual(dist.builder, imageutils.build_ubuntu_23_10)

    def test_distribution_unknown_raises_error(self):
        """Test Distribution raises ValueError for unknown distribution"""
        with self.assertRaises(ValueError) as context:
            Distribution("unknown_distro")
        self.assertIn("Unknown distribution", str(context.exception))

    def test_distribution_cache_volume(self):
        """Test _cache_volume method"""
        dist = Distribution("ubuntu22.04")
        mock_client = Mock()
        mock_cache_volume = Mock()
        mock_client.cache_volume.return_value = mock_cache_volume
        mock_platform = Mock()

        result = dist._cache_volume(mock_client, mock_platform, "/test/path")

        mock_client.cache_volume.assert_called_once()
        self.assertEqual(result, mock_cache_volume)

    def test_distribution_build_supported_platform(self):
        """Test build method with supported platform"""
        dist = Distribution("ubuntu22.04")
        mock_client = Mock()
        mock_container = Mock()
        mock_platform = list(SUPPORTED_PLATFORMS)[0].platform

        dist.builder = Mock(return_value=mock_container)
        imageutils.install_cargo = Mock(return_value=mock_container)

        result = dist.build(mock_client, mock_platform)

        dist.builder.assert_called_once_with(mock_client, mock_platform)
        imageutils.install_cargo.assert_called_once_with(mock_container)
        self.assertEqual(result, mock_container)

    def test_distribution_build_unsupported_platform_raises_error(self):
        """Test build method raises error for unsupported platform"""
        dist = Distribution("ubuntu22.04")
        dist.platforms = set()
        mock_client = Mock()
        mock_platform = Mock()

        with self.assertRaises(ValueError) as context:
            dist.build(mock_client, mock_platform)
        self.assertIn("not supported", str(context.exception))


class TestFeatureFlags(unittest.TestCase):
    """Test cases for FeatureFlags enum"""

    def test_feature_flags_dbengine(self):
        """Test FeatureFlags.DBEngine exists"""
        self.assertTrue(hasattr(FeatureFlags, 'DBEngine'))

    def test_feature_flags_goplugin(self):
        """Test FeatureFlags.GoPlugin exists"""
        self.assertTrue(hasattr(FeatureFlags, 'GoPlugin'))

    def test_feature_flags_extendedbpf(self):
        """Test FeatureFlags.ExtendedBPF exists"""
        self.assertTrue(hasattr(FeatureFlags, 'ExtendedBPF'))

    def test_feature_flags_logsmanagement(self):
        """Test FeatureFlags.LogsManagement exists"""
        self.assertTrue(hasattr(FeatureFlags, 'LogsManagement'))

    def test_feature_flags_machinelearning(self):
        """Test FeatureFlags.MachineLearning exists"""
        self.assertTrue(hasattr(FeatureFlags, 'MachineLearning'))

    def test_feature_flags_bundledprotobuf(self):
        """Test FeatureFlags.BundledProtobuf exists"""
        self.assertTrue(hasattr(FeatureFlags, 'BundledProtobuf'))

    def test_feature_flags_combination(self):
        """Test combining FeatureFlags"""
        flags = FeatureFlags.DBEngine | FeatureFlags.GoPlugin
        self.assertIn(FeatureFlags.DBEngine, flags)
        self.assertIn(FeatureFlags.GoPlugin, flags)

    def test_feature_flags_membership_check(self):
        """Test checking if flag is in combined flags"""
        flags = FeatureFlags.DBEngine | FeatureFlags.GoPlugin
        self.assertTrue(FeatureFlags.DBEngine in flags)
        self.assertFalse(FeatureFlags.ExtendedBPF in flags)


class TestNetdataInstaller(unittest.TestCase):
    """Test cases for NetdataInstaller class"""

    def setUp(self):
        """Set up test fixtures"""
        self.mock_platform = Mock(spec=Platform)
        self.mock_distro = Mock(spec=Distribution)
        self.repo_root = pathlib.Path("/repo")
        self.prefix = pathlib.Path("/opt/netdata")
        self.features = FeatureFlags.DBEngine

    def test_netdata_installer_init(self):
        """Test NetdataInstaller initialization"""
        installer = NetdataInstaller(
            self.mock_platform,
            self.mock_distro,
            self.repo_root,
            self.prefix,
            self.features
        )
        self.assertEqual(installer.platform, self.mock_platform)
        self.assertEqual(installer.distro, self.mock_distro)
        self.assertEqual(installer.repo_root, self.repo_root)
        self.assertEqual(installer.prefix, self.prefix)
        self.assertEqual(installer.features, self.features)

    @patch('pathlib.Path.as_posix')
    def test_mount_repo(self, mock_as_posix):
        """Test _mount_repo method"""
        mock_as_posix.return_value = "/repo"
        installer = NetdataInstaller(
            self.mock_platform,
            self.mock_distro,
            self.repo_root,
            self.prefix,
            self.features
        )
        mock_client = Mock()
        mock_ctr = Mock()
        mock_host = Mock()
        mock_directory = Mock()
        mock_host.directory.return_value = mock_directory
        mock_client.host.return_value = mock_host
        mock_ctr.with_directory.return_value = mock_ctr
        mock_ctr.with_workdir.return_value = mock_ctr
        mock_ctr.with_mounted_cache.return_value = mock_ctr

        self.mock_distro._cache_volume.return_value = Mock()

        result = installer._mount_repo(mock_client, mock_ctr, self.repo_root)

        self.assertEqual(result, mock_ctr)
        mock_ctr.with_directory.assert_called_once()
        mock_ctr.with_workdir.assert_called_once()
        mock_ctr.with_mounted_cache.assert_called_once()

    def test_install_with_all_features_enabled(self):
        """Test install method with all features enabled"""
        all_features = (
            FeatureFlags.DBEngine | FeatureFlags.GoPlugin |
            FeatureFlags.ExtendedBPF | FeatureFlags.LogsManagement |
            FeatureFlags.MachineLearning | FeatureFlags.BundledProtobuf
        )
        installer = NetdataInstaller(
            self.mock_platform,
            self.mock_distro,
            self.repo_root,
            self.prefix,
            all_features
        )
        mock_client = Mock()
        mock_ctr = Mock()
        mock_ctr.with_env_variable.return_value = mock_ctr
        mock_ctr.with_exec.return_value = mock_ctr

        installer._mount_repo = Mock(return_value=mock_ctr)

        result = installer.install(mock_client, mock_ctr)

        # Should have basic args only (no disable flags)
        self.assertEqual(result, mock_ctr)
        calls = mock_ctr.with_exec.call_args_list
        self.assertTrue(len(calls) > 0)

    def test_install_with_no_features_enabled(self):
        """Test install method with no features enabled"""
        installer = NetdataInstaller(
            self.mock_platform,
            self.mock_distro,
            self.repo_root,
            self.prefix,
            FeatureFlags(0)  # No flags set
        )
        mock_client = Mock()
        mock_ctr = Mock()
        mock_ctr.with_env_variable.return_value = mock_ctr
        mock_ctr.with_exec.return_value = mock_ctr

        installer._mount_repo = Mock(return_value=mock_ctr)

        result = installer.install(mock_client, mock_ctr)

        # Should have disable flags
        self.assertEqual(result, mock_ctr)
        calls = mock_ctr.with_exec.call_args_list
        self.assertTrue(len(calls) > 0)
        # Check that exec was called with disable flags
        exec_args = calls[0][0][0]
        self.assertIn("--disable-dbengine", exec_args)
        self.assertIn("--disable-go", exec_args)
        self.assertIn("--disable-ebpf", exec_args)
        self.assertIn("--disable-ml", exec_args)

    def test_install_dbengine_disabled(self):
        """Test install with DBEngine disabled"""
        features = FeatureFlags.GoPlugin | FeatureFlags.ExtendedBPF
        installer = NetdataInstaller(
            self.mock_platform,
            self.mock_distro,
            self.repo_root,
            self.prefix,
            features
        )
        mock_client = Mock()
        mock_ctr = Mock()
        mock_ctr.with_env_variable.return_value = mock_ctr
        mock_ctr.with_exec.return_value = mock_ctr

        installer._mount_repo = Mock(return_value=mock_ctr)

        result = installer.install(mock_client, mock_ctr)

        calls = mock_ctr.with_exec.call_args_list
        exec_args = calls[0][0][0]
        self.assertIn("--disable-dbengine", exec_args)

    def test_install_goplugin_disabled(self):
        """Test install with GoPlugin disabled"""
        features = FeatureFlags.DBEngine | FeatureFlags.ExtendedBPF
        installer = NetdataInstaller(
            self.mock_platform,
            self.mock_distro,
            self.repo_root,
            self.prefix,
            features
        )
        mock_client = Mock()
        mock_ctr = Mock()
        mock_ctr.with_env_variable.return_value = mock_ctr
        mock_ctr.with_exec.return_value = mock_ctr

        installer._mount_repo = Mock(return_value=mock_ctr)

        result = installer.install(mock_client, mock_ctr)

        calls = mock_ctr.with_exec.call_args_list
        exec_args = calls[0][0][0]
        self.assertIn("--disable-go", exec_args)

    def test_install_ebpf_disabled(self):
        """Test install with ExtendedBPF disabled"""
        features = FeatureFlags.DBEngine | FeatureFlags.GoPlugin
        installer = NetdataInstaller(
            self.mock_platform,
            self.mock_distro,
            self.repo_root,
            self.prefix,
            features
        )
        mock_client = Mock()
        mock_ctr = Mock()
        mock_ctr.with_env_variable.return_value = mock_ctr
        mock_ctr.with_exec.return_value = mock_ctr

        installer._mount_repo = Mock(return_value=mock_ctr)

        result = installer.install(mock_client, mock_ctr)

        calls = mock_ctr.with_exec.call_args_list
        exec_args = calls[0][0][0]
        self.assertIn("--disable-ebpf", exec_args)

    def test_install_ml_disabled(self):
        """Test install with MachineLearning disabled"""
        features = FeatureFlags.DBEngine | FeatureFlags.GoPlugin
        installer = NetdataInstaller(
            self.mock_platform,
            self.mock_distro,
            self.repo_root,
            self.prefix,
            features
        )
        mock_client = Mock()
        mock_ctr = Mock()
        mock_ctr.with_env_variable.return_value = mock_ctr
        mock_ctr.with_exec.return_value = mock_ctr

        installer._mount_repo = Mock(return_value=mock_ctr)

        result = installer.install(mock_client, mock_ctr)

        calls = mock_ctr.with_exec.call_args_list
        exec_args = calls[0][0][0]
        self.assertIn("--disable-ml", exec_args)

    def test_install_bundled_protobuf_disabled(self):
        """Test install with BundledProtobuf disabled"""
        features = FeatureFlags.DBEngine | FeatureFlags.GoPlugin
        installer = NetdataInstaller(
            self.mock_platform,
            self.mock_distro,
            self.repo_root,
            self.prefix,
            features
        )
        mock_client = Mock()
        mock_ctr = Mock()
        mock_ctr.with_env_variable.return_value = mock_ctr
        mock_ctr.with_exec.return_value = mock_ctr

        installer._mount_repo = Mock(return_value=mock_ctr)

        result = installer.install(mock_client, mock_ctr)

        calls = mock_ctr.with_exec.call_args_list
        exec_args = calls[0][0][0]
        self.assertIn("--use-system-protobuf", exec_args)


class TestEndpoint(unittest.TestCase):
    """Test cases for Endpoint class"""

    def test_endpoint_init(self):
        """Test Endpoint initialization"""
        endpoint = Endpoint("localhost", 19999)
        self.assertEqual(endpoint.hostname, "localhost")
        self.assertEqual(endpoint.port, 19999)

    def test_endpoint_init_with_different_hostname(self):
        """Test Endpoint with different hostname"""
        endpoint = Endpoint("example.com", 8080)
        self.assertEqual(endpoint.hostname, "example.com")
        self.assertEqual(endpoint.port, 8080)

    def test_endpoint_str(self):
        """Test Endpoint string representation"""
        endpoint = Endpoint("localhost", 19999)
        self.assertEqual(str(endpoint), "localhost:19999")

    def test_endpoint_str_with_different_values(self):
        """Test Endpoint string representation with different values"""
        endpoint = Endpoint("127.0.0.1", 8080)
        self.assertEqual(str(endpoint), "127.0.0.1:8080")

    def test_endpoint_with_zero_port(self):
        """Test Endpoint with port 0"""
        endpoint = Endpoint("localhost", 0)
        self.assertEqual(str(endpoint), "localhost:0")

    def test_endpoint_with_high_port(self):
        """Test Endpoint with high port number"""
        endpoint = Endpoint("localhost", 65535)
        self.assertEqual(str(endpoint), "localhost:65535")


class TestChildStreamConf(unittest.TestCase):
    """Test cases for ChildStreamConf class"""

    def setUp(self):
        """Set up test fixtures"""
        self.mock_installer = Mock(spec=NetdataInstaller)
        self.destinations = [Endpoint("parent1", 19999), Endpoint("parent2", 19999)]
        self.api_key = uuid.uuid4()

    def test_child_stream_conf_init(self):
        """Test ChildStreamConf initialization"""
        conf = ChildStreamConf(self.mock_installer, self.destinations, self.api_key)
        self.assertEqual(conf.installer, self.mock_installer)
        self.assertIn("enabled", conf.substitutions)
        self.assertEqual(conf.substitutions["enabled"], "yes")

    def test_child_stream_conf_destinations(self):
        """Test ChildStreamConf destination substitution"""
        conf = ChildStreamConf(self.mock_installer, self.destinations, self.api_key)
        self.assertIn("destination", conf.substitutions)
        self.assertIn("parent1:19999", conf.substitutions["destination"])
        self.assertIn("parent2:19999", conf.substitutions["destination"])

    def test_child_stream_conf_api_key(self):
        """Test ChildStreamConf API key substitution"""
        conf = ChildStreamConf(self.mock_installer, self.destinations, self.api_key)
        self.assertEqual(conf.substitutions["api_key"], self.api_key)

    def test_child_stream_conf_timeout_seconds(self):
        """Test ChildStreamConf timeout_seconds substitution"""
        conf = ChildStreamConf(self.mock_installer, self.destinations, self.api_key)
        self.assertEqual(conf.substitutions["timeout_seconds"], 60)

    def test_child_stream_conf_default_port(self):
        """Test ChildStreamConf default_port substitution"""
        conf = ChildStreamConf(self.mock_installer, self.destinations, self.api_key)
        self.assertEqual(conf.substitutions["default_port"], 19999)

    def test_child_stream_conf_send_charts_matching(self):
        """Test ChildStreamConf send_charts_matching substitution"""
        conf = ChildStreamConf(self.mock_installer, self.destinations, self.api_key)
        self.assertEqual(conf.substitutions["send_charts_matching"], "*")

    def test_child_stream_conf_buffer_size_bytes(self):
        """Test ChildStreamConf buffer_size_bytes substitution"""
        conf = ChildStreamConf(self.mock_installer, self.destinations, self.api_key)
        self.assertEqual(conf.substitutions["buffer_size_bytes"], 1024 * 1024)

    def test_child_stream_conf_reconnect_delay_seconds(self):
        """Test ChildStreamConf reconnect_delay_seconds substitution"""
        conf = ChildStreamConf(self.mock_installer, self.destinations, self.api_key)
        self.assertEqual(conf.substitutions["reconnect_delay_seconds"], 5)

    def test_child_stream_conf_initial_clock_resync_iterations(self):
        """Test ChildStreamConf initial_clock_resync_iterations substitution"""
        conf = ChildStreamConf(self.mock_installer, self.destinations, self.api_key)
        self.assertEqual(conf.substitutions["initial_clock_resync_iterations"], 60)

    @patch('builtins.open', new_callable=mock_open, read_data="[child]\nenabled = {{ enabled }}")
    def test_child_stream_conf_render(self, mock_file):
        """Test ChildStreamConf render method"""
        conf = ChildStreamConf(self.mock_installer, self.destinations, self.api_key)
        result = conf.render()
        self.assertIsInstance(result, str)
        self.assertIn("enabled", result)

    @patch('builtins.open', new_callable=mock_open, read_data="")
    def test_child_stream_conf_render_calls_open(self, mock_file):
        """Test ChildStreamCon
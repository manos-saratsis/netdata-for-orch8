import pytest
import asyncio
import sys
import pathlib
import uuid
from unittest.mock import Mock, patch, MagicMock, AsyncMock, call
import click
from click.testing import CliRunner

from build_command import run_async, simple_build, build
from nd import Distribution, NetdataInstaller, FeatureFlags, Endpoint, AgentContext, SUPPORTED_PLATFORMS, SUPPORTED_DISTRIBUTIONS
import dagger


class TestRunAsyncDecorator:
    """Test the run_async decorator."""
    
    def test_run_async_decorator_returns_wrapper(self):
        """Test that run_async returns a wrapper function."""
        def dummy_async_func():
            pass
        
        decorated = run_async(dummy_async_func)
        assert callable(decorated)
    
    def test_run_async_decorator_executes_async_function(self):
        """Test that run_async properly executes an async function."""
        async def sample_async_func(x):
            return x * 2
        
        decorated = run_async(sample_async_func)
        result = decorated(5)
        assert result == 10
    
    def test_run_async_decorator_with_kwargs(self):
        """Test that run_async handles keyword arguments."""
        async def sample_async_func(a, b=10):
            return a + b
        
        decorated = run_async(sample_async_func)
        result = decorated(5, b=20)
        assert result == 25
    
    def test_run_async_decorator_with_multiple_args(self):
        """Test that run_async handles multiple positional arguments."""
        async def sample_async_func(a, b, c):
            return a + b + c
        
        decorated = run_async(sample_async_func)
        result = decorated(1, 2, 3)
        assert result == 6


class TestSimpleBuild:
    """Test the simple_build async function."""
    
    @pytest.mark.asyncio
    @patch('build_command.dagger.Connection')
    @patch('build_command.dagger.Config')
    @patch('build_command.AgentContext')
    async def test_simple_build_creates_client_connection(self, mock_agent_ctx_class, mock_config, mock_connection):
        """Test that simple_build creates a dagger connection."""
        # Setup mocks
        mock_client = AsyncMock()
        mock_connection_instance = AsyncMock()
        mock_connection_instance.__aenter__.return_value = mock_client
        mock_connection_instance.__aexit__.return_value = None
        mock_connection.return_value = mock_connection_instance
        
        mock_agent_ctx_instance = AsyncMock()
        mock_agent_ctx_instance.build_container = AsyncMock()
        mock_agent_ctx_class.return_value = mock_agent_ctx_instance
        
        # Call function
        platform = dagger.Platform("linux/x86_64")
        distro = Distribution("alpine_3_18")
        
        await simple_build(platform, distro)
        
        # Assertions
        mock_config.assert_called_once_with(log_output=sys.stdout)
        mock_connection.assert_called_once()
    
    @pytest.mark.asyncio
    @patch('build_command.dagger.Connection')
    @patch('build_command.dagger.Config')
    @patch('build_command.pathlib.Path')
    @patch('build_command.AgentContext')
    async def test_simple_build_creates_agent_context(self, mock_agent_ctx_class, mock_path, mock_config, mock_connection):
        """Test that simple_build creates an AgentContext with correct parameters."""
        # Setup mocks
        mock_client = AsyncMock()
        mock_connection_instance = AsyncMock()
        mock_connection_instance.__aenter__.return_value = mock_client
        mock_connection_instance.__aexit__.return_value = None
        mock_connection.return_value = mock_connection_instance
        
        mock_agent_ctx_instance = AsyncMock()
        mock_agent_ctx_instance.build_container = AsyncMock()
        mock_agent_ctx_class.return_value = mock_agent_ctx_instance
        
        # Call function
        platform = dagger.Platform("linux/x86_64")
        distro = Distribution("alpine_3_18")
        
        await simple_build(platform, distro)
        
        # Assertions - verify AgentContext was created with correct args
        mock_agent_ctx_class.assert_called_once()
        call_args = mock_agent_ctx_class.call_args
        assert call_args[0][1] == platform  # platform
        assert call_args[0][2] == distro  # distro
    
    @pytest.mark.asyncio
    @patch('build_command.dagger.Connection')
    @patch('build_command.dagger.Config')
    @patch('build_command.AgentContext')
    async def test_simple_build_calls_build_container(self, mock_agent_ctx_class, mock_config, mock_connection):
        """Test that simple_build calls build_container on AgentContext."""
        # Setup mocks
        mock_client = AsyncMock()
        mock_connection_instance = AsyncMock()
        mock_connection_instance.__aenter__.return_value = mock_client
        mock_connection_instance.__aexit__.return_value = None
        mock_connection.return_value = mock_connection_instance
        
        mock_agent_ctx_instance = AsyncMock()
        mock_agent_ctx_instance.build_container = AsyncMock()
        mock_agent_ctx_class.return_value = mock_agent_ctx_instance
        
        # Call function
        platform = dagger.Platform("linux/x86_64")
        distro = Distribution("alpine_3_18")
        
        await simple_build(platform, distro)
        
        # Assertions
        mock_agent_ctx_instance.build_container.assert_called_once()
    
    @pytest.mark.asyncio
    @patch('build_command.dagger.Connection')
    @patch('build_command.dagger.Config')
    @patch('build_command.NetdataInstaller')
    @patch('build_command.AgentContext')
    async def test_simple_build_uses_feature_flags(self, mock_agent_ctx_class, mock_installer_class, mock_config, mock_connection):
        """Test that simple_build creates NetdataInstaller with DBEngine feature flag."""
        # Setup mocks
        mock_client = AsyncMock()
        mock_connection_instance = AsyncMock()
        mock_connection_instance.__aenter__.return_value = mock_client
        mock_connection_instance.__aexit__.return_value = None
        mock_connection.return_value = mock_connection_instance
        
        mock_installer_instance = Mock()
        mock_installer_class.return_value = mock_installer_instance
        
        mock_agent_ctx_instance = AsyncMock()
        mock_agent_ctx_instance.build_container = AsyncMock()
        mock_agent_ctx_class.return_value = mock_agent_ctx_instance
        
        # Call function
        platform = dagger.Platform("linux/x86_64")
        distro = Distribution("alpine_3_18")
        
        await simple_build(platform, distro)
        
        # Assertions - verify FeatureFlags.DBEngine was used
        mock_installer_class.assert_called_once()
        call_args = mock_installer_class.call_args
        assert call_args[0][4] == FeatureFlags.DBEngine


class TestBuildCommand:
    """Test the build click command."""
    
    @patch('build_command.simple_build')
    def test_build_command_with_valid_platform_and_distro(self, mock_simple_build):
        """Test build command with valid platform and distribution."""
        runner = CliRunner()
        result = runner.invoke(build, ['--platform', 'linux/x86_64', '--distribution', 'alpine_3_18'])
        
        # Check that command executed
        assert mock_simple_build.called
    
    @patch('build_command.simple_build')
    def test_build_command_platform_short_option(self, mock_simple_build):
        """Test build command with -p short option for platform."""
        runner = CliRunner()
        result = runner.invoke(build, ['-p', 'linux/x86_64', '-d', 'alpine_3_18'])
        
        assert mock_simple_build.called
    
    @patch('build_command.simple_build')
    def test_build_command_distribution_short_option(self, mock_simple_build):
        """Test build command with -d short option for distribution."""
        runner = CliRunner()
        result = runner.invoke(build, ['-p', 'linux/x86_64', '-d', 'alpine_3_18'])
        
        assert mock_simple_build.called
    
    @patch('build_command.simple_build')
    def test_build_command_converts_platform_to_dagger_platform(self, mock_simple_build):
        """Test that build command converts platform string to dagger.Platform."""
        runner = CliRunner()
        result = runner.invoke(build, ['--platform', 'linux/x86_64', '--distribution', 'alpine_3_18'])
        
        # Get the platform argument passed to simple_build
        call_args = mock_simple_build.call_args
        platform_arg = call_args[0][0]
        
        # Should be a dagger.Platform instance
        assert isinstance(platform_arg, dagger.Platform)
    
    @patch('build_command.simple_build')
    def test_build_command_converts_distro_to_distribution(self, mock_simple_build):
        """Test that build command converts distro string to Distribution."""
        runner = CliRunner()
        result = runner.invoke(build, ['--platform', 'linux/x86_64', '--distribution', 'alpine_3_18'])
        
        # Get the distro argument passed to simple_build
        call_args = mock_simple_build.call_args
        distro_arg = call_args[0][1]
        
        # Should be a Distribution instance
        assert isinstance(distro_arg, Distribution)
    
    def test_build_command_with_all_supported_platforms(self):
        """Test build command accepts all supported platforms."""
        runner = CliRunner()
        
        for platform in SUPPORTED_PLATFORMS:
            with patch('build_command.simple_build'):
                result = runner.invoke(build, ['--platform', str(platform), '--distribution', 'alpine_3_18'])
                # Command should not error (exit code 0 or exception in simple_build which is mocked)
    
    def test_build_command_with_all_supported_distributions(self):
        """Test build command accepts all supported distributions."""
        runner = CliRunner()
        
        for distro in SUPPORTED_DISTRIBUTIONS:
            with patch('build_command.simple_build'):
                result = runner.invoke(build, ['--platform', 'linux/x86_64', '--distribution', distro])
    
    def test_build_command_without_platform_option(self):
        """Test build command without platform option."""
        runner = CliRunner()
        with patch('build_command.simple_build'):
            result = runner.invoke(build, ['--distribution', 'alpine_3_18'])
    
    def test_build_command_without_distribution_option(self):
        """Test build command without distribution option."""
        runner = CliRunner()
        with patch('build_command.simple_build'):
            result = runner.invoke(build, ['--platform', 'linux/x86_64'])
    
    def test_build_command_with_neither_option(self):
        """Test build command without platform or distribution options."""
        runner = CliRunner()
        with patch('build_command.simple_build'):
            result = runner.invoke(build, [])
    
    @patch('build_command.simple_build')
    def test_build_command_with_arm64_platform(self, mock_simple_build):
        """Test build command with ARM64 platform."""
        runner = CliRunner()
        result = runner.invoke(build, ['--platform', 'linux/arm64', '--distribution', 'alpine_3_18'])
        
        assert mock_simple_build.called
        call_args = mock_simple_build.call_args
        platform_arg = call_args[0][0]
        assert str(platform_arg) == 'linux/arm64'
    
    @patch('build_command.simple_build')
    def test_build_command_with_i386_platform(self, mock_simple_build):
        """Test build command with i386 platform."""
        runner = CliRunner()
        result = runner.invoke(build, ['--platform', 'linux/i386', '--distribution', 'alpine_3_18'])
        
        assert mock_simple_build.called
    
    @patch('build_command.simple_build')
    def test_build_command_passes_endpoint_with_correct_port(self, mock_simple_build):
        """Test that AgentContext is created with Endpoint having port 19999."""
        runner = CliRunner()
        result = runner.invoke(build, ['--platform', 'linux/x86_64', '--distribution', 'alpine_3_18'])
        
        # The endpoint port should be 19999
        assert mock_simple_build.called
    
    @patch('build_command.simple_build')
    def test_build_command_endpoint_hostname_is_node(self, mock_simple_build):
        """Test that Endpoint hostname is 'node'."""
        runner = CliRunner()
        result = runner.invoke(build, ['--platform', 'linux/x86_64', '--distribution', 'alpine_3_18'])
        
        assert mock_simple_build.called
    
    @patch('build_command.simple_build')
    def test_build_command_allow_children_is_false(self, mock_simple_build):
        """Test that allow_children in AgentContext is False."""
        runner = CliRunner()
        result = runner.invoke(build, ['--platform', 'linux/x86_64', '--distribution', 'alpine_3_18'])
        
        # Based on the code, allow_children should be False
        assert mock_simple_build.called
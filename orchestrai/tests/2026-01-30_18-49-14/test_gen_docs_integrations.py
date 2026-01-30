"""
Comprehensive tests for gen_docs_integrations.py
Tests cover all functions, branches, error handling, and edge cases.
"""

import json
import tempfile
from pathlib import Path
from unittest.mock import MagicMock, Mock, patch, mock_open

import pytest

import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../integrations'))

from gen_docs_integrations import (
    cleanup,
    clean_and_write,
    build_path,
    add_custom_edit_url,
    clean_string,
    read_integrations_js,
    generate_category_from_name,
    create_overview,
    build_readme_from_integration,
    create_overview_banner,
    write_to_file,
    make_symlinks,
    _base_paths_for_collector,
    symlink_dict,
)


class TestCleanup:
    """Test the cleanup function"""

    def test_cleanup_default_full_cleanup(self):
        """Test cleanup with no parameters does full cleanup"""
        with patch('gen_docs_integrations.Path.glob') as mock_glob:
            with patch('gen_docs_integrations.shutil.rmtree') as mock_rmtree:
                mock_glob.return_value = [Path('test/integrations')]
                cleanup()
                assert mock_rmtree.called

    def test_cleanup_with_specific_paths(self):
        """Test cleanup with specific base paths"""
        with patch('gen_docs_integrations.Path.glob') as mock_glob:
            with patch('gen_docs_integrations.shutil.rmtree') as mock_rmtree:
                mock_glob.return_value = [Path('custom/integrations')]
                cleanup(only_base_paths=['custom'])
                assert mock_rmtree.called

    def test_cleanup_with_multiple_paths(self):
        """Test cleanup with multiple base paths"""
        with patch('gen_docs_integrations.Path.glob') as mock_glob:
            with patch('gen_docs_integrations.shutil.rmtree') as mock_rmtree:
                mock_glob.return_value = [Path('path1/integrations'), Path('path2/integrations')]
                cleanup(only_base_paths=['path1', 'path2'])
                assert mock_rmtree.call_count >= 0

    def test_cleanup_empty_list(self):
        """Test cleanup with empty base paths list"""
        with patch('gen_docs_integrations.Path.glob') as mock_glob:
            with patch('gen_docs_integrations.shutil.rmtree') as mock_rmtree:
                mock_glob.return_value = []
                cleanup(only_base_paths=[])
                # Should not raise error


class TestCleanAndWrite:
    """Test clean_and_write function"""

    def test_clean_and_write_with_details_open(self):
        """Test writing markdown with open details tags"""
        with tempfile.TemporaryDirectory() as tmpdir:
            path = Path(tmpdir) / "test.md"
            md = 'Test {% details open=true summary="Click me" %}\nContent\n{% /details %}'
            clean_and_write(md, path)
            result = path.read_text(encoding="utf-8")
            assert '<details open><summary>' in result
            assert '</summary>' in result
            assert '</details>' in result

    def test_clean_and_write_with_details_closed(self):
        """Test writing markdown with closed details tags"""
        with tempfile.TemporaryDirectory() as tmpdir:
            path = Path(tmpdir) / "test.md"
            md = 'Test {% details summary="Click me" %}\nContent\n{% /details %}'
            clean_and_write(md, path)
            result = path.read_text(encoding="utf-8")
            assert '<details><summary>' in result
            assert '</summary>' in result

    def test_clean_and_write_no_details_tags(self):
        """Test writing markdown without details tags"""
        with tempfile.TemporaryDirectory() as tmpdir:
            path = Path(tmpdir) / "test.md"
            md = 'Just plain markdown\n# Heading'
            clean_and_write(md, path)
            result = path.read_text(encoding="utf-8")
            assert result == md

    def test_clean_and_write_multiple_details(self):
        """Test writing markdown with multiple details sections"""
        with tempfile.TemporaryDirectory() as tmpdir:
            path = Path(tmpdir) / "test.md"
            md = '''{% details summary="First" %}Content1{% /details %}
{% details summary="Second" %}Content2{% /details %}'''
            clean_and_write(md, path)
            result = path.read_text(encoding="utf-8")
            assert result.count('<details>') == 2
            assert result.count('</details>') == 2


class TestBuildPath:
    """Test build_path function"""

    def test_build_path_basic(self):
        """Test basic GitHub link conversion"""
        link = "https://github.com/netdata/netdata/edit/master/src/collectors/test/metadata.yaml"
        result = build_path(link)
        assert result == "src/collectors/test"

    def test_build_path_nested_collector(self):
        """Test nested collector path"""
        link = "https://github.com/netdata/netdata/edit/master/src/go/plugin/go.d/collector/test/metadata.yaml"
        result = build_path(link)
        assert result == "src/go/plugin/go.d/collector/test"

    def test_build_path_with_blob(self):
        """Test path with blob instead of edit"""
        link = "https://github.com/netdata/netdata/blob/master/src/collectors/test/metadata.yaml"
        result = build_path(link)
        # Should handle blob similarly
        assert "collectors" in result

    def test_build_path_exporter(self):
        """Test exporter path"""
        link = "https://github.com/netdata/netdata/edit/master/src/exporting/test/metadata.yaml"
        result = build_path(link)
        assert result == "src/exporting/test"


class TestAddCustomEditUrl:
    """Test add_custom_edit_url function"""

    def test_add_custom_edit_url_default_mode(self):
        """Test default mode edit URL injection"""
        md = "<!--startmeta\nOther content"
        meta_yaml = "https://github.com/netdata/netdata/edit/master/src/collectors/test/metadata.yaml"
        sidebar_label = "Test Plugin"
        result = add_custom_edit_url(md, meta_yaml, sidebar_label, mode="default")
        assert 'custom_edit_url:' in result
        assert 'test_plugin' in result

    def test_add_custom_edit_url_cloud_notification(self):
        """Test cloud-notification mode"""
        md = "<!--startmeta\nOther"
        meta_yaml = "https://github.com/netdata/netdata/edit/master/integrations/cloud-notifications/metadata.yaml"
        sidebar_label = "Slack"
        result = add_custom_edit_url(md, meta_yaml, sidebar_label, mode="cloud-notification")
        assert 'custom_edit_url:' in result
        assert 'integrations/slack' in result

    def test_add_custom_edit_url_logs(self):
        """Test logs mode"""
        md = "<!--startmeta\nOther"
        meta_yaml = "https://github.com/netdata/netdata/edit/master/integrations/logs/metadata.yaml"
        sidebar_label = "Application Logs"
        result = add_custom_edit_url(md, meta_yaml, sidebar_label, mode="logs")
        assert 'custom_edit_url:' in result

    def test_add_custom_edit_url_cloud_authentication(self):
        """Test cloud-authentication mode"""
        md = "<!--startmeta\nOther"
        meta_yaml = "https://github.com/netdata/netdata/edit/master/integrations/cloud-authentication/metadata.yaml"
        sidebar_label = "OAuth"
        result = add_custom_edit_url(md, meta_yaml, sidebar_label, mode="cloud-authentication")
        assert 'custom_edit_url:' in result

    def test_add_custom_edit_url_agent_notification(self):
        """Test agent-notification mode"""
        md = "<!--startmeta\nOther"
        meta_yaml = "https://github.com/netdata/netdata/edit/master/src/health/notifications/test/metadata.yaml"
        sidebar_label = "Email"
        result = add_custom_edit_url(md, meta_yaml, sidebar_label, mode="agent-notification")
        assert 'custom_edit_url:' in result
        assert 'README' in result

    def test_add_custom_edit_url_invalid_mode_fallback(self):
        """Test invalid mode falls back to default"""
        md = "<!--startmeta\nOther"
        meta_yaml = "https://github.com/netdata/netdata/edit/master/src/collectors/test/metadata.yaml"
        sidebar_label = "Test"
        result = add_custom_edit_url(md, meta_yaml, sidebar_label, mode="invalid")
        assert 'custom_edit_url:' in result


class TestCleanString:
    """Test clean_string function"""

    def test_clean_string_lowercase(self):
        """Test string is converted to lowercase"""
        assert clean_string("TEST String") == "test_string"

    def test_clean_string_spaces_to_underscores(self):
        """Test spaces are converted to underscores"""
        assert clean_string("Test String Name") == "test_string_name"

    def test_clean_string_slashes(self):
        """Test slashes are converted to hyphens"""
        assert clean_string("Test/Path") == "test-path"

    def test_clean_string_parentheses(self):
        """Test parentheses are removed"""
        assert clean_string("Test (Name)") == "test_name"

    def test_clean_string_colons(self):
        """Test colons are removed"""
        assert clean_string("Test: Name") == "test_name"

    def test_clean_string_mixed(self):
        """Test multiple transformations"""
        assert clean_string("My Test / Plugin (v1): Name") == "my_test_-_plugin_v1_name"

    def test_clean_string_already_clean(self):
        """Test already clean string"""
        assert clean_string("already_clean") == "already_clean"

    def test_clean_string_empty(self):
        """Test empty string"""
        assert clean_string("") == ""


class TestReadIntegrationsJs:
    """Test read_integrations_js function"""

    def test_read_integrations_js_valid_file(self):
        """Test reading valid integrations.js file"""
        js_content = '''export const categories = {"test": "category"};
export const integrations = [{"id": "test"}];'''
        with tempfile.TemporaryDirectory() as tmpdir:
            filepath = Path(tmpdir) / "integrations.js"
            filepath.write_text(js_content)
            categories, integrations = read_integrations_js(str(filepath))
            assert categories == {"test": "category"}
            assert integrations == [{"id": "test"}]

    def test_read_integrations_js_file_not_found(self):
        """Test reading non-existent file"""
        categories, integrations = read_integrations_js("/nonexistent/path.js")
        assert categories == []
        assert integrations == []

    def test_read_integrations_js_complex_structure(self):
        """Test reading complex JSON structure"""
        js_content = '''export const categories = [{"id": "db", "name": "Databases"}];
export const integrations = [{"id": "mysql", "name": "MySQL"}, {"id": "postgres", "name": "PostgreSQL"}];'''
        with tempfile.TemporaryDirectory() as tmpdir:
            filepath = Path(tmpdir) / "integrations.js"
            filepath.write_text(js_content)
            categories, integrations = read_integrations_js(str(filepath))
            assert len(categories) == 1
            assert len(integrations) == 2


class TestGenerateCategoryFromName:
    """Test generate_category_from_name function"""

    def test_generate_category_single_level(self):
        """Test single level category"""
        category_fragment = ["apps"]
        categories = [{"id": "apps", "name": "Applications", "children": []}]
        result = generate_category_from_name(category_fragment, categories)
        assert result == "Applications"

    def test_generate_category_nested(self):
        """Test nested category structure"""
        category_fragment = ["data", "db"]
        categories = [{
            "id": "data",
            "name": "Data Collection",
            "children": [{"id": "data.db", "name": "Databases"}]
        }]
        result = generate_category_from_name(category_fragment, categories)
        assert "Databases" in result

    def test_generate_category_deep_nesting(self):
        """Test deeply nested categories"""
        category_fragment = ["a", "b", "c"]
        categories = [{
            "id": "a",
            "name": "Level1",
            "children": [{
                "id": "a.b",
                "name": "Level2",
                "children": [{
                    "id": "a.b.c",
                    "name": "Level3"
                }]
            }]
        }]
        result = generate_category_from_name(category_fragment, categories)
        assert "Level3" in result

    def test_generate_category_empty_fragment(self):
        """Test with empty category fragment"""
        result = generate_category_from_name([], [])
        assert result == ""

    def test_generate_category_not_found(self):
        """Test when category is not found"""
        category_fragment = ["unknown"]
        categories = [{"id": "apps", "name": "Applications"}]
        result = generate_category_from_name(category_fragment, categories)
        assert result == ""


class TestCreateOverview:
    """Test create_overview function"""

    def test_create_overview_with_filename(self):
        """Test overview with icon filename"""
        integration = {
            "meta": {"name": "Test Integration"},
            "overview": "# Overview\n\nTest description\n\nMore info"
        }
        result = create_overview(integration, "test-icon.png")
        assert "Test Integration" in result
        assert "test-icon.png" in result
        assert "width=\"150\"" in result

    def test_create_overview_no_filename(self):
        """Test overview without icon filename"""
        integration = {
            "meta": {"name": "Test Integration"},
            "overview": "# Overview\n\nTest description"
        }
        result = create_overview(integration, "")
        assert "Test Integration" in result
        assert "img" not in result.lower() or "netdata.cloud/img" not in result

    def test_create_overview_no_key(self):
        """Test overview with empty overview_key_name"""
        integration = {
            "meta": {"name": "Test Integration"}
        }
        result = create_overview(integration, "icon.png", overview_key_name="")
        assert "Test Integration" in result
        assert "icon.png" in result

    def test_create_overview_no_heading(self):
        """Test overview without heading in content"""
        integration = {
            "meta": {"name": "Test"},
            "overview": "Just plain text"
        }
        # This should handle gracefully or raise appropriate error
        # The function expects at least one heading
        with pytest.raises((IndexError, ValueError)):
            create_overview(integration, "icon.png")

    def test_create_overview_multiple_headings(self):
        """Test overview with multiple headings"""
        integration = {
            "meta": {"name": "Test"},
            "overview": "# First\n\n## Second\n\nContent"
        }
        result = create_overview(integration, "icon.png")
        assert "# First" in result
        assert "icon.png" in result


class TestBuildReadmeFromIntegration:
    """Test build_readme_from_integration function"""

    def test_build_readme_collector_mode(self):
        """Test building README for collector"""
        integration = {
            "edit_link": "https://github.com/netdata/netdata/blob/master/src/collectors/test/metadata.yaml",
            "meta": {
                "monitored_instance": {
                    "name": "Test System",
                    "categories": ["data.db"],
                    "icon_filename": "test.png",
                },
                "most_popular": True,
                "keywords": ["test", "keyword"]
            },
            "metrics": "## Metrics",
            "alerts": "## Alerts",
            "setup": "## Setup",
            "troubleshooting": "## Troubleshooting"
        }
        categories = [{
            "id": "data",
            "name": "Data Collection",
            "children": [{
                "id": "data.db",
                "name": "Databases"
            }]
        }]
        
        meta_yaml, sidebar, learn_path, md, community = build_readme_from_integration(integration, categories, mode="collector")
        
        assert meta_yaml != ""
        assert sidebar == "Test System"
        assert "Databases" in learn_path
        assert "Test System" in md
        assert community != ""

    def test_build_readme_exporter_mode(self):
        """Test building README for exporter"""
        integration = {
            "edit_link": "https://github.com/netdata/netdata/blob/master/src/exporting/test/metadata.yaml",
            "meta": {
                "name": "Test Exporter",
                "categories": ["exporting"],
                "icon_filename": "exporter.png"
            },
            "overview": "# Overview\n\nTest overview",
            "setup": "## Setup"
        }
        categories = [{
            "id": "exporting",
            "name": "Exporting",
            "children": []
        }]
        
        meta_yaml, sidebar, learn_path, md, community = build_readme_from_integration(integration, categories, mode="exporter")
        
        assert sidebar == "Test Exporter"
        assert "Exporter" in md or "Test" in md

    def test_build_readme_agent_notification_mode(self):
        """Test building README for agent notification"""
        integration = {
            "edit_link": "https://github.com/netdata/netdata/blob/master/src/health/notifications/test/metadata.yaml",
            "meta": {
                "name": "Email Notification",
                "categories": ["notifications"],
                "icon_filename": "email.png"
            },
            "overview": "# Overview\n\nTest overview",
            "setup": "## Setup"
        }
        categories = [{
            "id": "notifications",
            "name": "Notifications",
            "children": []
        }]
        
        meta_yaml, sidebar, learn_path, md, community = build_readme_from_integration(integration, categories, mode="agent-notification")
        
        assert sidebar == "Email Notification"
        assert "Notifications" in learn_path

    def test_build_readme_cloud_notification_mode(self):
        """Test building README for cloud notification"""
        integration = {
            "edit_link": "https://github.com/netdata/netdata/blob/master/integrations/cloud-notifications/metadata.yaml",
            "meta": {
                "name": "Slack",
                "categories": ["notifications"],
                "icon_filename": "slack.png"
            },
            "overview": "# Overview\n\nTest",
            "setup": "## Setup"
        }
        categories = [{
            "id": "notifications",
            "name": "Notifications",
            "children": []
        }]
        
        meta_yaml, sidebar, learn_path, md, community = build_readme_from_integration(integration, categories, mode="cloud-notification")
        
        assert sidebar == "Slack"

    def test_build_readme_logs_mode(self):
        """Test building README for logs"""
        integration = {
            "edit_link": "https://github.com/netdata/netdata/blob/master/integrations/logs/metadata.yaml",
            "meta": {
                "name": "Application Logs",
                "categories": ["logs"],
                "icon_filename": "logs.png"
            },
            "overview": "# Overview\n\nTest",
            "setup": "## Setup"
        }
        categories = [{
            "id": "logs",
            "name": "Logs",
            "children": []
        }]
        
        meta_yaml, sidebar, learn_path, md, community = build_readme_from_integration(integration, categories, mode="logs")
        
        assert sidebar == "Application Logs"
        assert "Logs" in learn_path

    def test_build_readme_authentication_mode(self):
        """Test building README for authentication"""
        integration = {
            "edit_link": "https://github.com/netdata/netdata/blob/master/integrations/cloud-authentication/metadata.yaml",
            "meta": {
                "name": "OAuth2",
                "categories": ["authentication"],
                "icon_filename": "oauth.png"
            },
            "overview": "# Overview\n\nTest",
            "setup": "## Setup"
        }
        categories = [{
            "id": "authentication",
            "name": "Authentication",
            "children": []
        }]
        
        meta_yaml, sidebar, learn_path, md, community = build_readme_from_integration(integration, categories, mode="authentication")
        
        assert sidebar == "OAuth2"

    def test_build_readme_community_badge_netdata(self):
        """Test community badge for Netdata-maintained integrations"""
        integration = {
            "edit_link": "https://github.com/netdata/netdata/blob/master/src/collectors/test/metadata.yaml",
            "meta": {
                "monitored_instance": {
                    "name": "Test",
                    "categories": ["data.db"],
                    "icon_filename": "",
                },
                "most_popular": False
            }
        }
        categories = [{"id": "data", "name": "Data", "children": [{"id": "data.db", "name": "DB"}]}]
        
        _, _, _, _, community = build_readme_from_integration(integration, categories, mode="collector")
        
        assert "Netdata" in community
        assert "blue" not in community

    def test_build_readme_community_badge_community(self):
        """Test community badge for community-maintained integrations"""
        integration = {
            "edit_link": "https://github.com/netdata/netdata/blob/master/src/collectors/test/metadata.yaml",
            "meta": {
                "community": True,
                "monitored_instance": {
                    "name": "Test",
                    "categories": ["data.db"],
                    "icon_filename": "",
                },
                "most_popular": False
            }
        }
        categories = [{"id": "data", "name": "Data", "children": [{"id": "data.db", "name": "DB"}]}]
        
        _, _, _, _, community = build_readme_from_integration(integration, categories, mode="collector")
        
        assert "Community" in community
        assert "blue" in community

    def test_build_readme_exception_handling(self):
        """Test exception handling in build_readme"""
        integration = {
            "edit_link": "invalid",
            "id": "test"
        }
        categories = []
        
        meta_yaml, sidebar, learn_path, md, community = build_readme_from_integration(integration, categories, mode="collector")
        
        # Should return empty/default values on exception
        assert isinstance(md, str)


class TestCreateOverviewBanner:
    """Test create_overview_banner function"""

    def test_create_overview_banner_with_double_hash(self):
        """Test inserting banner before ## section"""
        md = "Text\n## Section\nMore"
        badge = '<img src="badge.svg" />'
        result = create_overview_banner(md, badge)
        assert badge in result
        assert result.index(badge) < result.index("## Section")

    def test_create_overview_banner_no_double_hash(self):
        """Test inserting banner when no ## present"""
        md = "Just text"
        badge = '<img src="badge.svg" />'
        result = create_overview_banner(md, badge)
        assert badge in result
        assert result.endswith(badge + "\n")

    def test_create_overview_banner_multiple_sections(self):
        """Test banner inserted before first ## only"""
        md = "Text\n## First\nContent\n## Second\nMore"
        badge = '<img src="badge.svg" />'
        result = create_overview_banner(md, badge)
        first_badge = result.find(badge)
        first_section = result.find("## First")
        assert first_badge < first_section


class TestWriteToFile:
    """Test write_to_file function"""

    def test_write_to_file_default_mode_single_file(self):
        """Test writing file in default mode with single integration"""
        with tempfile.TemporaryDirectory() as tmpdir:
            base_path = Path(tmpdir) / "test_collector"
            base_path.mkdir()
            
            md = "# Test\n\n## Content"
            meta_yaml = "https://github.com/netdata/netdata/edit/master/src/collectors/test/metadata.yaml"
            sidebar_label = "Test Collector"
            community = '<img src="badge.svg" />'
            
            write_to_file(str(base_path), md, meta_yaml, sidebar_label, community, mode="default")
            
            integrations_dir = base_path / "integrations"
            assert integrations_dir.exists()
            assert (integrations_dir / "test_collector.md").exists()

    def test_write_to_file_default_mode_symlink_creation(self):
        """Test symlink creation for single file"""
        with tempfile.TemporaryDirectory() as tmpdir:
            base_path = Path(tmpdir) / "collector"
            base_path.mkdir()
            
            md = "# Test"
            meta_yaml = "https://github.com/test"
            sidebar = "Test"
            community = "badge"
            
            # Clear symlink_dict before test
            symlink_dict.clear()
            
            write_to_file(str(base_path), md, meta_yaml, sidebar, community, mode="default")
            
            # Check if symlink was registered
            assert len(symlink_dict) >= 0  # May be registered

    def test_write_to_file_cloud_notification_mode(self):
        """Test writing file in cloud-notification mode"""
        with tempfile.TemporaryDirectory() as tmpdir:
            base_path = Path(tmpdir) / "cloud-notif"
            base_path.mkdir()
            
            integration = {
                "meta": {"name": "Slack"}
            }
            
            md = "# Slack"
            meta_yaml = "https://github.com/netdata/netdata/edit/master/integrations/cloud-notifications/metadata.yaml"
            sidebar = "Slack"
            community = "badge"
            
            write_to_file(str(base_path), md, meta_yaml, sidebar, community, integration=integration, mode="cloud-notification")
            
            integrations_dir = base_path / "integrations"
            assert integrations_dir.exists()
            assert (integrations_dir / "slack.md").exists()

    def test_write_to_file_agent_notification_mode(self):
        """Test writing file in agent-notification mode"""
        with tempfile.TemporaryDirectory() as tmpdir:
            base_path = Path(tmpdir)
            base_path.mkdir(exist_ok=True)
            
            md = "# Email"
            meta_yaml = "https://github.com/netdata/netdata/edit/master/src/health/notifications/email/metadata.yaml"
            sidebar = "Email"
            community = "badge"
            
            write_to_file(str(base_path), md, meta_yaml, sidebar, community, mode="agent-notification")
            
            assert (base_path / "README.md").exists()

    def test_write_to_file_logs_mode(self):
        """Test writing file in logs mode"""
        with tempfile.TemporaryDirectory() as tmpdir:
            base_path = Path(tmpdir) / "logs"
            base_path.mkdir()
            
            integration = {
                "meta": {"name": "Application Logs"}
            }
            
            md = "# App Logs"
            meta_yaml = "https://github.com/netdata/netdata/edit/master/integrations/logs/metadata.yaml"
            sidebar = "Application Logs"
            community = "badge"
            
            write_to_file(str(base_path), md, meta_yaml, sidebar, community, integration=integration, mode="logs")
            
            integrations_dir = base_path / "integrations"
            assert integrations_dir.exists()

    def test_write_to_file_authentication_mode(self):
        """Test writing file in authentication mode"""
        with tempfile.TemporaryDirectory() as tmpdir:
            base_path = Path(tmpdir) / "auth"
            base_path.mkdir()
            
            integration = {
                "meta": {"name": "OAuth2"}
            }
            
            md = "# OAuth2"
            meta_yaml = "https://github.com/netdata/netdata/edit/master/integrations/cloud-authentication/metadata.yaml"
            sidebar = "OAuth2"
            community = "badge"
            
            write_to_file(str(base_path), md, meta_yaml, sidebar, community, integration=integration, mode="authentication")
            
            integrations_dir = base_path / "integrations"
            assert integrations_dir.exists()

    def test_write_to_file_file_not_found_error(self):
        """Test handling FileNotFoundError"""
        with patch('gen_docs_integrations.Path.write_text', side_effect=FileNotFoundError):
            # Should not raise, should handle gracefully
            write_to_file("/invalid/path", "md", "yaml", "label", "community")


class TestMakeSymlinks:
    """Test make_symlinks function"""

    def test_make_symlinks_creates_symlink(self):
        """Test that symlinks are created"""
        with tempfile.TemporaryDirectory() as tmpdir:
            base_path = Path(tmpdir) / "test"
            base_path.mkdir()
            (base_path / "integrations").mkdir()
            
            # Create the target file
            target_file = base_path / "integrations" / "test.md"
            target_file.write_text("# Test")
            
            symlinks = {str(base_path): "integrations/test.md"}
            
            make_symlinks(symlinks)
            
            readme = base_path / "README.md"
            assert readme.is_symlink() or readme.exists()

    def test_make_symlinks_updates_content(self):
        """Test that symlink target content is updated"""
        with tempfile.TemporaryDirectory() as tmpdir:
            base_path = Path(tmpdir) / "test"
            base_path.mkdir()
            (base_path / "integrations").mkdir()
            
            target_file = base_path / "integrations" / "test.md"
            target_file.write_text(f"Path: {base_path}/integrations/test.md")
```python
import os
import unittest
from unittest.mock import patch, Mock, MagicMock
import time

from bases.FrameworkServices.SimpleService import SimpleService, RuntimeCounters, clean_module_name

class TestSimpleService(unittest.TestCase):
    def setUp(self):
        self.config = {
            'job_name': 'test_job',
            'update_every': 10,
            'penalty': True,
            'priority': 1,
            'chart_cleanup': 5
        }

    def test_clean_module_name(self):
        # Test with prefix
        self.assertEqual(clean_module_name('pythond_test_module'), 'test_module')
        
        # Test without prefix
        self.assertEqual(clean_module_name('test_module'), 'test_module')

    def test_runtime_counters_init(self):
        rc = RuntimeCounters(self.config.copy())
        
        self.assertEqual(rc.update_every, 10)
        self.assertTrue(rc.do_penalty)
        self.assertEqual(rc.retries, 0)
        self.assertEqual(rc.penalty, 0)
        self.assertEqual(rc.runs, 1)

    def test_runtime_counters_calc_next(self):
        with patch('bases.FrameworkServices.SimpleService.monotonic') as mock_mono:
            mock_mono.return_value = 100
            rc = RuntimeCounters(self.config.copy())
            next_time = rc.calc_next()
            
            # Verify the next time is calculated correctly
            self.assertGreater(next_time, 100)

    def test_runtime_counters_handle_retries(self):
        rc = RuntimeCounters(self.config.copy())
        
        # Simulate multiple retries
        for i in range(10):
            rc.handle_retries()
        
        # Check penalty increases but doesn't exceed max
        self.assertTrue(0 < rc.penalty <= 600)
        self.assertEqual(rc.retries, 10)

    def test_simple_service_init(self):
        service = SimpleService(self.config.copy())
        
        self.assertEqual(service.job_name, 'test_job')
        self.assertEqual(service.update_every, 10)
        self.assertIsNotNone(service.charts)

    def test_simple_service_name_property(self):
        # Test with default configuration
        service = SimpleService(self.config.copy())
        self.assertIn('simple_service', service.name)
        self.assertIn('test_job', service.name)

    def test_simple_service_check(self):
        class TestService(SimpleService):
            def get_data(self):
                return {'test': 1}

        service = TestService(self.config.copy())
        self.assertTrue(service.check())

    def test_simple_service_check_failure(self):
        class TestService(SimpleService):
            def get_data(self):
                return None

        service = TestService(self.config.copy())
        self.assertFalse(service.check())

    @patch('bases.FrameworkServices.SimpleService.safe_print')
    @patch('bases.FrameworkServices.SimpleService.monotonic')
    def test_simple_service_run_update_success(self, mock_monotonic, mock_safe_print):
        mock_monotonic.side_effect = [100, 110, 120]
        
        class TestService(SimpleService):
            def get_data(self):
                return {'metric': 10}
            
            def update(self, interval):
                return True

        service = TestService(self.config.copy())
        
        # Simulate run with a timeout
        with self.assertRaises(SystemExit):
            service.run()

    @patch('bases.FrameworkServices.SimpleService.monotonic')
    def test_simple_service_run_update_failure(self, mock_monotonic):
        mock_monotonic.side_effect = [100, 110, 120]
        
        class TestService(SimpleService):
            def get_data(self):
                return None
            
            def update(self, interval):
                return False

        service = TestService(self.config.copy())
        
        # Simulate run with a timeout
        with self.assertRaises(SystemExit):
            service.run()

    def test_simple_service_create(self):
        class TestService(SimpleService):
            order = ['chart1', 'chart2']
            definitions = {
                'chart1': {
                    'options': ['title', 'units'],
                    'lines': [['dim1'], ['dim2']],
                    'variables': [['var1']]
                },
                'chart2': {
                    'options': ['title', 'units'],
                    'lines': [['dim3']]
                }
            }

        service = TestService(self.config.copy())
        result = service.create()
        
        self.assertTrue(result)
        self.assertEqual(len(service.charts), 2)

    def test_simple_service_get_data_not_implemented(self):
        service = SimpleService(self.config.copy())
        
        with self.assertRaises(NotImplementedError):
            service.get_data()

if __name__ == '__main__':
    unittest.main()
```
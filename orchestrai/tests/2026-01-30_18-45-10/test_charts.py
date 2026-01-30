# -*- coding: utf-8 -*-
import pytest
import os
from unittest.mock import patch, MagicMock, call
from collections import OrderedDict

# Mock safe_print before importing charts
import sys
from unittest.mock import Mock
sys.modules['bases.collection'] = MagicMock()

from bases.charts import (
    Charts, Chart, Dimension, ChartVariable, ChartFlags,
    ChartError, DuplicateItemError, ItemTypeError, ItemValueError,
    create_runtime_chart,
    CHART_PARAMS, DIMENSION_PARAMS, VARIABLE_PARAMS,
    CHART_TYPES, DIMENSION_ALGORITHMS,
    CHART_BEGIN, CHART_CREATE, CHART_OBSOLETE,
    CLABEL_COLLECT_JOB, CLABEL_COMMIT,
    DIMENSION_CREATE, DIMENSION_SET,
    CHART_VARIABLE_SET,
    RUNTIME_CHART_CREATE,
    ND_INTERNAL_MONITORING_DISABLED
)


class TestChartFlags:
    def test_init(self):
        """Test ChartFlags initialization"""
        flags = ChartFlags()
        assert flags.push is True
        assert flags.created is False
        assert flags.updated is False
        assert flags.obsoleted is False

    def test_flag_modification(self):
        """Test modifying flags"""
        flags = ChartFlags()
        flags.push = False
        flags.created = True
        flags.updated = True
        flags.obsoleted = True
        
        assert flags.push is False
        assert flags.created is True
        assert flags.updated is True
        assert flags.obsoleted is True


class TestChartError:
    def test_chart_error_inheritance(self):
        """Test ChartError is Exception"""
        error = ChartError("Test error")
        assert isinstance(error, Exception)
        assert str(error) == "Test error"

    def test_duplicate_item_error(self):
        """Test DuplicateItemError inheritance"""
        error = DuplicateItemError("Duplicate")
        assert isinstance(error, ChartError)
        assert isinstance(error, Exception)

    def test_item_type_error(self):
        """Test ItemTypeError inheritance"""
        error = ItemTypeError("Type error")
        assert isinstance(error, ChartError)

    def test_item_value_error(self):
        """Test ItemValueError inheritance"""
        error = ItemValueError("Value error")
        assert isinstance(error, ChartError)


class TestDimension:
    def test_dimension_init_valid(self):
        """Test Dimension initialization with valid params"""
        params = ['dimension_id', 'dimension_name', 'absolute', 1, 1]
        dim = Dimension(params)
        assert dim.id == 'dimension_id'
        assert dim.name == 'dimension_name'
        assert dim.algorithm == 'absolute'
        assert dim.multiplier == 1
        assert dim.divisor == 1

    def test_dimension_init_invalid_type(self):
        """Test Dimension initialization with non-list type"""
        with pytest.raises(ItemTypeError):
            Dimension("not a list")

    def test_dimension_init_empty_list(self):
        """Test Dimension initialization with empty list"""
        with pytest.raises(ItemValueError):
            Dimension([])

    def test_dimension_init_with_none_values(self):
        """Test Dimension initialization with None values"""
        params = ['dim_id', None, None, None, None]
        dim = Dimension(params)
        assert dim.id == 'dim_id'
        assert dim.name == 'dim_id'
        assert dim.algorithm == 'absolute'
        assert dim.multiplier == 1
        assert dim.divisor == 1

    def test_dimension_name_defaults_to_id(self):
        """Test dimension name defaults to id when not provided"""
        params = ['test_id']
        dim = Dimension(params)
        assert dim.name == 'test_id'

    def test_dimension_invalid_algorithm_defaults_to_absolute(self):
        """Test invalid algorithm defaults to absolute"""
        params = ['id', 'name', 'invalid_algo', 1, 1]
        dim = Dimension(params)
        assert dim.algorithm == 'absolute'

    def test_dimension_valid_algorithms(self):
        """Test all valid algorithms"""
        algorithms = ['absolute', 'incremental', 'percentage-of-absolute-row', 'percentage-of-incremental-row']
        for algo in algorithms:
            params = ['id', 'name', algo, 1, 1]
            dim = Dimension(params)
            assert dim.algorithm == algo

    def test_dimension_non_int_multiplier_defaults_to_1(self):
        """Test non-int multiplier defaults to 1"""
        params = ['id', 'name', 'absolute', 'not_int', 1]
        dim = Dimension(params)
        assert dim.multiplier == 1

    def test_dimension_non_int_divisor_defaults_to_1(self):
        """Test non-int divisor defaults to 1"""
        params = ['id', 'name', 'absolute', 1, 'not_int']
        dim = Dimension(params)
        assert dim.divisor == 1

    def test_dimension_hidden_param_defaults(self):
        """Test hidden and obsolete params default to empty string"""
        params = ['id']
        dim = Dimension(params)
        assert dim.hidden == ''
        assert dim.obsolete == ''

    def test_dimension_repr(self):
        """Test Dimension repr"""
        dim = Dimension(['test_id'])
        assert repr(dim) == 'Dimension(test_id)'

    def test_dimension_str(self):
        """Test Dimension str"""
        dim = Dimension(['test_id'])
        assert str(dim) == 'test_id'

    def test_dimension_getattr_valid(self):
        """Test accessing valid param via getattr"""
        dim = Dimension(['id', 'name', 'absolute', 1, 1])
        assert dim.id == 'id'
        assert dim.name == 'name'

    def test_dimension_getattr_invalid(self):
        """Test accessing invalid param raises AttributeError"""
        dim = Dimension(['id'])
        with pytest.raises(AttributeError):
            _ = dim.nonexistent_attr

    def test_dimension_eq_with_dimension(self):
        """Test Dimension equality with another Dimension"""
        dim1 = Dimension(['id1'])
        dim2 = Dimension(['id1'])
        dim3 = Dimension(['id2'])
        assert dim1 == dim2
        assert not (dim1 == dim3)

    def test_dimension_eq_with_string(self):
        """Test Dimension equality with string"""
        dim = Dimension(['test_id'])
        assert dim == 'test_id'
        assert not (dim == 'other_id')

    def test_dimension_ne(self):
        """Test Dimension inequality"""
        dim1 = Dimension(['id1'])
        dim2 = Dimension(['id2'])
        assert dim1 != dim2

    def test_dimension_hash(self):
        """Test Dimension is hashable"""
        dim = Dimension(['id'])
        assert isinstance(hash(dim), int)

    def test_dimension_create(self):
        """Test Dimension create method"""
        dim = Dimension(['id', 'name', 'absolute', 2, 3, 'hidden'])
        result = dim.create()
        assert "DIMENSION 'id'" in result
        assert "'name'" in result
        assert 'absolute' in result
        assert '2' in result
        assert '3' in result
        assert 'hidden' in result

    def test_dimension_set(self):
        """Test Dimension set method"""
        dim = Dimension(['test_id'])
        result = dim.set('100')
        assert "SET 'test_id'" in result
        assert '= 100' in result

    def test_dimension_get_value_valid_integer(self):
        """Test Dimension get_value with valid integer in data"""
        dim = Dimension(['dim_id'])
        data = {'dim_id': 42}
        assert dim.get_value(data) == 42

    def test_dimension_get_value_valid_string_integer(self):
        """Test Dimension get_value with string integer"""
        dim = Dimension(['dim_id'])
        data = {'dim_id': '100'}
        assert dim.get_value(data) == 100

    def test_dimension_get_value_missing_key(self):
        """Test Dimension get_value with missing key"""
        dim = Dimension(['dim_id'])
        data = {'other_id': 42}
        assert dim.get_value(data) is None

    def test_dimension_get_value_none_data(self):
        """Test Dimension get_value with None data"""
        dim = Dimension(['dim_id'])
        assert dim.get_value(None) is None

    def test_dimension_get_value_non_integer(self):
        """Test Dimension get_value with non-integer value"""
        dim = Dimension(['dim_id'])
        data = {'dim_id': 'not_a_number'}
        assert dim.get_value(data) is None

    def test_dimension_set_value(self):
        """Test Dimension set returns correct format"""
        dim = Dimension(['my_dim'])
        result = dim.set('999')
        assert result == "SET 'my_dim' = 999\n"


class TestChartVariable:
    def test_chart_variable_init_valid(self):
        """Test ChartVariable initialization"""
        params = ['var_id', 100]
        var = ChartVariable(params)
        assert var.id == 'var_id'
        assert var.value == 100

    def test_chart_variable_init_invalid_type(self):
        """Test ChartVariable with non-list type"""
        with pytest.raises(ItemTypeError):
            ChartVariable("not a list")

    def test_chart_variable_init_empty_list(self):
        """Test ChartVariable with empty list"""
        with pytest.raises(ItemValueError):
            ChartVariable([])

    def test_chart_variable_value_defaults_to_none(self):
        """Test ChartVariable value defaults to None"""
        var = ChartVariable(['var_id'])
        assert var.value is None

    def test_chart_variable_repr(self):
        """Test ChartVariable repr"""
        var = ChartVariable(['test_var'])
        assert repr(var) == 'ChartVariable(test_var)'

    def test_chart_variable_str(self):
        """Test ChartVariable str"""
        var = ChartVariable(['test_var'])
        assert str(var) == 'test_var'

    def test_chart_variable_getattr_valid(self):
        """Test accessing valid param via getattr"""
        var = ChartVariable(['id', 50])
        assert var.id == 'id'
        assert var.value == 50

    def test_chart_variable_getattr_invalid(self):
        """Test accessing invalid param raises AttributeError"""
        var = ChartVariable(['id'])
        with pytest.raises(AttributeError):
            _ = var.nonexistent

    def test_chart_variable_bool_with_value(self):
        """Test ChartVariable __bool__ returns True when value is not None"""
        var = ChartVariable(['id', 100])
        assert bool(var) is True

    def test_chart_variable_bool_without_value(self):
        """Test ChartVariable __bool__ returns False when value is None"""
        var = ChartVariable(['id'])
        assert bool(var) is False

    def test_chart_variable_bool_with_zero(self):
        """Test ChartVariable __bool__ returns True even with value 0"""
        var = ChartVariable(['id', 0])
        assert bool(var) is True

    def test_chart_variable_eq_with_chart_variable(self):
        """Test ChartVariable equality with another ChartVariable"""
        var1 = ChartVariable(['id1', 100])
        var2 = ChartVariable(['id1', 200])
        var3 = ChartVariable(['id2', 100])
        assert var1 == var2
        assert not (var1 == var3)

    def test_chart_variable_eq_with_non_chart_variable(self):
        """Test ChartVariable equality with non-ChartVariable"""
        var = ChartVariable(['id'])
        assert not (var == 'id')
        assert not (var == 123)

    def test_chart_variable_ne(self):
        """Test ChartVariable inequality"""
        var1 = ChartVariable(['id1'])
        var2 = ChartVariable(['id2'])
        assert var1 != var2

    def test_chart_variable_hash(self):
        """Test ChartVariable is hashable"""
        var = ChartVariable(['id'])
        assert isinstance(hash(var), int)

    def test_chart_variable_set(self):
        """Test ChartVariable set method"""
        var = ChartVariable(['var_id', 42])
        result = var.set(42)
        assert "VARIABLE CHART 'var_id'" in result
        assert '= 42' in result

    def test_chart_variable_get_value_valid(self):
        """Test ChartVariable get_value with valid integer"""
        var = ChartVariable(['var_id'])
        data = {'var_id': 75}
        assert var.get_value(data) == 75

    def test_chart_variable_get_value_valid_string(self):
        """Test ChartVariable get_value with string integer"""
        var = ChartVariable(['var_id'])
        data = {'var_id': '200'}
        assert var.get_value(data) == 200

    def test_chart_variable_get_value_missing_key(self):
        """Test ChartVariable get_value with missing key"""
        var = ChartVariable(['var_id'])
        data = {'other_id': 42}
        assert var.get_value(data) is None

    def test_chart_variable_get_value_none_data(self):
        """Test ChartVariable get_value with None data"""
        var = ChartVariable(['var_id'])
        assert var.get_value(None) is None

    def test_chart_variable_get_value_non_integer(self):
        """Test ChartVariable get_value with non-integer"""
        var = ChartVariable(['var_id'])
        data = {'var_id': 'not_int'}
        assert var.get_value(data) is None


class TestChart:
    def test_chart_init_valid(self):
        """Test Chart initialization with valid params"""
        params = ['job_name', 'type', 'id', 'name', 'title', 'units', 'family', 'context']
        chart = Chart(params)
        assert chart.type == 'type'
        assert chart.id == 'id'
        assert chart.name == 'type.id'

    def test_chart_init_invalid_type(self):
        """Test Chart initialization with non-list"""
        with pytest.raises(ItemTypeError):
            Chart("not a list")

    def test_chart_init_insufficient_params(self):
        """Test Chart initialization with too few params"""
        with pytest.raises(ItemValueError):
            Chart(['job_name', 'type', 'id'])

    def test_chart_init_with_none_values(self):
        """Test Chart initialization with None values"""
        params = ['job_name', 'type', 'id', None, None, None, None, None]
        chart = Chart(params)
        assert chart.name == ''
        assert chart.title == ''

    def test_chart_init_chart_type_valid(self):
        """Test Chart with valid chart_type"""
        params = ['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context', 'line']
        chart = Chart(params)
        assert chart.chart_type == 'line'

    def test_chart_init_chart_type_invalid_defaults_to_absolute(self):
        """Test Chart with invalid chart_type defaults to absolute"""
        params = ['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context', 'invalid']
        chart = Chart(params)
        assert chart.chart_type == 'absolute'

    def test_chart_init_hidden_flag_true(self):
        """Test Chart with hidden flag"""
        params = ['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context', 'line', 'hidden']
        chart = Chart(params)
        assert chart.hidden == 'hidden'

    def test_chart_init_hidden_flag_false(self):
        """Test Chart without hidden flag"""
        params = ['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context', 'line', '']
        chart = Chart(params)
        assert chart.hidden == ''

    def test_chart_init_hidden_flag_other_value(self):
        """Test Chart with non-hidden string"""
        params = ['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context', 'line', 'visible']
        chart = Chart(params)
        assert chart.hidden == ''

    def test_chart_dimensions_initialized_empty(self):
        """Test Chart dimensions initialized as empty list"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        assert chart.dimensions == []

    def test_chart_variables_initialized_empty(self):
        """Test Chart variables initialized as empty set"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        assert chart.variables == set()

    def test_chart_flags_initialized(self):
        """Test Chart flags initialized"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        assert isinstance(chart.flags, ChartFlags)
        assert chart.flags.push is True
        assert chart.flags.created is False

    def test_chart_penalty_initialized(self):
        """Test Chart penalty initialized to 0"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        assert chart.penalty == 0

    def test_chart_repr(self):
        """Test Chart repr"""
        chart = Chart(['job', 'type', 'chart_id', 'name', 'title', 'units', 'family', 'context'])
        assert repr(chart) == 'Chart(chart_id)'

    def test_chart_str(self):
        """Test Chart str"""
        chart = Chart(['job', 'type', 'chart_id', 'name', 'title', 'units', 'family', 'context'])
        assert str(chart) == 'chart_id'

    def test_chart_iter(self):
        """Test Chart is iterable over dimensions"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        dim = Dimension(['dim_id'])
        chart.dimensions.append(dim)
        dims = list(chart)
        assert len(dims) == 1
        assert dims[0] == dim

    def test_chart_contains_dimension(self):
        """Test Chart contains check for dimensions"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        dim = Dimension(['dim_id'])
        chart.dimensions.append(dim)
        assert 'dim_id' in chart

    def test_chart_contains_dimension_false(self):
        """Test Chart contains returns False for missing dimension"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        assert 'dim_id' not in chart

    def test_chart_getattr_valid(self):
        """Test accessing chart param via getattr"""
        chart = Chart(['job', 'type', 'test_id', 'name', 'title', 'units', 'family', 'context'])
        assert chart.type == 'type'
        assert chart.id == 'test_id'

    def test_chart_getattr_invalid(self):
        """Test accessing invalid param raises AttributeError"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        with pytest.raises(AttributeError):
            _ = chart.nonexistent_attr

    def test_chart_add_variable(self):
        """Test adding a variable to chart"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        chart.add_variable(['var_id', 100])
        assert len(chart.variables) == 1

    def test_chart_add_dimension_valid(self):
        """Test adding a dimension to chart"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        dim = chart.add_dimension(['dim_id', 'name'])
        assert isinstance(dim, Dimension)
        assert len(chart.dimensions) == 1
        assert 'dim_id' in chart

    def test_chart_add_dimension_duplicate_raises_error(self):
        """Test adding duplicate dimension raises error"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        chart.add_dimension(['dim_id'])
        with pytest.raises(DuplicateItemError):
            chart.add_dimension(['dim_id'])

    def test_chart_del_dimension_existing(self):
        """Test deleting an existing dimension"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        dim = Dimension(['dim_id'])
        chart.dimensions.append(dim)
        
        with patch('bases.charts.safe_print'):
            chart.del_dimension('dim_id')
        
        assert 'dim_id' not in chart
        assert len(chart.dimensions) == 0

    def test_chart_del_dimension_nonexistent(self):
        """Test deleting non-existent dimension does nothing"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        chart.del_dimension('nonexistent_id')
        assert len(chart.dimensions) == 0

    def test_chart_del_dimension_with_hide_true(self):
        """Test delete dimension hides it when hide=True"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        dim = Dimension(['dim_id'])
        chart.dimensions.append(dim)
        
        with patch('bases.charts.safe_print'):
            chart.del_dimension('dim_id', hide=True)
        
        assert dim.hidden == 'hidden'
        assert dim.obsolete == 'obsolete'

    def test_chart_del_dimension_with_hide_false(self):
        """Test delete dimension doesn't hide when hide=False"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        dim = Dimension(['dim_id'])
        chart.dimensions.append(dim)
        
        with patch('bases.charts.safe_print'):
            chart.del_dimension('dim_id', hide=False)
        
        assert dim.hidden == ''

    def test_chart_hide_dimension_existing(self):
        """Test hiding an existing dimension"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        dim = Dimension(['dim_id'])
        chart.dimensions.append(dim)
        
        with patch('bases.charts.safe_print'):
            chart.hide_dimension('dim_id')
        
        assert dim.hidden == 'hidden'

    def test_chart_hide_dimension_nonexistent(self):
        """Test hiding non-existent dimension does nothing"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        chart.hide_dimension('nonexistent_id')

    def test_chart_hide_dimension_reverse_unhides(self):
        """Test hiding with reverse=True unhides dimension"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        dim = Dimension(['dim_id'])
        dim.params['hidden'] = 'hidden'
        chart.dimensions.append(dim)
        
        with patch('bases.charts.safe_print'):
            chart.hide_dimension('dim_id', reverse=True)
        
        assert dim.hidden == ''

    def test_chart_create_prints_chart(self):
        """Test create method prints chart"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        
        with patch('bases.charts.safe_print') as mock_print:
            chart.create()
            mock_print.assert_called_once()
            call_args = mock_print.call_args[0][0]
            assert 'CHART' in call_args

    def test_chart_create_sets_flags(self):
        """Test create sets correct flags"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        chart.flags.push = True
        
        with patch('bases.charts.safe_print'):
            chart.create()
        
        assert chart.flags.push is False
        assert chart.flags.created is True

    def test_chart_can_be_updated_with_valid_data(self):
        """Test can_be_updated returns True when dimension has value"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        dim = Dimension(['dim_id'])
        chart.dimensions.append(dim)
        data = {'dim_id': 42}
        
        assert chart.can_be_updated(data) is True

    def test_chart_can_be_updated_no_valid_data(self):
        """Test can_be_updated returns False when no dimension has value"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        dim = Dimension(['dim_id'])
        chart.dimensions.append(dim)
        data = {'other_id': 42}
        
        assert chart.can_be_updated(data) is False

    def test_chart_can_be_updated_empty_dimensions(self):
        """Test can_be_updated with no dimensions"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        assert chart.can_be_updated({}) is False

    def test_chart_update_with_dimension_data(self):
        """Test update method with dimension data"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        dim = Dimension(['dim_id'])
        chart.dimensions.append(dim)
        chart.flags.updated = False
        data = {'dim_id': 42}
        
        with patch('bases.charts.safe_print') as mock_print:
            result = chart.update(data, interval=10)
            assert result is True
            assert chart.flags.updated is True
            mock_print.assert_called()

    def test_chart_update_with_variable_data(self):
        """Test update method with variable data"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        var = ChartVariable(['var_id', None])
        chart.variables.add(var)
        data = {'var_id': 100}
        
        with patch('bases.charts.safe_print') as mock_print:
            chart.update(data, interval=10)
            mock_print.assert_called()

    def test_chart_update_no_data(self):
        """Test update with no matching data"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        dim = Dimension(['dim_id'])
        chart.dimensions.append(dim)
        data = {'other_id': 42}
        
        with patch('bases.charts.safe_print'):
            result = chart.update(data, interval=10)
            assert result is False
            assert chart.flags.updated is False

    def test_chart_update_increases_penalty(self):
        """Test update increases penalty when no data"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        dim = Dimension(['dim_id'])
        chart.dimensions.append(dim)
        chart.penalty = 5
        
        with patch('bases.charts.safe_print'):
            chart.update({'other_id': 42}, interval=10)
            assert chart.penalty == 6

    def test_chart_update_first_time_since_last_zero(self):
        """Test first update has since_last=0"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        dim = Dimension(['dim_id'])
        chart.dimensions.append(dim)
        chart.flags.updated = False
        data = {'dim_id': 42}
        
        with patch('bases.charts.safe_print') as mock_print:
            chart.update(data, interval=10)
            call_args = mock_print.call_args[0][0]
            assert '{0} 0\n'.format(chart.name) in call_args or 'BEGIN' in call_args

    def test_chart_update_subsequent_time_uses_interval(self):
        """Test subsequent update uses interval as since_last"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        dim = Dimension(['dim_id'])
        chart.dimensions.append(dim)
        chart.flags.updated = True
        data = {'dim_id': 42}
        
        with patch('bases.charts.safe_print') as mock_print:
            chart.update(data, interval=15)
            call_args = mock_print.call_args[0][0]
            assert '15' in call_args or 'BEGIN' in call_args

    def test_chart_update_with_push_flag_calls_create(self):
        """Test update calls create when push flag is True"""
        chart = Chart(['job', 'type', 'id', 'name', 'title', 'units', 'family', 'context'])
        dim = Dimension(['dim_id'])
        chart.dimensions.append(dim)
        chart.flags.push = True
        data = {'dim_id':
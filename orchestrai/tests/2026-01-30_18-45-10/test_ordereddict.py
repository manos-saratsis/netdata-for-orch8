import unittest
from ..third_party.ordereddict import OrderedDict


class TestOrderedDictInit(unittest.TestCase):
    """Test OrderedDict.__init__ method"""

    def test_init_empty(self):
        """Should initialize an empty OrderedDict"""
        od = OrderedDict()
        self.assertEqual(len(od), 0)

    def test_init_with_dict(self):
        """Should initialize with a dictionary"""
        od = OrderedDict({'a': 1, 'b': 2})
        self.assertIn('a', od)
        self.assertIn('b', od)

    def test_init_with_kwargs(self):
        """Should initialize with keyword arguments"""
        od = OrderedDict(a=1, b=2)
        self.assertIn('a', od)
        self.assertIn('b', od)

    def test_init_with_list_of_tuples(self):
        """Should initialize with list of tuples"""
        od = OrderedDict([('a', 1), ('b', 2)])
        self.assertEqual(list(od.keys()), ['a', 'b'])

    def test_init_with_too_many_args_raises_typeerror(self):
        """Should raise TypeError when more than 1 positional argument given"""
        with self.assertRaises(TypeError) as cm:
            OrderedDict({'a': 1}, {'b': 2})
        self.assertIn('expected at most 1 arguments', str(cm.exception))

    def test_init_with_both_args_and_kwargs(self):
        """Should initialize with both positional arg and kwargs"""
        od = OrderedDict({'a': 1}, b=2)
        self.assertIn('a', od)
        self.assertIn('b', od)


class TestOrderedDictClear(unittest.TestCase):
    """Test OrderedDict.clear method"""

    def test_clear_empty_dict(self):
        """Should clear an empty dict without error"""
        od = OrderedDict()
        od.clear()
        self.assertEqual(len(od), 0)

    def test_clear_nonempty_dict(self):
        """Should remove all items from dict"""
        od = OrderedDict([('a', 1), ('b', 2)])
        od.clear()
        self.assertEqual(len(od), 0)
        self.assertNotIn('a', od)

    def test_clear_reinitializes_internal_structures(self):
        """Should reinitialize internal __end and __map structures"""
        od = OrderedDict([('a', 1)])
        od.clear()
        od['x'] = 10
        self.assertEqual(od['x'], 10)


class TestOrderedDictSetitem(unittest.TestCase):
    """Test OrderedDict.__setitem__ method"""

    def test_setitem_new_key(self):
        """Should add a new key-value pair"""
        od = OrderedDict()
        od['a'] = 1
        self.assertEqual(od['a'], 1)

    def test_setitem_update_existing_key(self):
        """Should update value for existing key"""
        od = OrderedDict()
        od['a'] = 1
        od['a'] = 2
        self.assertEqual(od['a'], 2)

    def test_setitem_maintains_order(self):
        """Should maintain insertion order"""
        od = OrderedDict()
        od['a'] = 1
        od['b'] = 2
        od['c'] = 3
        self.assertEqual(list(od.keys()), ['a', 'b', 'c'])

    def test_setitem_multiple_keys(self):
        """Should handle multiple different keys"""
        od = OrderedDict()
        for i in range(10):
            od[str(i)] = i
        self.assertEqual(len(od), 10)


class TestOrderedDictDelitem(unittest.TestCase):
    """Test OrderedDict.__delitem__ method"""

    def test_delitem_single_item(self):
        """Should delete a single item"""
        od = OrderedDict([('a', 1), ('b', 2)])
        del od['a']
        self.assertNotIn('a', od)
        self.assertIn('b', od)

    def test_delitem_raises_keyerror_for_missing_key(self):
        """Should raise KeyError for missing key"""
        od = OrderedDict()
        with self.assertRaises(KeyError):
            del od['missing']

    def test_delitem_maintains_order_after_deletion(self):
        """Should maintain order after deletion"""
        od = OrderedDict([('a', 1), ('b', 2), ('c', 3)])
        del od['b']
        self.assertEqual(list(od.keys()), ['a', 'c'])

    def test_delitem_first_item(self):
        """Should delete first item correctly"""
        od = OrderedDict([('a', 1), ('b', 2)])
        del od['a']
        self.assertEqual(list(od.keys()), ['b'])

    def test_delitem_last_item(self):
        """Should delete last item correctly"""
        od = OrderedDict([('a', 1), ('b', 2)])
        del od['b']
        self.assertEqual(list(od.keys()), ['a'])


class TestOrderedDictIter(unittest.TestCase):
    """Test OrderedDict.__iter__ method"""

    def test_iter_empty_dict(self):
        """Should iterate over empty dict without error"""
        od = OrderedDict()
        keys = list(od)
        self.assertEqual(keys, [])

    def test_iter_returns_keys_in_order(self):
        """Should iterate keys in insertion order"""
        od = OrderedDict([('a', 1), ('b', 2), ('c', 3)])
        keys = list(od)
        self.assertEqual(keys, ['a', 'b', 'c'])

    def test_iter_single_item(self):
        """Should iterate single item"""
        od = OrderedDict([('a', 1)])
        keys = list(od)
        self.assertEqual(keys, ['a'])

    def test_iter_after_deletion(self):
        """Should iterate correctly after deletions"""
        od = OrderedDict([('a', 1), ('b', 2), ('c', 3)])
        del od['b']
        keys = list(od)
        self.assertEqual(keys, ['a', 'c'])


class TestOrderedDictReversed(unittest.TestCase):
    """Test OrderedDict.__reversed__ method"""

    def test_reversed_empty_dict(self):
        """Should iterate over empty dict in reverse"""
        od = OrderedDict()
        keys = list(reversed(od))
        self.assertEqual(keys, [])

    def test_reversed_returns_keys_in_reverse_order(self):
        """Should iterate keys in reverse insertion order"""
        od = OrderedDict([('a', 1), ('b', 2), ('c', 3)])
        keys = list(reversed(od))
        self.assertEqual(keys, ['c', 'b', 'a'])

    def test_reversed_single_item(self):
        """Should reverse iterate single item"""
        od = OrderedDict([('a', 1)])
        keys = list(reversed(od))
        self.assertEqual(keys, ['a'])

    def test_reversed_after_deletion(self):
        """Should reverse iterate correctly after deletions"""
        od = OrderedDict([('a', 1), ('b', 2), ('c', 3)])
        del od['b']
        keys = list(reversed(od))
        self.assertEqual(keys, ['c', 'a'])


class TestOrderedDictPopitem(unittest.TestCase):
    """Test OrderedDict.popitem method"""

    def test_popitem_empty_dict_raises_keyerror(self):
        """Should raise KeyError for empty dict"""
        od = OrderedDict()
        with self.assertRaises(KeyError) as cm:
            od.popitem()
        self.assertIn('dictionary is empty', str(cm.exception))

    def test_popitem_last_true_removes_last_item(self):
        """Should remove and return last item when last=True"""
        od = OrderedDict([('a', 1), ('b', 2), ('c', 3)])
        key, value = od.popitem(last=True)
        self.assertEqual(key, 'c')
        self.assertEqual(value, 3)
        self.assertNotIn('c', od)

    def test_popitem_last_false_removes_first_item(self):
        """Should remove and return first item when last=False"""
        od = OrderedDict([('a', 1), ('b', 2), ('c', 3)])
        key, value = od.popitem(last=False)
        self.assertEqual(key, 'a')
        self.assertEqual(value, 1)
        self.assertNotIn('a', od)

    def test_popitem_default_is_last(self):
        """Should default to last=True"""
        od = OrderedDict([('a', 1), ('b', 2)])
        key, value = od.popitem()
        self.assertEqual(key, 'b')

    def test_popitem_single_item(self):
        """Should popitem from single-item dict"""
        od = OrderedDict([('a', 1)])
        key, value = od.popitem()
        self.assertEqual(key, 'a')
        self.assertEqual(value, 1)
        self.assertEqual(len(od), 0)


class TestOrderedDictReduce(unittest.TestCase):
    """Test OrderedDict.__reduce__ method"""

    def test_reduce_empty_dict(self):
        """Should reduce empty OrderedDict correctly"""
        od = OrderedDict()
        result = od.__reduce__()
        self.assertEqual(len(result), 2)
        self.assertEqual(result[0], OrderedDict)
        self.assertEqual(result[1][0], [])

    def test_reduce_nonempty_dict(self):
        """Should reduce non-empty OrderedDict correctly"""
        od = OrderedDict([('a', 1), ('b', 2)])
        result = od.__reduce__()
        self.assertEqual(result[0], OrderedDict)
        self.assertIsInstance(result[1][0], list)

    def test_reduce_with_instance_dict(self):
        """Should handle instance dict in reduce"""
        od = OrderedDict([('a', 1)])
        od.custom_attr = 'value'
        result = od.__reduce__()
        # Should have 3 parts when instance dict is non-empty
        self.assertEqual(len(result), 3)


class TestOrderedDictKeys(unittest.TestCase):
    """Test OrderedDict.keys method"""

    def test_keys_empty_dict(self):
        """Should return empty list for empty dict"""
        od = OrderedDict()
        keys = od.keys()
        self.assertEqual(keys, [])

    def test_keys_returns_list(self):
        """Should return list of keys"""
        od = OrderedDict([('a', 1), ('b', 2)])
        keys = od.keys()
        self.assertIsInstance(keys, list)
        self.assertEqual(keys, ['a', 'b'])

    def test_keys_maintains_order(self):
        """Should return keys in insertion order"""
        od = OrderedDict([('x', 1), ('y', 2), ('z', 3)])
        keys = od.keys()
        self.assertEqual(keys, ['x', 'y', 'z'])


class TestOrderedDictRepr(unittest.TestCase):
    """Test OrderedDict.__repr__ method"""

    def test_repr_empty_dict(self):
        """Should have correct repr for empty dict"""
        od = OrderedDict()
        result = repr(od)
        self.assertEqual(result, 'OrderedDict()')

    def test_repr_nonempty_dict(self):
        """Should have correct repr for non-empty dict"""
        od = OrderedDict([('a', 1)])
        result = repr(od)
        self.assertIn('OrderedDict', result)
        self.assertIn('a', result)

    def test_repr_multiple_items(self):
        """Should represent multiple items"""
        od = OrderedDict([('a', 1), ('b', 2)])
        result = repr(od)
        self.assertIn('a', result)
        self.assertIn('b', result)


class TestOrderedDictCopy(unittest.TestCase):
    """Test OrderedDict.copy method"""

    def test_copy_empty_dict(self):
        """Should copy empty dict"""
        od = OrderedDict()
        copy = od.copy()
        self.assertIsInstance(copy, OrderedDict)
        self.assertEqual(len(copy), 0)

    def test_copy_nonempty_dict(self):
        """Should copy non-empty dict"""
        od = OrderedDict([('a', 1), ('b', 2)])
        copy = od.copy()
        self.assertEqual(copy['a'], 1)
        self.assertEqual(copy['b'], 2)

    def test_copy_is_independent(self):
        """Should create independent copy"""
        od = OrderedDict([('a', 1)])
        copy = od.copy()
        copy['b'] = 2
        self.assertNotIn('b', od)

    def test_copy_preserves_order(self):
        """Should preserve insertion order in copy"""
        od = OrderedDict([('a', 1), ('b', 2), ('c', 3)])
        copy = od.copy()
        self.assertEqual(list(copy.keys()), ['a', 'b', 'c'])


class TestOrderedDictFromkeys(unittest.TestCase):
    """Test OrderedDict.fromkeys class method"""

    def test_fromkeys_with_list(self):
        """Should create OrderedDict from list of keys"""
        od = OrderedDict.fromkeys(['a', 'b', 'c'])
        self.assertEqual(len(od), 3)
        self.assertIn('a', od)

    def test_fromkeys_with_default_value(self):
        """Should create OrderedDict with default value"""
        od = OrderedDict.fromkeys(['a', 'b'], 'default')
        self.assertEqual(od['a'], 'default')
        self.assertEqual(od['b'], 'default')

    def test_fromkeys_with_none_value(self):
        """Should handle None as value"""
        od = OrderedDict.fromkeys(['a', 'b'])
        self.assertIsNone(od['a'])
        self.assertIsNone(od['b'])

    def test_fromkeys_maintains_order(self):
        """Should maintain key order from iterable"""
        od = OrderedDict.fromkeys(['z', 'y', 'x'])
        self.assertEqual(list(od.keys()), ['z', 'y', 'x'])

    def test_fromkeys_empty_iterable(self):
        """Should handle empty iterable"""
        od = OrderedDict.fromkeys([])
        self.assertEqual(len(od), 0)


class TestOrderedDictEquality(unittest.TestCase):
    """Test OrderedDict.__eq__ and __ne__ methods"""

    def test_eq_same_ordereddict(self):
        """Should be equal to OrderedDict with same items in same order"""
        od1 = OrderedDict([('a', 1), ('b', 2)])
        od2 = OrderedDict([('a', 1), ('b', 2)])
        self.assertTrue(od1 == od2)

    def test_eq_different_order_ordereddict(self):
        """Should not be equal if order differs"""
        od1 = OrderedDict([('a', 1), ('b', 2)])
        od2 = OrderedDict([('b', 2), ('a', 1)])
        self.assertFalse(od1 == od2)

    def test_eq_different_length_ordereddict(self):
        """Should not be equal if lengths differ"""
        od1 = OrderedDict([('a', 1)])
        od2 = OrderedDict([('a', 1), ('b', 2)])
        self.assertFalse(od1 == od2)

    def test_eq_different_values_ordereddict(self):
        """Should not be equal if values differ"""
        od1 = OrderedDict([('a', 1)])
        od2 = OrderedDict([('a', 2)])
        self.assertFalse(od1 == od2)

    def test_eq_regular_dict(self):
        """Should equal regular dict with same items"""
        od = OrderedDict([('a', 1), ('b', 2)])
        d = {'a': 1, 'b': 2}
        self.assertTrue(od == d)

    def test_ne_different_ordereddict(self):
        """Should not be equal when items differ"""
        od1 = OrderedDict([('a', 1)])
        od2 = OrderedDict([('b', 2)])
        self.assertTrue(od1 != od2)

    def test_ne_same_ordereddict(self):
        """Should be equal when items are same"""
        od1 = OrderedDict([('a', 1)])
        od2 = OrderedDict([('a', 1)])
        self.assertFalse(od1 != od2)


class TestOrderedDictMixedOperations(unittest.TestCase):
    """Test OrderedDict with mixed operations"""

    def test_setitem_after_clear(self):
        """Should work correctly after clear"""
        od = OrderedDict([('a', 1)])
        od.clear()
        od['b'] = 2
        self.assertEqual(od['b'], 2)

    def test_multiple_operations_in_sequence(self):
        """Should handle multiple operations in sequence"""
        od = OrderedDict()
        od['a'] = 1
        od['b'] = 2
        del od['a']
        od['c'] = 3
        self.assertEqual(list(od.keys()), ['b', 'c'])

    def test_pop_from_dict(self):
        """Should pop items correctly"""
        od = OrderedDict([('a', 1), ('b', 2)])
        value = od.pop('a')
        self.assertEqual(value, 1)
        self.assertNotIn('a', od)

    def test_update_dict(self):
        """Should update from dict"""
        od = OrderedDict([('a', 1)])
        od.update({'b': 2, 'c': 3})
        self.assertIn('b', od)
        self.assertIn('c', od)

    def test_setdefault_new_key(self):
        """Should set default for new key"""
        od = OrderedDict()
        result = od.setdefault('a', 1)
        self.assertEqual(result, 1)
        self.assertEqual(od['a'], 1)

    def test_setdefault_existing_key(self):
        """Should return existing value"""
        od = OrderedDict([('a', 1)])
        result = od.setdefault('a', 2)
        self.assertEqual(result, 1)
        self.assertEqual(od['a'], 1)
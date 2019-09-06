import unittest

import so3g
from spt3g import core
SEC = core.G3Units.sec

import numpy as np

def get_test_block(length, keys, offset=0):
    type_cycle = [core.G3VectorDouble, core.G3VectorInt]
    t0 = core.G3Time('2019-01-01T12:30:00') + offset*SEC
    m = so3g.IrregBlock()
    m.times = core.G3VectorTime([t0 + t*SEC for t in np.arange(length)])
    for i,k in enumerate(keys):
        y = (np.random.uniform(size=length) * 100).astype(int)
        m[k] = type_cycle[i % len(type_cycle)](y)
    return m
    
class TestIrregBlock(unittest.TestCase):
    def test_internal_checks(self):
        # Construct invalid blocks.
        m = so3g.IrregBlock()
        t0 = core.G3Time('2019-01-01T12:30:00')
        m.t = core.G3VectorTime([t0, t0 + 10*SEC, t0 + 20*SEC])
        m['x'] = core.G3VectorDouble([1,2])
        with self.assertRaises(ValueError):
            m.Check()

    def test_concat(self):
        # Test concatenation.
        key_list = ['x', 'y', 'z']
        m0 = get_test_block(100, key_list)
        m1 = get_test_block(200, key_list, 100)
        for fail_type, fail_vec in [
                (ValueError, get_test_block(200, key_list + ['extra'], 100)),
                (ValueError, get_test_block(200, key_list[:-1], 100)),
                ]:
            with self.assertRaises(fail_type):
                m0.Concatenate(fail_vec)

    def test_serialization(self):
        m0 = get_test_block(100, ['x', 'y'])
        m1 = get_test_block(200, ['x', 'y'], 100)
        m2 = m0.Concatenate(m1)
        f = core.G3Frame()
        f['irreg0'] = m0
        f['irreg1'] = m1
        core.G3Writer('test.g3').Process(f)
        f = core.G3Reader('test.g3').Process(None)[0]
        f['irreg0'].Concatenate(f['irreg1'])['x']
                
if __name__ == '__main__':
    unittest.main()
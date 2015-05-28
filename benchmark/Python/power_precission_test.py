from __future__ import print_function
import util
#if util.Benchmark().bohrium:
#    import bohrium as np
#else:
#    import numpy as np
import bohrium as np


def benchmark():
    B = util.Benchmark()

    np.random.seed(seed=1)

    m = np.array(np.random.random([N]), dtype=dtype)
    m = m*10

    R = np.power(m, 31)

    return (m, R.tolist())

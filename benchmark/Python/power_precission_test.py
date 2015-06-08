from __future__ import print_function
import util
#if util.Benchmark().bohrium:
#    import bohrium as np
#else:
#    import numpy as np
import bohrium as np


def benchmark(n, dtype=np.float64):
    B = util.Benchmark()

    np.random.seed(seed=1)

    m = np.array(np.random.random(n), dtype=dtype)
    m = m*10

    R = np.power(m, 31)

    """
    (a1,a2,a3,a4,a5) = (0.31938153, -0.356563782, 1.781477937, -1.821255978, 1.330274429)
    K = m
    R = a1*K + a2*(K**2) + a3*(K**3) + a4*(K**4) + a5*(K**5)
    """

    return (m.tolist(), R.tolist())

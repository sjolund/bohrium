from __future__ import print_function
import util
B = util.Benchmark()
"""if B.bohrium:
    import bohrium as np
else:
    import numpy as np
"""
import bohrium as np

def montecarlo_pi(N, I):
    acc=0.0
    for i in xrange(I):
        x = np.array(np.random.random(N), dtype=B.dtype)
        y = np.array(np.random.random(N), dtype=B.dtype)

        z = np.sqrt(x**2 + y**2) <= 1.0
        acc += np.sum(z) * 4.0 / N

    acc /= I
    return acc

def main():
    N, I = B.size
    B.start()
    R = montecarlo_pi(N, I)
    B.stop()
    B.pprint()

def compute_mc(N,I):
    B.start()
    R = montecarlo_pi(N, I)
    B.stop()
    return (I,B.elapsed())

def benchmark():
    N = 10000
    measures = [compute_mc(N, I) for I in [8,16,32,64,128,256,512]]
    (x,y) = [list(t) for t in zip(*measures)]

    return (x,y)

if __name__ == "__main__":
    main()

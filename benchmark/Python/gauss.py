import util

import bohrium as np
from bohriumbridge import flush

def gauss(a):
    """
    Performe Gausian elimination on matrix a without pivoting
    """
    for c in xrange(1,a.shape[0]):
        a[c:,c-1:] = a[c:,c-1:] - (a[c:,c-1]/a[c-1,c-1:c])[:,None] * a[c-1,c-1:]
        if util.Benchmark().bohrium:        
            flush()
    a /= np.diagonal(a)[:,None]
    return a

def main():
    B = util.Benchmark()
    N = B.size[0]

    S = np.random.random((N, N), dtype=B.dtype,bohrium=False)
    if util.Benchmark().bohrium:
        S.bohrium=True


    B.start()
    R = gauss(S)
    B.stop()

    B.pprint()

if __name__ == "__main__":
    main()

import util

import bohrium as np
from bohriumbridge import flush

def lu(a,i):
    """
    Performe LU decomposition on the matrix a so A = L*U
    """
    u = a.copy()
    l = np.zeros_like(a)
    np.diagonal(l)[:] = 1.0
    for c in xrange(1,i+1):
        l[c:,c-1] = (u[c:,c-1]/u[c-1,c-1:c])
        u[c:,c-1:] = u[c:,c-1:] - l[c:,c-1][:,None] * u[c-1,c-1:]
        flush()
    return (l,u)

def main():
    B = util.Benchmark()
    N = B.size[0]
    I = B.size[1]

    S = np.random.random((N, N), dtype=B.dtype,bohrium=False)
    if util.Benchmark().bohrium:
        S.bohrium=True


    B.start()
    (L,U) = lu(S,I)
    L.bohrium=False
    U.bohrium=False
    B.stop()

    B.pprint()

if __name__ == "__main__":
    main()

from __future__ import print_function
import util
"""if util.Benchmark().bohrium:
    import bohrium as np
else:
    import numpy as np"""
import bohrium as np

def model(N, dtype=np.float32):
    """Construct pseudo-data representing price samples?"""
    np.random.seed(seed=1)

    S = np.array(np.random.random([N]), dtype=dtype)
    S = S*4.0-2.0 + 60.0 # Price is between 58-62

    return S

def CND(X):
    """
    Cumulative normal distribution.
    Helper function used by BS(...).
    """

    (a1,a2,a3,a4,a5) = (0.31938153, -0.356563782, 1.781477937, -1.821255978, 1.330274429)
    L = np.absolute(X)
    K = 1.0 / (1.0 + 0.2316419 * L)
    w = 1.0 - 1.0 / np.sqrt(2*np.pi)*np.exp(-L*L/2.) * \
        (a1*K + a2*(K**2) + a3*(K**3) + a4*(K**4) + a5*(K**5))

    mask    = X<0
    w       = w * ~mask + (1.0-w)*mask  # This probably leads to a copy/identity,
                                        # Since we have mixed input-types.
    return w

def BS(CallPutFlag, S, X, T, r, v):
    """Black Sholes Function."""

    d1 = (np.log(S/X)+(r+v*v/2.)*T)/(v*np.sqrt(T))
    d2 = d1-v*np.sqrt(T)
    #print("d2 shape: " + str(d2.shape))
    if CallPutFlag=='c':
        return S*CND(d1)-X*np.exp(-r*T)*CND(d2)
    else:
        return X*np.exp(-r*T)*CND(-d2)-S*CND(-d1)

def price(S, I, flag='c', X=65.0, dT=(1.0/365.0), r=0.08, v=0.3, visualize=False):
    """Model price as a function of time."""

    T   = dT
    Ps  = []
    N   = len(S)
    for i in xrange (I):
        P = np.sum(BS(flag, S, X, T, r, v)) / N
        Ps.append(P)
        T += dT
        if visualize:   #NB: this is only for experiments
            np.visualize(P, "3d", 0, 0.0, 10)

    return Ps

def benchmark(N,I):
    B = util.Benchmark()
    ntimes = 10

    S = model(N, dtype=B.dtype)              # Construct pseudo-data
    R0 = price(S, I, visualize=False)   # Run the model

    elapsed = 0.0
    for i in range(ntimes):
        S = model(N, dtype=B.dtype)              # Construct pseudo-data
        #print("S shape: " + str(S.shape))
        B.start()
        R = price(S, I, visualize=False)   # Run the model
        B.stop()
        elapsed += B.elapsed()
    B.pprint()
    print ("Result: " + str(R[0]))
    print ("Elapsed (average over 10):" + str(elapsed/ntimes))

    R = R[0].item()

    return (elapsed/ntimes, R)

def main():
    B = util.Benchmark()
    (N, I) = B.size

    if B.inputfn:
        S = B.load_array()
    else:
        S = model(N, dtype=B.dtype)              # Construct pseudo-data

    if B.dumpinput:
        B.dump_arrays("black_scholes", {'input': S})

    B.start()
    R = price(S, I, visualize=B.visualize)   # Run the model
    B.stop()
    B.pprint()
    if B.outputfn:
        B.tofile(B.outputfn, {'res': np.array(R)})

    if B.verbose:
        print (R)

if __name__ == "__main__":
    main()

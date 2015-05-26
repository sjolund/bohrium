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

    a = np.array(np.random.random([N,N,N]), dtype=dtype)
    a = a*4.0-2.0 + 60.0 # Price is between 58-62

    np.random.seed(seed=2)

    b = np.array(np.random.random([N,N,N]), dtype=dtype)
    b = b*4.0+1

    return (a,b)

def common(a,b):
    g = 8.2
    res = ((a[:,1:,:]**2/b[:,1:,:] + g/2*b[:,1:,:]**3) -
            (a[:,:-1,:]**2/b[:,:-1,:] + g/2*b[:,:-1,:]**3))
    return res

def benchmark(N,I):
    B = util.Benchmark()
    ntimes = 10

    elapsed = 0.0
    for i in range(ntimes):
        (a,b) = model(N, dtype=B.dtype) # Construct pseudo-data
        B.start()
        R = common(a,b)   # Run the model
        B.stop()
        elapsed += B.elapsed()
    B.pprint()
    R = np.add.reduce(np.add.reduce(np.add.reduce(R)))
    print ("Result for N="+str(N)+": " + str(R))
    print ("Elapsed (average over 10):" + str(elapsed/ntimes))

    #if not np.isscalar(R):
    R = R.item()

    return (elapsed/ntimes, R)

def main():
    B = util.Benchmark()

    (N, I) = (1000,1)
    (a,b) = model(N, dtype=B.dtype) # Construct pseudo-data
    B.start()
    R = common(a,b)   # Run the model
    B.stop()
    print ("Result for N="+str(N)+": " + str(R))
    print ("Elapsed:" + str(B.elapsed()))


if __name__ == "__main__":
    main()

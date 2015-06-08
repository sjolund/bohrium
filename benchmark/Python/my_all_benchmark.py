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

    a = np.array(np.random.random([N,N]), dtype=dtype)
    a = a*10.0 # Price is between 58-62

    np.random.seed(seed=2)

    b = np.array(np.random.random([N,N]), dtype=dtype)
    b = b*10.0

    return (a,b)

def expression(a,b):
    res = (a[:,1:]**6*a[:,1:]**7/2 - 
            a[:,:-1]**6*a[:,:-1]**7/2)
    return res

def benchmark(N,I):
    B = util.Benchmark()
    ntimes = 10

    (a,b) = model(N, dtype=B.dtype) # Construct pseudo-data
    R0 = expression(a,b)   # Run the model

    elapsed = 0.0
    for i in range(ntimes):
        (a,b) = model(N, dtype=B.dtype) # Construct pseudo-data
        B.start()
        R = expression(a,b)   # Run the model
        B.stop()
        elapsed += B.elapsed()
    B.pprint()
    R = np.add.reduce(np.add.reduce(R))
    print ("Result for N="+str(N)+": " + str(R))
    print ("Elapsed (average over 10):" + str(elapsed/ntimes))

    #if not np.isscalar(R):
    R = R.item()

    return (elapsed/ntimes, R)

def main():
    B = util.Benchmark()

    (N, I) = (10,1)
    (a,b) = model(N, dtype=B.dtype) # Construct pseudo-data
    B.start()
    R = expression(a,b)   # Run the model
    B.stop()
    print ("Result for N="+str(N)+": " + str(R))
    print ("Elapsed:" + str(B.elapsed()))


if __name__ == "__main__":
    main()

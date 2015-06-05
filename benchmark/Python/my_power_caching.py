from __future__ import print_function
import util
"""if util.Benchmark().bohrium:
    import bohrium as np
else:
    import numpy as np"""
import bohrium as np

def model(N, dtype=np.float64):
    """Construct pseudo-data representing price samples?"""
    np.random.seed(seed=1)

    S = np.array(np.random.random([N,N]), dtype=dtype)
    S = S*10 # Price is between 58-62

    return S

def polynomial(K):
    (a1,a2,a3,a4,a5) = (0.31938153, -0.356563782, 1.781477937, -1.821255978, 1.330274429)
    #w = a1*K + a2*(K**2) + a3*(K**3) + a4*(K**4) + a5*(K**5)
    w = a1*K + a2*(K**2) + a3*(K**3) + a5*(K**5)
    #w = a1*K + a2*(K**2) + a3*(K**3) + a4*(K**4)
    #w = a1*K + a2*(K**2) + a3*(K**3)
    #w = a2*(K**2) + a3*(K**3)
    return w


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
        R = polynomial(S)   # Run the model
        B.stop()
        elapsed += B.elapsed()
    B.pprint()
    print ("Result: " + str(R[0]))
    print ("Elapsed (average over 10):" + str(elapsed/ntimes))

    R = R[0].item()

    return (elapsed/ntimes, R)

def main():
    B = util.Benchmark()
    N = 500

    S = model(N)
    B.start()
    R = polynomial(S)   # Run the model
    B.stop()
    B.pprint()
    #print ("Result: " + str(R[0]))
    print ("Elapsed (average over 10):" + str(B.elapsed()))
    B.stop()
    

if __name__ == "__main__":
    main()

import cphvbnumpy as np
import util

# Cumulative normal distribution
def CND(X):

    (a1,a2,a3,a4,a5) = (0.31938153, -0.356563782, 1.781477937, -1.821255978, 1.330274429)
    L = np.absolute(X)
    K = 1.0 / (1.0 + 0.2316419 * L)
    
    w = 1.0 - 1.0 / np.sqrt(2*np.pi)*np.exp(-L*L/2.) * \
        (a1*K + a2*(K**2) + a3*(K**3) + a4*(K**4) + a5*(K**5))
    
    mask    = X<0
    #print "mask",mask
    w       = w * ~mask + (1.0-w)*mask  # This probably leads to a copy/identity,
                                        # Since we have mixed input-types.
    
    return w

# Black Sholes Function
def BS(CallPutFlag,S,X,T,r,v):

    d1 = (np.log(S/X)+(r+v*v/2.)*T)/(v*np.sqrt(T))
    #print "d1",d1
    d2 = d1-v*np.sqrt(T)
    #print "d2", d2
    if CallPutFlag=='c':
        #print "BS",  S*CND(d1)-X,CND(d2)
        return S*CND(d1)-X*np.exp(-r*T)*CND(d2)
    else:
        return X*np.exp(-r*T)*CND(-d2)-S*CND(-d1)

def main():

    B       = util.Benchmark()
    N       = B.size[0]
    year    = B.size[1]

    S = np.ones([N], cphvb=B.cphvb)
    S = np.array([0.62352766, 0.68084793,  0.67529464, 0.28902324, 0.85378185], cphvb=B.cphvb)   #0.01675295, 0.21176588 , 0.8606528  , 0.17270649 , 0.89418568], cphvb=B.cphvb)
    
    S = S*4.0-2.0 + 60.0 # Price is 58-62
    
    #print "S'", S
    
    X   = 65.0
    r   = 0.08
    v   = 0.3
    day = 1.0/year
    T   = day

    B.start()
    for t in xrange(year):
        #print BS('c', S, X, T, r, v)
        np.sum(BS('c', S, X, T, r, v)) / N
        T += day
    B.stop()
    B.pprint()   

if __name__ == "__main__":
    main()

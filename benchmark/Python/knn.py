import cphvbnumpy as np
import util

def compute_targets(base, target):
    print "b", base
    print "t", target
    base    = base[:,np.newaxis]
    
    target  = target[:,:,np.newaxis]
    print "b", base
    print "t", target
    tmp = (base - target)**2
    tmp = np.add.reduce(tmp)
    print "tmp",tmp
    np.sqrt(tmp, tmp)
    print "tmp",tmp
    r  = np.max(tmp, axis=0)
    print "r",r
    return r

def main():
    B = util.Benchmark()
    ndims       = B.size[0]
    db_length   = B.size[1]

    targets = np.random.random((ndims,db_length), cphvb=B.cphvb)
    base    = np.random.random((ndims,db_length), cphvb=B.cphvb)

    B.start()
    compute_targets(base, targets)
    B.stop()
    B.pprint()

if __name__ == "__main__":
    main()


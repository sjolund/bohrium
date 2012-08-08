import numpy as np
import cphvbbridge
import time

type=np.float32

def jacobi_stencil_fixed(H,W,Dist):
    full = np.zeros((H+2,W+2), dtype=type)
    work = np.zeros((H,W), dtype=type)
    diff = np.zeros((H,W), dtype=type)
    tmpdelta = np.zeros((H), dtype=type)

    if Dist:
        cphvbbridge.handle_array(full)
        cphvbbridge.handle_array(work)
        cphvbbridge.handle_array(diff)
        cphvbbridge.handle_array(tmpdelta)

    cells = full[1:-1, 1:-1]
    up    = full[1:-1, 0:-2]
    left  = full[0:-2, 1:-1]
    right = full[2:  , 1:-1]
    down  = full[1:-1, 2:  ]

    full[:,0]  += -273.15
    full[:,-1] += -273.15
    full[0,:]  +=  40.0
    full[-1,:] += -273.13

    epsilon=W*H*0.002
    delta=epsilon+1
    i=0
    #while epsilon<delta:
    for i in xrange(4000):
      i+=1
      work[:] = cells
      work += up
      work += left
      work += right
      work += down
      work *= 0.2      
      cells[:] = work    
      # cells = (cells + up + left + right + down)*0.2
      
      
    return cells

def jacobi_sencil(H,W,Dist):
    full = np.zeros((H+2,W+2), dtype=type)
    work = np.zeros((H,W), dtype=type)
    diff = np.zeros((H,W), dtype=type)
    tmpdelta = np.zeros((H), dtype=type)

    if Dist:
        cphvbbridge.handle_array(full)
        cphvbbridge.handle_array(work)
        cphvbbridge.handle_array(diff)
        cphvbbridge.handle_array(tmpdelta)

    cells = full[1:-1, 1:-1]
    up    = full[1:-1, 0:-2]
    left  = full[0:-2, 1:-1]
    right = full[2:  , 1:-1]
    down  = full[1:-1, 2:  ]

    full[:,0]  += -273.15
    full[:,-1] += -273.15
    full[0,:]  +=  40.0
    full[-1,:] += -273.13

    epsilon=W*H*0.002
    delta=epsilon+1
    i=0
    #while epsilon<delta:
    for i in xrange(4000):
      i+=1
      work[:] = cells
      work += up
      work += left
      work += right
      work += down
      work *= 0.2
      np.subtract(cells,work,diff)
      diff = np.absolute(diff)
      np.add.reduce(diff, out=tmpdelta)
      delta = np.add.reduce(tmpdelta)
      cells[:] = work
      
    return cells

def run():
    #Seq = jacobi_sencil(20,20,False)
    #print Seq.shape
    #Par = jacobi_sencil(4000,4000,True)
    Par = jacobi_stencil_fixed(1000,1000,True)

    #if not numpytest.array_equal(Seq,Par):
    #    raise Exception("Uncorrect result matrix\n")

if __name__ == "__main__":
    s = time.clock()
    run()
    print time.clock() - s

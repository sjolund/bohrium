import numpy as np
import time
import sys

H = W = int(sys.argv[1])
I = int(sys.argv[2])

full        = np.empty((H+2,W+2),   dtype=np.float64)
work        = np.empty((H+2,W+2),   dtype=np.float64)

full[:]     = np.arange((H+2)*(W+2), dtype=np.float64).reshape((H+2,W+2))

start=time.time()
for i in xrange(I):
    work[1:-1, 1:-1]  = full[1:-1, 1:-1]
    work[1:-1, 1:-1] += full[1:-1, 0:-2]
    work[1:-1, 1:-1] += full[1:-1, 2:  ] 
    work[1:-1, 1:-1] += full[0:-2, 1:-1]  
    work[1:-1, 1:-1] += full[2:  , 1:-1]
    work[1:-1, 1:-1] *= 0.2
    temp=work; work=full; full=temp
stop=time.time()
print stop-start

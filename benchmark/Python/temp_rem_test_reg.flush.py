import cphvbnumpy as np
import cphvbbridge
import util

B = util.Benchmark()
H = B.size[0]
W = B.size[1]
I = B.size[2]

a = np.ones((H,W),       dtype=B.dtype, cphvb=B.cphvb)
b = np.ones((H,W),       dtype=B.dtype, cphvb=B.cphvb)
c = np.ones((H,W),       dtype=B.dtype, cphvb=B.cphvb)
d = np.ones((H,W),       dtype=B.dtype, cphvb=B.cphvb)
e = np.ones((H,W),       dtype=B.dtype, cphvb=B.cphvb)

B.start()
for i in xrange(I):
    cphvbbridge.flush()
    a = a + b + c + d + e            
B.stop()
B.pprint()



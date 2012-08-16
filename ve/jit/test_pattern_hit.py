import numpy as np
#import numpytest
import cphvbbridge

type=np.float32
def test_pattern_001(H,W):
    print "- test_pattern_001()";
    work = np.zeros((H,W), dtype=type)    
    ones = np.ones((H,W),dtype=type)    
    cphvbbridge.handle_array(work)    
    cphvbbridge.handle_array(ones)      
    
    work += ones    
    work -= ones
    work *= 0.2 
    return work,ones
    
    
def test_pattern_002(H,W):     
    A = np.ones((H,W),dtype=type)
    B = np.ones((H,W),dtype=type)
    C = np.ones((H,W),dtype=type);
    D = np.ones((H,W),dtype=type);
    B = B*2;
    D = D*3
    
    cphvbbridge.handle_array(A)
    cphvbbridge.handle_array(B)
    cphvbbridge.handle_array(C)
    cphvbbridge.handle_array(D)
        
    
    A = A + B + C
    C = B + B * 2
    
    return (A,B)





def test_pattern_miss_b(H,W):
    print "- Third pattern mis";
    work = np.zeros((H,W), dtype=type)    
    ones = np.ones((H,W),dtype=type)    
    cphvbbridge.handle_array(work)    
    cphvbbridge.handle_array(ones)      
    
    work += ones
    work += ones
    work -= ones
    #work += ones
    #work -= ones
    work *= 0.2

def test_pattern_miss_a(H,W):
    print "- Second pattern mis";
    work = np.zeros((H,W), dtype=type)    
    ones = np.ones((H,W),dtype=type)    
    cphvbbridge.handle_array(work)    
    cphvbbridge.handle_array(ones)      
    
    work += ones    
    work -= ones
    #work += ones
    #work -= ones
    work *= 0.2    

def test_pattern_miss_c(H,W):
    print "- No pattern match";
    work = np.zeros((H,W), dtype=type)    
    ones = np.ones((H,W),dtype=type)    
    cphvbbridge.handle_array(work)    
    cphvbbridge.handle_array(ones)      
        
    work -= ones
    #work += ones
    #work -= ones
    work *= 0.2     
    

def test_pattern_hit_a():
    H,W = 5,5
    work = np.zeros((H,W), dtype=type)    
    ones = np.ones((H,W),dtype=type)    
    cphvbbridge.handle_array(work)    
    cphvbbridge.handle_array(ones)        

        
    work += ones
    work += ones
    work += ones
    work += ones    
    work *= 0.2
    #print work
    #print ones


def test_pattern_hit_b():
    H,W = 5,5
    work = np.zeros((H,W), dtype=type)    
    ones = np.ones((H,W),dtype=type)
    
    cphvbbridge.handle_array(work)    
    cphvbbridge.handle_array(ones)        

    for i in xrange(5):
        work += ones
        work += ones
        work += ones
        work += ones
        work *= 0.2

def testing_ast():
    H,W = 5,5
    A = np.ones((H,W), dtype=type)
    A*=5;    
    B = np.ones((H,W),dtype=type)
    #C = np.float32(np.random.rand(H,W))
    #print A,B
    
    C = np.ones((H,W),dtype=type) * 2.5
    
    cphvbbridge.handle_array(A)
    cphvbbridge.handle_array(B)
    cphvbbridge.handle_array(C)
        
    for i in range(2):
        C += (B + A)*2
    
    #A = B - C + B
    #A += B
    #A += C
    #A += B
    return C

def test_slice():
    H,W = 5,5
    A = np.ones((H,W), dtype=type)
    A*=5;    
    B = np.ones((H,W),dtype=type)

    cphvbbridge.handle_array(A)
    cphvbbridge.handle_array(B)
    
    H = A[:,1:] + B[:,1:]
    G = A+B

def test_001():
    H,W = 5,5    
    A = np.ones((H,W),dtype=type)
    B = np.ones((H,W),dtype=type)
    C = np.ones((H,W),dtype=type);
    D = np.ones((H,W),dtype=type);
    B = B*2;
    D = D*3
    
    cphvbbridge.handle_array(A)
    cphvbbridge.handle_array(B)
    cphvbbridge.handle_array(C)
    cphvbbridge.handle_array(D)
        
    
    A = -A
    A = -A
    
    #A += C
    #A += D
    return A
    
    # what happens when two identical constants are used in and expression?
    
    #print A
type = np.float32
def jacobi_stencil(H,W,Dist,elems=2):
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
    #for i in xrange(400):
    for i in xrange(elems):
      i+=1
      work[:] = cells
      work += up
      work += left
      work += right
      work += down
      work *= 0.2
      
      # alternative writing:      
      #~ work[:] = (cells + up + left + right + down) * 0.2
      
      #~ np.subtract(cells,work,diff)
      #~ diff = np.absolute(diff)
      #~ np.add.reduce(diff, out=tmpdelta)
      #~ delta = np.add.reduce(tmpdelta)
      cells[:] = work
      
    return cells

if __name__ == "__main__":
    
    # get arguments
    from optparse import OptionParser
    
    parser = OptionParser()
    (options, args) = parser.parse_args()
    
    do_sten_fix = 0
    do_sten = 0    
    do_num = 0
    
    for i in range(len(args)):
        arg = args[i]        
        if arg == "do":
            if len(args) > i+1:
                try:                
                    do_num = int(args[i+1]);
                except:
                    print "ERROR! : In 'do n' , n must be a number!"
                    exit(0)
        
        
    #print args
    #print "do_num",do_num
    
    
    if do_num == 1:
        test_pattern_001(5,5)
    if do_num == 2:
        test_pattern_002(5,5)
    #test_pattern_hit(5,5)
    
    #test_pattern_miss_a(5,5)
    #test_pattern_miss_b(5,5)
    #test_pattern_miss_c(5,5)
    
    #K = test_001()
    
    #jacobi_stencil(5,5,True);
    #test_slice()
    #test_pattern_hit_a()
    

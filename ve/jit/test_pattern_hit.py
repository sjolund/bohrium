import numpy as np
#import numpytest
import cphvbbridge

type=np.float32

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
    B*2;
    
    cphvbbridge.handle_array(A)
    cphvbbridge.handle_array(B)
    cphvbbridge.handle_array(C)
        
    A = B + C + A
    
    # what happens when two identical constants are used in and expression?
    
    #print A

if __name__ == "__main__":
    #test_pattern_hit(5,5)
    
    #test_pattern_miss_a(5,5)
    #test_pattern_miss_b(5,5)
    #test_pattern_miss_c(5,5)
    
    test_001()
    #test_slice()
    #test_pattern_hit_a()
    

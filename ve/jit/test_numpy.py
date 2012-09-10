# python testing cphVB.

import numpy as np
#import numpytest
import cphvbbridge

type=np.float32
def test_pattern_000(H,W):
    cphvbbridge.flush()
    print ''
    print "- test_pattern_000()";
    
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)    
    C = np.ones((H,W), dtype=type)    
    A = A * 3;
    B = B * 2
    
    cphvbbridge.handle_array(C)    
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)   
        
    A[0,1]
    
def test_pattern_001(H,W):
    cphvbbridge.flush()
    print ''
    print "- test_pattern_001()";    
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)    
    C = np.ones((H,W), dtype=type)    
    A = A * 3;
    B = B * 2
    
    cphvbbridge.handle_array(C)    
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)   
    
    t = A + B
    C[0,1]
    

def test_pattern_002(H,W):
    cphvbbridge.flush()
    print ''
    print "- test_pattern_002()";    
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)    
    C = np.ones((H,W), dtype=type)    
    A = A * 3;
    B = B * 2
    
    cphvbbridge.handle_array(C)    
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)   
    
    A += B   
    A[0,1]
    
def test_pattern_003(H,W):
    cphvbbridge.flush()
    print ''
    print "- test_pattern_003()";
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)    
    C = np.ones((H,W), dtype=type)    
    A = A * 3;
    B = B * 2
    
    cphvbbridge.handle_array(C)    
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)   
    
    A = A + B   
    A[0,1]
    
    
def test_pattern_004(H,W):
    cphvbbridge.flush()
    print ''
    print "- test_pattern_004()";
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)    
    C = np.ones((H,W), dtype=type)    
    A = A * 3;
    B = B * 2
    
    cphvbbridge.handle_array(C)    
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)   
    
    A = A + B + C   
    A[0,1]
    
def test_pattern_005(H,W):
    cphvbbridge.flush()
    print ''
    print "- test_pattern_005()";
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)    
    C = np.ones((H,W), dtype=type)    
    A = A * 3;
    B = B * 2
    
    cphvbbridge.handle_array(C)    
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)   
    
    A += B + C       
    A[0,1]        

def test_pattern_006(H,W):
    cphvbbridge.flush()
    print ''
    print "- test_pattern_006()";
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)    
    C = np.ones((H,W), dtype=type)    
    A = A * 3;
    B = B * 2
    
    cphvbbridge.handle_array(C)    
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)   
    
    A += B 
    A += C
    A[0,1] 
    
def test_pattern_007(H,W):
    cphvbbridge.flush()
    print ''
    print "- test_pattern_007()";    
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)    
    C = np.ones((H,W), dtype=type)    
    A = A * 3;
    B = B * 2
    
    cphvbbridge.handle_array(C)    
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)   
    
    A = A + B 
    A = A + C
    A[0,1] 

def test_pattern_008(H,W):
    cphvbbridge.flush()
    print ''
    print "- test_pattern_008()";
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)    
    C = np.ones((H,W), dtype=type)    
    A = A * 3;
    B = B * 2
    
    cphvbbridge.handle_array(C)    
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)   
    
    A = A + B 
    print C
    

if __name__ == "__main__":
    #~ test_pattern_000(1,5)     
    #~ exit(0)
    #~ 
    #~ 
    test_pattern_001(1,5)    
    #~ test_pattern_002(1,5)   
    #~ test_pattern_003(1,5)
    #~ 
    #~ # tests of different A + B + C methods
    #~ test_pattern_004(1,5)
    #~ test_pattern_005(1,5)
    #~ test_pattern_006(1,5)
    #~ test_pattern_007(1,5)
    #~ 
    #~ # effect of print statement
    #~ test_pattern_008(1,5)
    #~ 
    #~ 
    
    

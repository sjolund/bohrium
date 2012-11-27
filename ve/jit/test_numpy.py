# python testing cphVB.

import numpy as np
import time
#import numpytest
try:
    import cphvbbridge
except:
    None

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
    type=np.float64
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)    
    #C = np.ones((H,W), dtype=type)    
    A = A * 3;
    B = B * 2
    
    #cphvbbridge.handle_array(C)    
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)   
    
    C = A + B
    print C
    
    

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
    A[:,1] = 0
    B = B * 2
    
    cphvbbridge.handle_array(C)    
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)   
        
    A = A + B + C
    
    A[0,1]
    print A
    
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
    print 'Test of re-assignment of full array'
    
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
    
    
def test_pattern_009(H,W):
    cphvbbridge.flush()
    print ''
    print "- test_pattern_009()";
    
    #A = np.ones((H,W),dtype=type)
    A = np.ones((H,W),dtype=np.float64)    
    B = np.ones((H,W),dtype=type)    
    C = np.ones((H,W),dtype=type)    
    A = A * 3;
    B = B * 2
        
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)   
    cphvbbridge.handle_array(C)
    
    # Aa = A[0,1:3] + 2
    
    #~ A[0,1:3] = B[0,1:3] + C[0,1:3]
    #~ # Aa = A[0,1:3] + 2
    #~ A[0,3:5] = B[0,1:3] + 2
    #~ A[0,1:3] = A[0,1:3] + 8            
    #~ print A[0,1:3]
    #~ 
    
    # ---------------------------
    #~ A = 3
    #~ Aa = A[0,1:3] + 2
    #~ A[0,1:3] = 0
    #~ print Aa     
    # Aa = 5, not 0.
    # Aa thus hold as copy of A[0,1:3] + 2
    # results of a operations is thus a copy in Numpy-land
    # ---------------------------
        
    #~ A[0,2:4] = A[0,1:3] + 2
    #~ A[0,1:3] = 0
    #~ print A
    #~ 
    #~ 
    #~ A[0,2:4] = A[0,2:4] + 2
    #~ #A[0,2:4] += 2
    #~ A[0,3:5] = 0
    #~ print A
    #~ 
    #~ 

    
    # these will from the nametable view be destinct. Need to be able to handle this.
    # A[0,2:4] = A[0,2:4] + 2
    #A += 1
    
    #Aa = A[0,1:3] + A[0,2:4] + 10
    
    #A[0,0:2] = A[0,2:4] + C[0,2:4]
    
    #A[0,0:2] = A[0,0:2] + C[0,2:4]    
    A[0,1:3] = A[0,0:2] + C[0,2:4]    

    AA = A[0,0:2] + 2
    
    #~ A[0,3:5] = 0 # has no effect on Ba, but on A
    #~ B[0,2:4] = B[0,1:3] + Aa 
    #~ 
    #~ A = A + B
    #~


    #A[0,1]
    AA[0]
    
    #~ # assignments to the base array is done on the base array (the same view) as one would expect.
    

def test_pattern_010(H,W):
    cphvbbridge.flush()
    print ''
    print "- test_pattern_010()";
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)    
    C = np.ones((H,W),dtype=type)    
    A = A * 3;
    B = B * 2
        
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)   
    cphvbbridge.handle_array(C)

    BB = A[0,0:2] + 2
    BB = A[0,1:3] + 2
    BB = A[0,2:4] + 2

def test_pattern_011(H,W):
    cphvbbridge.flush()
    print ''
    print "- test_pattern_011()";
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)    
    C = np.ones((H,W),dtype=type)    
    A = A * 3;
    B = B * 2
        
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)   
    cphvbbridge.handle_array(C)

    A[0,0:2] = A[0,0:2] + B[0,1:3]
    A[0,0:2] = A[0,0:2] + B[0,2:4]
    A[0,0:2] = A[0,0:2] + B[0,0:2]

    #~ A[0,0:2] += B[0,1:3]
    #~ A[0,0:2] += B[0,2:4]
    #~ A[0,0:2] += B[0,0:2]

    # A[0,0:2] = A[0,0:2] + B[0,1:3] + B[0,2:4] + B[0,0:2]    

    #~ A[0,0:2] = A[0,0:2] + B[0,1:3]
    #~ A[0,0:2] = A[0,0:2] + B[0,2:4]
    A[0,1]

def test_pattern_012(H,W):
    cphvbbridge.flush()
    print ''
    print "- test_pattern_012()";
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)    
    C = np.ones((H,W),dtype=type)    
    A = A * 3;
    B = B * 2
        
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)   
    cphvbbridge.handle_array(C)

    
    A[0,0:2] = B[0,1:3]
    A[0,2]





def test_pattern_013(H,W):
    cphvbbridge.flush()
    print ''
    print "- test_pattern_013()";
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)    
    C = np.ones((H,W),dtype=type)    
    A = A * 3;
    B = B * 2
        
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)   
    cphvbbridge.handle_array(C)

    
    A[0,0:2] = B[0,1:3] + 2
    A[0,1]    

def test_pattern_014(H,W):
    cphvbbridge.flush()
    print ''
    print "- test_pattern_014()";
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)    
    C = np.ones((H,W),dtype=type)    
    A = A * 3
    B = B * 2
    C = C * 2
    
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)   
    cphvbbridge.handle_array(C)

    #s = time.time()    
    #K1 = (A + B + C + A + B + C + A + B + C + A + B + C + A + B + C + A + B + C + A + B + C + A + B + C + A + B + C + B + C + A + B + C )
    K1 = (A * B / C) +  (A * B / C) + (A * B / C) + (A * B / C)+ (A * B / C)+ (A * B / C)
    #K1 += B
    #K1 += C
    print K1[1,0]
    #print s - time.time()


def test_pattern_015(H,W):
    cphvbbridge.flush()
    print ''
    print "- test_pattern_015()";
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)
    C = np.ones((H,W),dtype=type)
    
    A = A * 3;
    B = B * 2
        
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)   
    cphvbbridge.handle_array(C)
    
    B += 10 + A    
    K = B + 2
    print K
    


def test_pattern_016(H,W):
    cphvbbridge.flush()
    print ''
    print "- test_pattern_016()";
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)
    C = np.ones((H,W),dtype=type)
    
    A = A * 3;
    B = B * 2
        
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)   
    cphvbbridge.handle_array(C)

    #B = 5 + A
    B = A + 5
    B[0,1]
    print B

def test_pattern_017(H,W):
    cphvbbridge.flush()
    print ''
    print "- test_pattern_017()";
    
    A = np.ones((H,W),dtype=type)    
    A = A * 3;
    B = np.ones((H,W),dtype=type)
    B = B * 2
    C = np.ones((H,W),dtype=type)
            
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)   
    cphvbbridge.handle_array(C)

    A = A + 10
    D = A + B
    D[0,1]
    E = A + 2
    print E


def test_pattern_018(H,W):
    cphvbbridge.flush()
    print 'simple assign'
    print "- test_pattern_018()";
    
    A = np.ones((H,W),dtype=type)    
    A = A * 3;
    B = np.ones((H,W),dtype=type)
    B = B * 2
    C = np.ones((H,W),dtype=type)
    
    cphvbbridge.handle_array(C)            
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)       
    A[:] = B
    B[:] = C
    print B


# ================================================

def test_pattern_101(H,W):
    cphvbbridge.flush()
    print 'testing double dto'
    print "- test_pattern_101()";
    # exelist = [3,]
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)    
    C = np.ones((H,W),dtype=type)    
    A = A * 3;
    B = B * 2
        
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)   
    cphvbbridge.handle_array(C)

    A += 2 + B
    B = A + C    
    A += 1
       
    A[0,1]

# max expr DONs
# good test for dependency and pre-execution list.
def test_pattern_102(H,W):
    cphvbbridge.flush()
    print 'testing inline BoundedArray update'
    print "- test_pattern_102()";
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)
    C = np.ones((H,W),dtype=type)
    
    A = A * 3;
    B = B * 2
        
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)   
    cphvbbridge.handle_array(C)

    C += 10    
    C[0,1:3] = A[0,0:2] + B[0,2:4]
    Cc = C + B
    Cc[0,1]


def test_pattern_103(H,W):
    cphvbbridge.flush()
    print ''
    print "- test_pattern_103()";
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)
    C = np.ones((H,W),dtype=type)
    
    A = A * 3;
    B = B * 2
        
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)   
    cphvbbridge.handle_array(C)
    
    B += 10 + A    
    K = B + 2     
    B[0,1:3] += 2
    C += B + 10
    C[0,1:3] = A[0,0:2] + B[0,2:4]
    Cc = C + B
    print Cc
        #~ 
    #~ Cc[0,1]
    #~ print Cc # [[ 41.  37.  35.  41.  41.]]
    #~ 
    #K[]

    #B += 10 + A                        B=[15,15,15,15,15]
    #K = B+2                            k=[17,17,17,17,17]
    #B[0,1:3] +=2                       B=[15,17,17,15,15]
    #C += B + 10                        C=[26,28,28,26,26]
    #C[0,1:3] = A[0,0:2] + B[0,2:4]     C=[26,20,18,26,26] << [26,0 , 0,26,26] + 3,3 + 17,15
    
    #Cc = C + B                         C=[41,37,35,41,41]

    
    # Execute list:
    #  B1 in l1, tdto = 2 (3: p[4,5]) 4 or 5 not reached from Cc expression tdto +1 (6 and 5 are the same. and identity is used)
    #  B2 in l3, tdto = 2, (6: DTO [7,11,]), 7 should remain after dto erase, since only 11 is reachable from expr.  7 is not. #11's child expr is #3.
    #  C1 in l4
    #  C2 in l5


    # [B1,B2,C1,C2] -> Cc  == [3,6,9,13] -> 14
    # B1 since B is used by K. Unrelated Use (to the expr). tdto > 0 after expr-parent use removal
    # B2 since B is update Out of Expression. Array shape different.
    # C1 since C is used by C[]. Unrelated use.
    # C2 since C is update Out of Expression. Array share different
    # before Cc can be computed in l6


    # B[0,2:4] should not be set as a update (added to the usage table) on the base array, since it is only read.
    # if a array with a base array is defined, and the base array already exist, do not add its pressent to the usage list!


def test_pattern_104(H,W):
    cphvbbridge.flush()
    print 'long distance dependency'
    print "- test_pattern_104()";
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)
    C = np.ones((H,W),dtype=type)
    
    A = A * 3;
    B = B * 2
        
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)   
    cphvbbridge.handle_array(C)
    
    A += 10
    A[0,1:3] = B[0,1:3] / 5    
    C = 3*B + (B-C)/3
    A = A + C*2
    A[0,1]

        
def test_pattern_105(H,W):
    cphvbbridge.flush()
    print 'long distance dependency'
    print "- test_pattern_104()";
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)    
    
    A = A * 3;
    B = B * 2
        
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)   
    
    T = A + B
    T[1,:] = 7
    T = T + 2
    print T
    



def test_pattern_200(H,W):
    cphvbbridge.flush()
    print 'loops'
    print "- test_pattern_200()";
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)
    C = np.ones((H,W),dtype=type)
    
    A = A * 3;
    B = B * 2
        
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)   
    cphvbbridge.handle_array(C)

    for i in xrange(4):
        C += A + B
    print C[0,1]



def test_pattern_201(H,W):
    cphvbbridge.flush()
    print 'loop with sync'
    print "- test_pattern_201()";
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)
    C = np.ones((H,W),dtype=type)
    
    A = A * 3;
    B = B * 2
        
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)   
    cphvbbridge.handle_array(C)

    for i in xrange(10):
        C += A + B
        C[0,2]  
    print C



def test_pattern_300(H,W):
    cphvbbridge.flush()
    print 'reduce userfunc'
    print "- test_pattern_300()";
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)    
    
    A = A * 4;
    B = B * 2
        
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)       

    h = np.add.reduce(A)
    t = np.add.reduce(h)
    print t
    A = B * t
    print A


def test_pattern_301(H,W):
    cphvbbridge.flush()
    print 'reduce userfunc'
    print "- test_pattern_301()";
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)    
    
    A = A * 4;
    B = B * 2
    A[0,:] = [1,2,3,4,5]
    cphvbbridge.handle_array(A)      
    cphvbbridge.handle_array(B)       
    #print A    
    #h = np.add.reduce(A,1)
    
    h = np.add.reduce(A,1)
    print h 
    t = np.add.reduce(h)
    print "t",t
    
   


def test_pattern_400(H,W):
    cphvbbridge.flush()    
    #print "- test_pattern_400()";    
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)        
    cphvbbridge.handle_array(A)                
    cphvbbridge.handle_array(B)
    
    cphvbbridge.flush()
    
    start = time.time()    
    T = A + B
    T[0,0]    
    #cphvbbridge.flush()    
    print "execution. Time: {0}".format(time.time()-start)

def test_pattern_401(H,W):
    cphvbbridge.flush()    
    #print "- test_pattern_401()";    
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)    
    C = np.ones((H,W),dtype=type)
   
    cphvbbridge.handle_array(A)                
    cphvbbridge.handle_array(B)
    cphvbbridge.handle_array(C)
  
    cphvbbridge.flush()
    
    start = time.time()    
    T = A + B + C
    T[0,0]    
    #cphvbbridge.flush()    
    print "execution. Time: {0}".format(time.time()-start)

def test_pattern_402(H,W):
    cphvbbridge.flush()    
    #print "- test_pattern_402()";    
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)    
    C = np.ones((H,W),dtype=type)
    D = np.ones((H,W),dtype=type)
    
    cphvbbridge.handle_array(A)                
    cphvbbridge.handle_array(B)
    cphvbbridge.handle_array(C)
    cphvbbridge.handle_array(D)    
    cphvbbridge.flush()
    
    start = time.time()    
    T = A + B + C + D 
    T[0,0]    
    #cphvbbridge.flush()    
    print "execution. Time: {0}".format(time.time()-start)

def test_pattern_403(H,W):
    cphvbbridge.flush()
   
    #print "- test_pattern_403()";    
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)    
    C = np.ones((H,W),dtype=type)
    D = np.ones((H,W),dtype=type)
    E = np.ones((H,W),dtype=type)    
    A+=1
    B+=2
    C+=3
    D+=4
    E+=5    
    cphvbbridge.handle_array(A)                
    cphvbbridge.handle_array(B)
    cphvbbridge.handle_array(C)
    cphvbbridge.handle_array(D)
    cphvbbridge.handle_array(E)    
    cphvbbridge.flush()
    
    start = time.time()    
    T = A + B + C + D + E
    T[0,0]    
    #cphvbbridge.flush()    
    print "execution. Time: {0}".format(time.time()-start)
    
def test_pattern_404(H,W):
    cphvbbridge.flush()    
    #print "- test_pattern_404()";    
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)    
    C = np.ones((H,W),dtype=type)
    D = np.ones((H,W),dtype=type)
    E = np.ones((H,W),dtype=type)
    F = np.ones((H,W),dtype=type)        
    A+=1
    B+=2
    C+=3
    D+=4
    E+=5
    F+=6
    cphvbbridge.handle_array(A)                
    cphvbbridge.handle_array(B)
    cphvbbridge.handle_array(C)
    cphvbbridge.handle_array(D)
    cphvbbridge.handle_array(E)
    cphvbbridge.handle_array(F)
    cphvbbridge.flush()
    
    start = time.time()    
    T = A + B + C + D + E + F
    T[0,0]    
    #cphvbbridge.flush()    
    print "execution. Time: {0}".format(time.time()-start)
    
def test_pattern_405(H,W):
    cphvbbridge.flush()    
    #print "- test_pattern_405()";    
    
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)    
    C = np.ones((H,W),dtype=type)
    D = np.ones((H,W),dtype=type)
    E = np.ones((H,W),dtype=type)
    F = np.ones((H,W),dtype=type)
    G = np.ones((H,W),dtype=type)    
    A+=1
    B+=2
    C+=3
    D+=4
    E+=5
    F+=6
    G+=7
    cphvbbridge.handle_array(A)                
    cphvbbridge.handle_array(B)
    cphvbbridge.handle_array(C)
    cphvbbridge.handle_array(D)
    cphvbbridge.handle_array(E)
    cphvbbridge.handle_array(F)
    cphvbbridge.handle_array(G)
    cphvbbridge.flush()
    
    start = time.time()    
    T = A + B + C + D + E + F + G
    T[0,0]    
    #cphvbbridge.flush()    
    print "execution. Time: {0}".format(time.time()-start)

def test_pattern_501(H,W):
    cphvbbridge.flush()    
    type=np.float32
    A = np.ones((H,W),dtype=type)    
    B = np.ones((H,W),dtype=type)    
    C = np.ones((H,W),dtype=type)

    type=np.float64
    D = np.ones((H,W),dtype=type)
    E = np.ones((H,W),dtype=type)
    F = np.ones((H,W),dtype=type)        

    A+=1
    B+=2
    C+=3
    D+=4
    E+=5
    F+=6
    cphvbbridge.handle_array(A)                
    cphvbbridge.handle_array(B)
    cphvbbridge.handle_array(C)
    cphvbbridge.handle_array(D)
    cphvbbridge.handle_array(E)
    cphvbbridge.handle_array(F)    
    cphvbbridge.flush()

    T1 = A+B+C
    cphvbbridge.flush()
    T2 = D+E+F
    cphvbbridge.flush()
    print T2
    
#delta = np.add.reduce(diff)
#delta = np.add.reduce(delta)


if __name__ == "__main__":
    #~ test_pattern_000(1,5)     
    #~ exit(0)


    # A + B
    H = 6000
    W = 6000
    #test_pattern_400(H,W)
    #test_pattern_401(H,W)
    #test_pattern_402(H,W)
    #test_pattern_403(H,W)    
    #test_pattern_404(H,W)    
    #test_pattern_405(H,W)
   
    #test_pattern_301(2,5)
    test_pattern_018(2,6)
    #test_pattern_016(2,5)  
    #test_pattern_017(2,5)
    
    # PROBLEM WITH THIS TEST!
    #test_pattern_015(1,5)
    #test_pattern_300(2,5)
    #print "hello world"

    #test_pattern_101(2,5)
     
    #test_pattern_102(1,5)

    #test_pattern_103(2,5)

    # testing double dto epxression
    #test_pattern_101(1,5)

    # testing inline update to boundedArray
    # [1,5,7]
    #test_pattern_102(1,5)

    # testing double inline update to boundedArray
    # [3,4,6,9,13,14,]
    #test_pattern_103(2,5)
    #test_pattern_105(2,5)
    
    #test_pattern_104(1,5)
    #test_pattern_501(2,5)    
    
    #test_pattern_200(2,5)
    #test_pattern_201(1,5)
    
    #~ s = time.time()    
    #~ #test_pattern_014(10000,3000)
    #~ test_pattern_103(10000,3000)
    #~ print time.time()-s
    

    #-0.0366699695587
    #-0.0363841056824
    #-0.0370109081268
    #-0.0359990596771

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


    #test_pattern_300(1,5)
    #~ doing test_pattern 14 (10,10)
    #~ simple    
    #~ -0.000265121459961
    #~ -0.000275850296021
    #~ 
    #~ jit - with simple
    #~ -0.000304222106934
    #~ -0.000330924987793

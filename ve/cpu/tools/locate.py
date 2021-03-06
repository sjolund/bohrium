from __future__ import print_function
## 3D Lattice Boltzmann (BGK) model of a fluid.
## D3Q19 model. At each timestep, particle densities propagate
## outwards in the directions indicated in the figure. An
## equivalent 'equilibrium' density is found, and the densities
## relax towards that state, in a proportion governed by omega.
##               Iain Haslam, March 2006.
import util
if util.Benchmark().bohrium:
    import bohrium as np
else:
    import numpy as np

def main():
    B = util.Benchmark()

    nx      = B.size[0]
    ny      = B.size[1]
    nz      = B.size[2]
    ITER    = B.size[3]

    NO_OBST = 1
    omega   = 1.0
    density = 1.0
    deltaU  = 1e-7
    t1      = 1/3.0
    t2      = 1/18.0
    t3      = 1/36.0

    B.start()
    F       = np.ones((19, nx, ny, nz), dtype=np.float64)
    F[:]    = density/19.0
    FEQ     = np.ones((19, nx, ny, nz), dtype=np.float64)
    FEQ[:]  = density/19.0
    T       = np.zeros((19, nx, ny, nz), dtype=np.float64)

    #Create the scenery.
    BOUND   = np.zeros((nx, ny, nz), dtype=np.float64)
    BOUNDi  = np.ones((nx, ny, nz), dtype=np.float64)
    """
    if not NO_OBST:
        for i in xrange(nx):
            for j in xrange(ny):
                for k in xrange(nz):
                    if ((i-4)**2+(j-5)**2+(k-6)**2) < 6:
                        BOUND[i,j,k] += 1.0
                        BOUNDi[i,j,k] += 0.0
    BOUND[:,0,:]    += 1.0
    BOUNDi[:,0,:]   *= 0.0
    """

    if util.Benchmark().bohrium:
        np.flush()
    for ts in xrange(0, ITER):

        ##Propagate / Streaming step
        T[:] = F
        #nearest-neighbours
        F[1,:,:,0]   = T[1,:,:,-1]
        F[1,:,:,1:]  = T[1,:,:,:-1]
        F[2,:,:,:-1] = T[2,:,:,1:]
        F[2,:,:,-1]  = T[2,:,:,0]
        F[3,:,0,:]   = T[3,:,-1,:]
        F[3,:,1:,:]  = T[3,:,:-1,:]
        F[4,:,:-1,:] = T[4,:,1:,:]
        F[4,:,-1,:]  = T[4,:,0,:]
        F[5,0,:,:]   = T[5,-1,:,:]
        F[5,1:,:,:]  = T[5,:-1,:,:]
        F[6,:-1,:,:] = T[6,1:,:,:]
        F[6,-1,:,:]  = T[6,0,:,:]
        #next-nearest neighbours
        F[7,0 ,0 ,:] = T[7,-1 , -1,:]
        F[7,0 ,1:,:] = T[7,-1 ,:-1,:]
        F[7,1:,0 ,:] = T[7,:-1, -1,:]
        F[7,1:,1:,:] = T[7,:-1,:-1,:]

        F[8,0 ,:-1,:] = T[8,-1 ,1:,:]
        F[8,0 , -1,:] = T[8,-1 ,0 ,:]
        F[8,1:,:-1,:] = T[8,:-1,1:,:]
        F[8,1:, -1,:] = T[8,:-1,0 ,:]

        F[9,:-1,0 ,:] = T[9,1:, -1,:]
        F[9,:-1,1:,:] = T[9,1:,:-1,:]
        F[9,-1 ,0 ,:] = T[9,0 ,  0,:]
        F[9,-1 ,1:,:] = T[9,0 ,:-1,:]

        F[10,:-1,:-1,:] = T[10,1:,1:,:]
        F[10,:-1, -1,:] = T[10,1:,0 ,:]
        F[10,-1 ,:-1,:] = T[10,0 ,1:,:]
        F[10,-1 , -1,:] = T[10,0 ,0 ,:]

        F[11,0 ,:,0 ] = T[11,0  ,:, -1]
        F[11,0 ,:,1:] = T[11,0  ,:,:-1]
        F[11,1:,:,0 ] = T[11,:-1,:, -1]
        F[11,1:,:,1:] = T[11,:-1,:,:-1]

        F[12,0 ,:,:-1] = T[12, -1,:,1:]
        F[12,0 ,:, -1] = T[12, -1,:,0 ]
        F[12,1:,:,:-1] = T[12,:-1,:,1:]
        F[12,1:,:, -1] = T[12,:-1,:,0 ]

        F[13,:-1,:,0 ] = T[13,1:,:, -1]
        F[13,:-1,:,1:] = T[13,1:,:,:-1]
        F[13, -1,:,0 ] = T[13,0 ,:, -1]
        F[13, -1,:,1:] = T[13,0 ,:,:-1]

        F[14,:-1,:,:-1] = T[14,1:,:,1:]
        F[14,:-1,:, -1] = T[14,1:,:,0 ]
        F[14,-1 ,:,:-1] = T[14,0 ,:,1:]
        F[14,-1 ,:, -1] = T[14,0 ,:,0 ]

        F[15,:,0 ,0 ] = T[15,:, -1, -1]
        F[15,:,0 ,1:] = T[15,:, -1,:-1]
        F[15,:,1:,0 ] = T[15,:,:-1, -1]
        F[15,:,1:,1:] = T[15,:,:-1,:-1]

        F[16,:,0 ,:-1] = T[16,:, -1,1:]
        F[16,:,0 , -1] = T[16,:, -1,0 ]
        F[16,:,1:,:-1] = T[16,:,:-1,1:]
        F[16,:,1:, -1] = T[16,:,:-1,0 ]

        F[17,:,:-1,0 ] = T[17,:,1:, -1]
        F[17,:,:-1,1:] = T[17,:,1:,:-1]
        F[17,:, -1,0 ] = T[17,:,0 , -1]
        F[17,:, -1,1:] = T[17,:,0 ,:-1]

        F[18,:,:-1,:-1] = T[18,:,1:,1:]
        F[18,:,:-1, -1] = T[18,:,1:,0 ]
        F[18,:,-1 ,:-1] = T[18,:,0 ,1:]
        F[18,:,-1 , -1] = T[18,:,0 ,0 ]
        #Densities bouncing back at next timestep
        BB = np.empty(F.shape)
        T[:] = F
        T[1:,:,:,:] *= BOUND[np.newaxis,:,:,:]
        BB[2 ,:,:,:] += T[1 ,:,:,:]
        BB[1 ,:,:,:] += T[2 ,:,:,:]
        BB[4 ,:,:,:] += T[3 ,:,:,:]
        BB[3 ,:,:,:] += T[4 ,:,:,:]
        BB[6 ,:,:,:] += T[5 ,:,:,:]
        BB[5 ,:,:,:] += T[6 ,:,:,:]
        BB[10,:,:,:] += T[7 ,:,:,:]
        BB[9 ,:,:,:] += T[8 ,:,:,:]
        BB[8 ,:,:,:] += T[9 ,:,:,:]
        BB[7 ,:,:,:] += T[10,:,:,:]
        BB[14,:,:,:] += T[11,:,:,:]
        BB[13,:,:,:] += T[12,:,:,:]
        BB[12,:,:,:] += T[13,:,:,:]
        BB[11,:,:,:] += T[14,:,:,:]
        BB[18,:,:,:] += T[15,:,:,:]
        BB[17,:,:,:] += T[16,:,:,:]
        BB[16,:,:,:] += T[17,:,:,:]
        BB[15,:,:,:] += T[18,:,:,:]

        # Relax calculate equilibrium state (FEQ) with equivalent speed and density to F
        DENSITY = np.add.reduce(F)

        #UX = F[5,:,:,:].copy()
        UX = np.ones(F[5,:,:,:].shape, dtype=np.float64)
        UX[:,:,:] = F[5,:,:,:]

        UX += F[7,:,:,:]
        UX += F[8,:,:,:]
        UX += F[11,:,:,:]
        UX += F[12,:,:,:]
        UX -= F[6,:,:,:]
        UX -= F[9,:,:,:]
        UX -= F[10,:,:,:]
        UX -= F[13,:,:,:]
        UX -= F[14,:,:,:]
        UX /=DENSITY

        #UY = F[3,:,:,:].copy()
        UY = np.ones(F[3,:,:,:].shape, dtype=np.float64)
        UY[:,:,:] = F[3,:,:,:]

        UY += F[7,:,:,:]
        UY += F[9,:,:,:]
        UY += F[15,:,:,:]
        UY += F[16,:,:,:]
        UY -= F[4,:,:,:]
        UY -= F[8,:,:,:]
        UY -= F[10,:,:,:]
        UY -= F[17,:,:,:]
        UY -= F[18,:,:,:]
        UY /=DENSITY

        #UZ = F[1,:,:,:].copy()
        UZ = np.ones(F[1,:,:,:].shape, dtype=np.float64)
        UZ[:,:,:] = F[1,:,:,:]

        UZ += F[11,:,:,:]
        UZ += F[13,:,:,:]
        UZ += F[15,:,:,:]
        UZ += F[17,:,:,:]
        UZ -= F[2,:,:,:]
        UZ -= F[12,:,:,:]
        UZ -= F[14,:,:,:]
        UZ -= F[16,:,:,:]
        UZ -= F[18,:,:,:]
        UZ /=DENSITY

        UX[0,:,:] += deltaU #Increase inlet pressure
                            #Set bourderies to zero.
        UX[:,:,:] *= BOUNDi
        UY[:,:,:] *= BOUNDi
        UZ[:,:,:] *= BOUNDi
        DENSITY[:,:,:] *= BOUNDi

        U_SQU = UX**2 + UY**2 + UZ**2

        # Calculate equilibrium distribution: stationary
        FEQ[0,:,:,:] = (t1*DENSITY)*(1.0-3.0*U_SQU/2.0)
        # nearest-neighbours
        T1 = 3.0/2.0*U_SQU
        tDENSITY = t2*DENSITY
        FEQ[1,:,:,:]=tDENSITY*(1.0 + 3.0*UZ + 9.0/2.0*UZ**2 - T1)
        FEQ[2,:,:,:]=tDENSITY*(1.0 - 3.0*UZ + 9.0/2.0*UZ**2 - T1)
        FEQ[3,:,:,:]=tDENSITY*(1.0 + 3.0*UY + 9.0/2.0*UY**2 - T1)
        FEQ[4,:,:,:]=tDENSITY*(1.0 - 3.0*UY + 9.0/2.0*UY**2 - T1)
        FEQ[5,:,:,:]=tDENSITY*(1.0 + 3.0*UX + 9.0/2.0*UX**2 - T1)
        FEQ[6,:,:,:]=tDENSITY*(1.0 - 3.0*UX + 9.0/2.0*UX**2 - T1)
        # next-nearest neighbours
        T1 = 3.0*U_SQU/2.0
        tDENSITY = t3*DENSITY
        U8 = UX+UY
        FEQ[7,:,:,:] =tDENSITY*(1.0 + 3.0*U8  + 9.0/2.0*(U8)**2  - T1)
        U9 = UX-UY
        FEQ[8,:,:,:] =tDENSITY*(1.0 + 3.0*U9  + 9.0/2.0*(U9)**2  - T1)
        U10 = -UX+UY
        FEQ[9,:,:,:] =tDENSITY*(1.0 + 3.0*U10 + 9.0/2.0*(U10)**2 - T1)
        U8 *= -1.0
        FEQ[10,:,:,:]=tDENSITY*(1.0 + 3.0*U8 + 9.0/2.0*(U8)**2 - T1)
        U12 = UX+UZ
        FEQ[11,:,:,:]=tDENSITY*(1.0 + 3.0*U12 + 9.0/2.0*(U12)**2 - T1)
        U12 *= 1.0
        FEQ[14,:,:,:]=tDENSITY*(1.0 + 3.0*U12 + 9.0/2.0*(U12)**2 - T1)
        U13 = UX-UZ
        FEQ[12,:,:,:]=tDENSITY*(1.0 + 3.0*U13 + 9.0/2.0*(U13)**2 - T1)
        U13 *= -1.0
        FEQ[13,:,:,:]=tDENSITY*(1.0 + 3.0*U13 + 9.0/2.0*(U13)**2 - T1)
        U16 = UY+UZ
        FEQ[15,:,:,:]=tDENSITY*(1.0 + 3.0*U16 + 9.0/2.0*(U16)**2 - T1)
        U17 = UY-UZ
        FEQ[16,:,:,:]=tDENSITY*(1.0 + 3.0*U17 + 9.0/2.0*(U17)**2 - T1)
        U17 *= -1.0
        FEQ[17,:,:,:]=tDENSITY*(1.0 + 3.0*U17 + 9.0/2.0*(U17)**2 - T1)
        U16 *= -1.0
        FEQ[18,:,:,:]=tDENSITY*(1.0 + 3.0*U16 + 9.0/2.0*(U16)**2 - T1)
        F *= (1.0-omega)
        F += omega * FEQ

        #Densities bouncing back at next timestep
        F[1:,:,:,:] *= BOUNDi[np.newaxis,:,:,:]
        F[1:,:,:,:] += BB[1:,:,:,:]

        del BB
        del T1
        del UX, UY, UZ
        del U_SQU
        del DENSITY, tDENSITY
        del U8, U9, U10, U12, U13, U16, U17
        if util.Benchmark().bohrium:
            np.flush()

    B.stop()
    B.pprint()

    if B.outputfn:
        B.tofile(B.outputfn, {'res': UX})

    """
    import matplotlib.pyplot as plt
    UX *= -1
    plt.hold(True)
    plt.quiver(UY[:,:,4],UX[:,:,4], pivot='middle')
    plt.imshow(BOUND[:,:,4])
    plt.show()

    """

if __name__ == "__main__":
    main()

from __future__ import print_function
"""
Jacobi Stencil
--------------

So what does this code example illustrate?
"""
import util
if util.Benchmark().bohrium:
    import bohrium as np
else:
    import numpy as np

def freezetrap(height, width, dtype=np.float32):

    grid = np.zeros((height+2,width+2), dtype=dtype)
    grid[:,0]  = dtype(-273.15)
    grid[:,-1] = dtype(-273.15)
    grid[-1,:] = dtype(-273.15)
    grid[0,:]  = dtype(40.0)

    return grid

def solve(grid, epsilon=0.005, max_iterations=None, visualize=False):

    center = grid[1:-1, 1:-1]
    north  = grid[0:-2, 1:-1]
    east   = grid[1:-1, 2:  ]
    west   = grid[1:-1, 0:-2]
    south  = grid[2:  , 1:-1]
    delta = epsilon + 1
    iteration = 0

    while delta > epsilon:
        iteration += 1
        work = 0.2*(center+north+east+west+south)
        delta = np.add.reduce(np.add.reduce(np.absolute(work-center)))
        center[:] = work
        if max_iterations != None and max_iterations <= iteration:
            break
        if visualize:
            np.visualize(grid, "2d", 0, 0.0, 5.5)

    return grid

def iterate(grid, iterations, visualize=False):

    center = grid[1:-1, 1:-1]
    north  = grid[0:-2, 1:-1]
    east   = grid[1:-1, 2:  ]
    west   = grid[1:-1, 0:-2]
    south  = grid[2:  , 1:-1]

    for i in xrange(iterations):
        center[:] = 0.2*(center+north+east+west+south)
        if visualize:
            np.visualize(grid, "2d", 0, 0.0, 5.5)

    return grid

def main():
    B = util.Benchmark()
    H = B.size[0]
    W = B.size[1]
    I = B.size[2]

    ft = freezetrap(H, W, dtype=B.dtype)

    B.start()
    R = solve(ft, max_iterations=I, visualize=B.visualize)
    B.stop()

    B.pprint()
    if B.verbose:
        print(R)
    if B.outputfn:
        B.tofile(B.outputfn, {'res': R})

if __name__ == "__main__":
    main()

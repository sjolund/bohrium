from __future__ import print_function
import util
#if util.Benchmark().bohrium:
#    import bohrium as np
#else:
#    import numpy as np
import bohrium as np
from bohrium.stdviews import cartesian



"""
def power_init(photo='datasets/Hell.jpg'):

    img     = Image.open(photo)
    rgb     = np.array(img)

    print("shape: " + str(rgb.shape))

    tones   = np.array((0.3, 0.6, 0.11))
    rgb     = np.add.reduce((rgb*tones[np.newaxis, np.newaxis, :]), axis=2)
    print("shape: " + str(rgb.shape))
    print("content: " + str(rgb[1:10,1:10]))
    return rgb
"""

def model(N, dtype=np.float32):
    """Construct pseudo-data representing price samples?"""

    np.random.seed(seed=1)

    m = np.array(np.random.random([N,N]), dtype=dtype)
    m *= 100.0

    return m

def benchmark(n, exponent):
    B = util.Benchmark()
    ntimes = 10

    elapsed = 0.0
    for i in range(ntimes):
        a = model(n)
        B.start()
        R = np.power(a, exponent)
        B.stop()
        elapsed += B.elapsed()
    B.pprint()
    R = np.add.reduce(np.add.reduce(R))
    print ("Result for exponent="+str(exponent)+": " + str(R))
    print ("Elapsed (average over 10):" + str(elapsed/ntimes))

    #if not np.isscalar(R):
    R = R.item()

    return (elapsed/ntimes, R)

def main():
    """
    Example parameter: --size=25.
    This will execute on a something related to 25....
    """
    #image = power_init()

    image = np.array([[1,2],[3,4]])

    #compute_power(image, 2, 10)
    #compute_power(image, 3, 10)
    #compute_power(image, 4, 10)
    #compute_power(image, 5, 10)
    #compute_power(image, 6, 10)
    #compute_power(image, 7, 10)
    #compute_power(image, 8, 10)
    [compute_power(image, e, 1) for e in range(2,20)]
    #compute_power(image, 8, 1)


if __name__ == "__main__":
    main()

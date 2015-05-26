import shutil
import my_power
import my_common
import mc
import shallow_water
import black_scholes
import time
import pickle
import sys
import preprocessor_benchmark_settings

"""
dst = "/home/chris/.bohrium/config.ini"
src_filterOn = "/home/chris/.bohrium/config-filterOn.ini"
src_filterOff = "/home/chris/.bohrium/config-filterOff.ini"

def activateFilter():
	shutil.copyfile(src_filterOn, dst)

def deactivateFilter():
	shutil.copyfile(src_filterOff, dst)
"""




def main():
    to_run = preprocessor_benchmark_settings.to_run()

    results = dict()
    assert len(sys.argv) > 1, "requires an argument"
    file = sys.argv[1]
    
    if 'power' in to_run:
        n = 5000
        n = 2000
        max_exponent = 100
        x = range(2,max_exponent+1)
        #x = [(2**e)-1 for e in range(2,12)]

        measures = [my_power.benchmark(n, e) for e in x]
        (time, res) = [list(t) for t in zip(*measures)]
        results['power'] = {'x':x, 'y':time, 'result':res[-1]}


    if 'black_scholes' in to_run:
        # black scholes
        #x = range(500,5001,500)
        x = [n**2 for n in range(500,5001,500)]
        #x = [100]
        I = 10
        measures = [black_scholes.benchmark(N, I) for N in x]
        (time, res) = [list(t) for t in zip(*measures)]
        results['black_scholes'] = {'x':x, 'y':time, 'result':res[-1]}


    if 'mc' in to_run:
        xy = mc.benchmark()
        results['mc'] = xy
    

    if 'shallow_water' in to_run:
        # shallow water
        #x = range(100,1001,100)
        x = range(100,1001,100)
        x = range(500,5001,500)
        #x = [1000]
        #x = [100]
        I = 100
        I = 1
        measures = [shallow_water.benchmark(w, I) for w in x]
        (time, res) = [list(t) for t in zip(*measures)]
        results['shallow_water'] = {'x':x, 'y':time, 'result':res[-1]}
    

    if 'common' in to_run:
        # common test (part of Shallow water)
        #x = range(100,1001,100)
        x = range(500,5001,500)
        x = range(3000,5001,500)
        #x = range(500,4001,500)
        #x = [5000]
        I = 1
        measures = [my_common.benchmark(w, I) for w in x]
        (time, res) = [list(t) for t in zip(*measures)]
        results['common'] = {'x':x, 'y':time, 'result':res[-1]}
    


    outfile = open(file, "wb" )
    pickle.dump(results, outfile)

if __name__ == "__main__":
    main()
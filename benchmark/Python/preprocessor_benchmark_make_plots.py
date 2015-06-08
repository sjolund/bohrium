from plot import *
import shutil
import time
import pickle
import sys
from time import localtime, strftime
import preprocessor_benchmark_settings





def main():
    timestamp = strftime("%Y-%m-%d_%H-%M-%S", localtime())
    to_run = preprocessor_benchmark_settings.to_run()

    print sys.argv
    assert len(sys.argv) > 2, "requires two arguments"
    file_filterOn = sys.argv[1]
    file_filterOff = sys.argv[2]

    infile_filterOn = open( file_filterOn, "rb" )
    results_filterOn = pickle.load(infile_filterOn)
    infile_filterOn.close()
    print("filterOn results:")
    print(results_filterOn)

    infile_filterOff = open( file_filterOff, "rb" )
    results_filterOff = pickle.load(infile_filterOff)
    infile_filterOff.close()
    print("filterOff results:")
    print(results_filterOff)

    if 'power' in to_run:
        filterOn = results_filterOn['power']
        filterOff = results_filterOff['power']
        (x1,y1,res1) = (filterOn['x'],filterOn['y'],filterOn['result'])
        (x2,y2,res2) = (filterOff['x'],filterOff['y'],filterOff['result'])
        #print ("Difference is: %s" % (str(abs(res1 - res2))))
        #assert (abs(res1 - res2)<1.0e-10), "results are not equal: %s and %s. Difference is: %s" % (str(res1), str(res2), str(abs(res1 - res2)))
        xs = [x1,x2]
        ys = [y1,y2]
        plot_comparison(xs, ys, 'power', 'Exponent', ["filter on", "filter off"], timestamp, showFig=True)
        plot_speedup(xs, ys, 'power', 'Exponent', timestamp, showFig=True)

    if 'powerodd' in to_run:
        filterOn = results_filterOn['powerodd']
        filterOff = results_filterOff['powerodd']
        (x1,y1,res1) = (filterOn['x'],filterOn['y'],filterOn['result'])
        (x2,y2,res2) = (filterOff['x'],filterOff['y'],filterOff['result'])
        #print ("Difference is: %s" % (str(abs(res1 - res2))))
        #assert (abs(res1 - res2)<1.0e-10), "results are not equal: %s and %s. Difference is: %s" % (str(res1), str(res2), str(abs(res1 - res2)))
        xs = [x1,x2]
        ys = [y1,y2]
        plot_comparison(xs, ys, 'powerodd', 'Exponent', ["filter on", "filter off"], timestamp, showFig=True)
        plot_speedup(xs, ys, 'powerodd', 'Exponent', timestamp, showFig=True)

    if 'powercaching' in to_run:
        # power caching
        filterOn = results_filterOn['powercaching']
        filterOff = results_filterOff['powercaching']
        (x1,y1,res1) = (filterOn['x'],filterOn['y'],filterOn['result'])
        (x2,y2,res2) = (filterOff['x'],filterOff['y'],filterOff['result'])
        print ("Difference is: %s" % (str(abs(res1 - res2))))
        assert (abs(res1 - res2)<1.0e-10), "results are not equal: %s and %s. Difference is: %s" % (str(res1), str(res2), str(abs(res1 - res2)))
        xs = [x1,x2]
        ys = [y1,y2]
        plot_comparison(xs, ys, 'powercaching', 'n', ["filter on", "filter off"], timestamp, showFig=True)
        plot_speedup(xs, ys, 'powercaching', 'n', timestamp, showFig=True)

    if 'powercorrectness' in to_run:

        filterOn = results_filterOn['powercorrectness']
        filterOff = results_filterOff['powercorrectness']
        (x1,y1) = (filterOn['x'],filterOn['y'])
        (x2,y2) = (filterOff['x'],filterOff['y'])
        for i in range(len(x1)):
            assert (x1[i] == x2[i]), "xs are not equal: %s and %s. Difference is: %s" % (str(x1[i]), str(x2[i]), str(abs(x1[i] - x2[i])))

        # sort the lists
        x1, y1, y2 = zip(*sorted(zip(x1, y1, y2)))

        print ("\n\\begin{table}[H]\n\\centering\n\\begin{tabular}{llll}")
        print ("x & $x^{31}$ (filter on) & $x^{31}$ (filter off) & difference ($\\mbox{on}-\\mbox{off}$)  \\\\ \\hline")
        res = []
        for i in range(len(y1)):
            print("%s & %s & %s & %s \\\\" % (str(x1[i]), str(y1[i]), str(y2[i]), str(y1[i] - y2[i])))
            #print("values are: %s and %s. Difference is: %s" % (str(y1[i]), str(y2[i]), str(abs(y1[i] - y2[i]))))
            res.append(y1[i]-y2[i])
        print ("\\end{tabular}\n\\caption{}\\label{tab:}\n\\end{table}\n")

        print("x: "+str(x1))
        print("y-diff: "+str(res))

    if 'black_scholes' in to_run:
        # Black scholes
        filterOn = results_filterOn['black_scholes']
        filterOff = results_filterOff['black_scholes']
        (x1,y1,res1) = (filterOn['x'],filterOn['y'],filterOn['result'])
        (x2,y2,res2) = (filterOff['x'],filterOff['y'],filterOff['result'])
        print ("Difference is: %s" % (str(abs(res1 - res2))))
        assert (abs(res1 - res2)<1.0e-10), "results are not equal: %s and %s. Difference is: %s" % (str(res1), str(res2), str(abs(res1 - res2)))
        xs = [x1,x2]
        ys = [y1,y2]
        plot_comparison(xs, ys, 'black_scholes', 'n', ["filter on", "filter off"], timestamp, showFig=True)
        plot_speedup(xs, ys, 'black_scholes', 'n', timestamp, showFig=True)
    

    if 'mc' in to_run:
        (x1,y1) = results_filterOn['mc']
        (x2,y2) = results_filterOff['mc']
        xs = [x1,x2]
        ys = [y1,y2]
        plot_comparison(xs, ys, 'mc', 'n', ["filter on", "filter off"], "0", showFig=True)
        plot_speedup(xs, ys, 'mc', 'n', "0", showFig=True)

    if 'shallow_water' in to_run:
        (x1,y1,res1) = (results_filterOn['shallow_water']['x'],results_filterOn['shallow_water']['y'],results_filterOn['shallow_water']['result'])
        (x2,y2,res2) = (results_filterOff['shallow_water']['x'],results_filterOff['shallow_water']['y'],results_filterOff['shallow_water']['result'])
        #assert res1 == res2, "results are not equal: %s and %s" % (str(res1), str(res2))
        xs = [x1,x2]
        ys = [y1,y2]
        plot_comparison(xs, ys, 'shallow_water', 'n', ["filter on", "filter off"], timestamp, showFig=True)
        plot_speedup(xs, ys, 'shallow_water', 'n', timestamp, showFig=True)

    if 'common' in to_run:
        # my_common
        filterOn = results_filterOn['common']
        filterOff = results_filterOff['common']
        (x1,y1,res1) = (filterOn['x'],filterOn['y'],filterOn['result'])
        (x2,y2,res2) = (filterOff['x'],filterOff['y'],filterOff['result'])
        print ("Difference is: %s" % (str(abs(res1 - res2))))
        assert (abs(res1 - res2)<1.0e-10), "results are not equal: %s and %s" % (str(res1), str(res2))     # random data - cannot assert
        xs = [x1,x2]
        ys = [y1,y2]
        plot_comparison(xs, ys, 'common', 'n', ["filter on", "filter off"], timestamp, showFig=True)
        plot_speedup(xs, ys, 'common', 'n', timestamp, showFig=True)

    if 'common4' in to_run:
        # my_common
        filterOn = results_filterOn['common4']
        filterOff = results_filterOff['common4']
        (x1,y1,res1) = (filterOn['x'],filterOn['y'],filterOn['result'])
        (x2,y2,res2) = (filterOff['x'],filterOff['y'],filterOff['result'])
        print ("Difference is: %s" % (str(abs(res1 - res2))))
        assert (abs(res1 - res2)<1.0e-10), "results are not equal: %s and %s" % (str(res1), str(res2))     # random data - cannot assert
        xs = [x1,x2]
        ys = [y1,y2]
        plot_comparison(xs, ys, 'common4', 'n', ["filter on", "filter off"], timestamp, showFig=True)
        plot_speedup(xs, ys, 'common4', 'n', timestamp, showFig=True)

    if 'common3d' in to_run:
        # my_common
        filterOn = results_filterOn['common3d']
        filterOff = results_filterOff['common3d']
        (x1,y1,res1) = (filterOn['x'],filterOn['y'],filterOn['result'])
        (x2,y2,res2) = (filterOff['x'],filterOff['y'],filterOff['result'])
        print ("Difference is: %s" % (str(abs(res1 - res2))))
        assert (abs(res1 - res2)<1.0e-10), "results are not equal: %s and %s" % (str(res1), str(res2))     # random data - cannot assert
        xs = [x1,x2]
        ys = [y1,y2]
        plot_comparison(xs, ys, 'common3d', 'n', ["filter on", "filter off"], timestamp, showFig=True)
        plot_speedup(xs, ys, 'common3d', 'n', timestamp, showFig=True)

    if 'commonexp' in to_run:
        # my_common
        filterOn = results_filterOn['commonexp']
        filterOff = results_filterOff['commonexp']
        (x1,y1,res1) = (filterOn['x'],filterOn['y'],filterOn['result'])
        (x2,y2,res2) = (filterOff['x'],filterOff['y'],filterOff['result'])
        print ("Difference is: %s" % (str(abs(res1 - res2))))
        #assert (abs(res1 - res2)<1.0e-10), "results are not equal: %s and %s. Difference is: %s" % (str(res1), str(res2), str(abs(res1 - res2)))
        xs = [x1,x2]
        ys = [y1,y2]
        plot_comparison(xs, ys, 'commonexp', 'n', ["filter on", "filter off"], timestamp, showFig=True)
        plot_speedup(xs, ys, 'commonexp', 'n', timestamp, showFig=True)
    

if __name__ == "__main__":
    main()
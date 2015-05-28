import numpy as np
import matplotlib.pyplot as plt

def plot(xs, ys, name, xlabel, ylabel, labels, timestamp, showFig=False):
    xs = np.array(xs)
    ys = np.array(ys)
    print("shape:")
    print(np.shape(xs))
    print(len(np.shape(xs)))
    if (len(np.shape(xs)) == 1):
        x_n = 1
    else:
        x_n = len(xs)
    if (len(np.shape(ys)) == 1):
        y_n = 1
    else:
        y_n = len(ys)
    assert x_n == y_n, "xs and ys must have same dimension. They have %s and %s" % (str(np.shape(xs)), str(np.shape(ys)))
    print 'x_n: ' + str(x_n)

    fig, ax = plt.subplots()
    
    opacity = 0.4
    error_config = {'ecolor': '0.3'}

    if (x_n > 1):
        for i in xrange(x_n):
            plt.plot(xs[i], ys[i], '-', marker='.', label=labels[i])
    else:
        plt.plot(xs, ys, '-', marker='.', label=labels[0])


    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    #plt.title('Performance comparison')

    plt.legend(loc='lower right', bbox_to_anchor=(1, 0.5))

    #ax.set_xscale('log', basex=2)
    #ax.set_xscale('log')
    ax.set_yscale('log')
    
    if showFig:
        plt.show()

    fig.savefig('figs/'+name+'_'+str(timestamp)+'_comparison.pdf')
    
    plt.close()



if __name__ == '__main__':
    xs = [x/10.0 for x in xrange(1,15)]
    ys1 = [(10**x)**31 for x in xs]
    ys2 = [(10**(-x))**31 for x in xs]
    plot([xs,xs], [ys1,ys2], "power-31-growth", "x", "y", ["y = (10^x)^31", "y = (10^-x)^31"], "0", showFig=True)
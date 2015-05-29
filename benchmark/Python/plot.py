"""
Bar chart demo with pairs of bars grouped for easy comparison.
"""
import numpy as np
import matplotlib.pyplot as plt

def create_comparison_plot(xs, ys, colors, labels, timestamp, cpus, showFig=False):
    #bodiel
    #n_groups = 3
    #sequential = (1, 3, 30)
    #vector = (0.01, 0.1, 1)
    #opencl = (0.01, 0.1, 0.8)

    n_groups = len(xs)
    n_bars = len(ys)
    print 'n_bars: ' + str(n_bars)
    print 'labels len: ' + str(len(labels))
    
#    sequential = seq
#    parallel = par
    
    fig, ax = plt.subplots()
    
    
    index = np.arange(n_groups)
    bar_width = 0.8/n_bars
    
    opacity = 0.4
    error_config = {'ecolor': '0.3'}
    
    #plt.rc('axes', grid=True)
    #plt.rc(('grid', color='0.75', linewidth=0.5)
    
    ax.grid(True)
    ax.yaxis.grid(True, which='minor')
    ax.xaxis.grid(False)
    
    for i in xrange(n_bars):
        plt.bar(index + (i+1-n_bars/2.0)*bar_width, ys[i], bar_width,
                     alpha=opacity,
                     color=colors[i],
                     error_kw=error_config,
                     label=labels[i])
    
    
    
    plt.xlabel('CPUs')
    plt.ylabel('Time')
    plt.title('Performance comparison')
    #plt.xticks(index + 1.5*bar_width, ('10', '100', '1000'))
    plt.xticks(index + bar_width, [str(n) for n in xs])
    #plt.legend()
    plt.legend(loc='lower right', bbox_to_anchor=(1, 0))
    ax.set_yscale("log", nonposy='clip')
    #ax.set_ylim(ymin=0.001)
    #plt.yscale('log')
    plt.tight_layout()
    
    [line.set_zorder(10) for line in ax.lines]
    
    if showFig:
        plt.show()
    
    fig.savefig('report/figs/'+str(timestamp)+'_comparison.pdf')
    plt.close()




def plot_comparison(xs, ys, name, xlabel, labels, timestamp, showFig=False):
    xs = np.array(xs)
    ys = np.array(ys)
    #print("shape:")
    #print(np.shape(xs))
    #print(len(np.shape(xs)))
    if (len(np.shape(xs)) == 1):
        x_n = 1
    else:
        x_n = len(xs)
    if (len(np.shape(ys)) == 1):
        y_n = 1
    else:
        y_n = len(ys)
    assert x_n == y_n, "xs and ys must have same dimension. They have %s and %s" % (str(np.shape(xs)), str(np.shape(ys)))
    #print 'x_n: ' + str(x_n)

    fig, ax = plt.subplots()
    
    opacity = 0.4
    error_config = {'ecolor': '0.3'}

    if (x_n > 1):
        for i in xrange(x_n):
            plt.plot(xs[i], ys[i], '-', marker='.', label=labels[i])
    else:
        plt.plot(xs, ys, '-', marker='.', label=labels[0])


    plt.xlabel(xlabel)
    plt.ylabel('Time (seconds)')
    plt.title('Performance comparison')

    plt.legend(loc='lower right', bbox_to_anchor=(1, 0))

    #ax.set_xscale('log', basex=2)
    #ax.set_xscale('log')
    #ax.set_yscale('log')
    
    if showFig:
        plt.show()

    fig.savefig('figs/'+name+'_'+str(timestamp)+'_comparison.pdf')
    
    plt.close()

def plot_speedup(xs, ys, name, xlabel, timestamp, showFig=False):
    xs = np.array(xs)
    ys = np.array(ys)
    assert (len(np.shape(ys)) > 1 and len(ys) == 2), "ys must have two rows"
    y = ys[1,:]/ys[0,:]

    if (len(np.shape(xs)) > 1):
        x = xs[0]

    fig, ax = plt.subplots()

    plt.plot(x, y, '-', marker='.')


    plt.xlabel(xlabel)
    plt.ylabel('Speedup')
    plt.title('Speedup')

    ax.set_ylim(ymin=0)
    #ax.set_xscale('log', basex=2)
    #ax.set_xscale('log')
    

    if showFig:
        plt.show()

    fig.savefig('figs/'+name+'_'+str(timestamp)+'_speedup.pdf')
    
    plt.close()


if __name__ == '__main__':
    simple_plot([10, 100, 1000], [10, 100, 1000])
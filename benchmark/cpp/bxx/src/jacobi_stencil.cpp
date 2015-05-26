#include <iostream>
#include "bxx/bohrium.hpp"
#include "util/timing.hpp"
#include "util/argparse.hpp"

using namespace std;
using namespace bxx;
using namespace argparse;

template <typename T>
T solve(int w, int h, int iterations)
{
    multi_array<T> grid, center, north, south, east, west;

    grid                = zeros<T>(w, h);   // Initialize grid
    grid[_(1,h,1)][ 0]  = -273.15;          // .
    grid[_(1,h,1)][-1]  = -273.15;          // .
    grid[-1][_(0,w,1)]  = -273.15;          // .
    grid[ 0][_(0,w,1)]  =    40.0;          // and border values.

    center  = grid[_(1,-1,1)][_(1,-1,1)];   // Setup stencil
    north   = grid[_(0,-2,1)][_(1,-1,1)];
    south   = grid[_(2, h,1)][_(1,-1,1)];
    east    = grid[_(1,-1,1)][_(2, w,1)];
    west    = grid[_(1,-1,1)][_(0,-2,1)];

    for(int i=0; i<iterations; i++) {       // Approximate
        center((T)0.2*(center+north+east+west+south));
    }
    return scalar(sum(grid));
}

int main(int argc, char* argv[])
{
    const char usage[] = "usage: ./jacobi_solver --size=1000*1000*10 [--verbose]";
    if (2>argc) {
        cout << usage << endl;
        return 1;
    }

    arguments_t args;                               // Parse command-line
    if (!parse_args(argc, argv, args)) {
        cout << "Err: Invalid argument(s)." << endl;
        cout << usage << endl;
        return 1;
    }
    if (3 > args.size.size()) {
        cout << "Err: Not enough arguments." << endl;
        cout << usage << endl;
        return 1;
    }
    if (3 < args.size.size()) {
        cout << "Err: Too many arguments." << endl;
        cout << usage << endl;
        return 1;
    }

    for(size_t i=0; i<3; i++) {
        cout << args.size[i] << endl;
    }

    size_t start = sample_time();
    double output = solve<double>(args.size[0], args.size[1], args.size[2]);
    size_t end = sample_time();
                                                    // Output timing
    cout << "{elapsed-time: "<< (end-start)/1000000.0 <<"";          
    if (args.verbose) {                             // and values.
        cout << ", \"output\": [";
        cout << output;
        cout << "]";
    }
    cout << "}" << endl;

    return 0;
}


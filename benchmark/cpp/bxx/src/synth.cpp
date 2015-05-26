/*
This file is part of Bohrium and Copyright (c) 2012 the Bohrium team:
http://bohrium.bitbucket.org

Bohrium is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation, either version 3
of the License, or (at your option) any later version.

Bohrium is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the
GNU Lesser General Public License along with bohrium.

If not, see <http://www.gnu.org/licenses/>.
*/
#include <iostream>
#include "bxx/bohrium.hpp"
#include "util/timing.hpp"
#include "util/argparse.hpp"

using namespace std;
using namespace bxx;
using namespace argparse;

template <typename T>
multi_array<T> compute12_3a(size_t n, size_t iterations)
{
    multi_array<T> a, b, c, r;
    a = ones<T>(n, n);
    b = ones<T>(n, n);
    c = ones<T>(n, n);

    Runtime::instance().flush();
    for(size_t i=0; i<iterations; i++) {
        r = a+b+c+a+b+c+a+b+c+a+b+c;
        Runtime::instance().flush();
    }

    return r;
}

template <typename T>
multi_array<T> compute3_1a(size_t n, size_t iterations)
{
    multi_array<T> a, r;
    a = ones<T>(n, n);

    Runtime::instance().flush();
    for(size_t i=0; i<iterations; i++) {
        r = a+a+a;
        Runtime::instance().flush();
    }

    return r;
}

template <typename T>
multi_array<T> compute_12_1a(size_t n, size_t iterations)
{
    multi_array<T> a, r;
    a = ones<T>(n, n);

    Runtime::instance().flush();
    for(size_t i=0; i<iterations; i++) {
        r = a+a+a+a+a+a+a+a+a+a+a+a;
        Runtime::instance().flush();
    }

    return r;
}

template <typename T>
multi_array<T> compute_36_1a(size_t n, size_t iterations)
{
    multi_array<T> a, r;
    a = ones<T>(n, n);

    Runtime::instance().flush();
    for(size_t i=0; i<iterations; i++) {
        r = a+a+a+a+a+a+a+a+a+a+a+a +\
            a+a+a+a+a+a+a+a+a+a+a+a +\
            a+a+a+a+a+a+a+a+a+a+a+a;
        Runtime::instance().flush();
    }

    return r;
}

template <typename T>
multi_array<T> compute_72_1a(size_t n, size_t iterations)
{
    multi_array<T> a, r;
    a = ones<T>(n, n);

    Runtime::instance().flush();
    for(size_t i=0; i<iterations; i++) {
        r = a+a+a+a+a+a+a+a+a+a+a+a +\
            a+a+a+a+a+a+a+a+a+a+a+a +\
            a+a+a+a+a+a+a+a+a+a+a+a +\
            a+a+a+a+a+a+a+a+a+a+a+a +\
            a+a+a+a+a+a+a+a+a+a+a+a +\
            a+a+a+a+a+a+a+a+a+a+a+a;
        Runtime::instance().flush();
    }

    return r;
}

template <typename T>
multi_array<T> compute_K72_1a(size_t n, size_t iterations)
{
    multi_array<T> a, r;
    a = ones<T>(n, n);

    T k = (T)2;

    Runtime::instance().flush();
    for(size_t i=0; i<iterations; i++) {
        r = a+k+a+k+a+k+a+k+a+k+a+k +\
            a+k+a+k+a+k+a+k+a+k+a+k +\
            a+k+a+k+a+k+a+k+a+k+a+k +\
            a+k+a+k+a+k+a+k+a+k+a+k +\
            a+k+a+k+a+k+a+k+a+k+a+k +\
            a+k+a+k+a+k+a+k+a+k+a+k;
        Runtime::instance().flush();
    }

    return r;
}

int main(int argc, char* argv[])
{
    const char usage[] = "usage: ./synth --size=1000*10 [--verbose]";
    if (2>argc) {
        cout << usage << endl;
        return 1;
    }

    arguments_t args;                   // Parse command-line
    if (!parse_args(argc, argv, args)) {
        cout << "Err: Invalid argument(s)." << endl;
        cout << usage << endl;
        return 1;
    }
    if (2 > args.size.size()) {
        cout << "Err: Not enough arguments." << endl;
        cout << usage << endl;
        return 1;
    }
    if (2 < args.size.size()) {
        cout << "Err: Too many arguments." << endl;
        cout << usage << endl;
        return 1;
    }

    bh_intp start = sample_time();
    //multi_array<double> res = compute_K72_1a<double>(args.size[0], args.size[1]);
    //multi_array<float>  res = compute_K72_1a<float>(args.size[0], args.size[1]);
    //multi_array<float>  res = compute_12_1a<float>(args.size[0], args.size[1]);
    multi_array<double>  res = compute_12_1a<double>(args.size[0], args.size[1]);
    Runtime::instance().flush();
                                        // Output timing
    cout << "{elapsed-time: "<< (sample_time()-start)/1000000.0 <<"";
    if (args.verbose) {                 // and values.
        cout << ", \"output\": [";
        cout << res << endl;;
        cout << "]" << endl;
    }
    cout << "}" << endl;

    return 0;
}

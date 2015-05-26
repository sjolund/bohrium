from __future__ import print_function
#!/usr/bin/python
import argparse
import pprint
import pickle
import time
import sys
import re

import bohrium as bh
import numpy as np

def t_or_f(arg):
    """Helper function to parse "True/true/TrUe/False..." as bools."""

    ua = str(arg).lower()
    if ua == 'true'[:len(ua)]:
       return True
    elif ua == 'false'[:len(ua)]:
       return False
    else:
        return arg

class Benchmark:
    """
    Helper class to aid running Python/NumPy programs with and without npbackend.

    Use it to sample elapsed time using: start()/stop()
    Pretty-prints results using pprint().
    start()/stop() will send flush signals to npbackend, ensuring that only
    the statements in-between start() and stop() are measured.
    """

    def __init__(self):

        self.__elapsed  = 0.0           # The quantity measured
        self.__script   = sys.argv[0]   # The script being run

        # Just for reference... these are the options parsed from cmd-line.
        options = [
            'size',         'dtype',
            'inputfn',      'dumpinput',
            'outputfn'
            'target',      'bohrium',
            'no_extmethods',
            'visualize',    'verbose',
        ]

        # Construct argument parser
        p = argparse.ArgumentParser(description='Benchmark runner for npbackend.')

        # We can only have required options when the module is run from
        # command-line. When either directly or indirectly imported
        # we cant.
        owns_main = __name__ == "__main__"

        p.add_argument('--size',
                       help = "Tell the script the size of the data to work on."
        )
        p.add_argument('--dtype',
                       choices  = ["uint8", "float32", "float64"],
                       default  = "float64",
                       help     = "Tell the the script which primitive type to use."
                                  " (default: %(default)s)"
        )

        p.add_argument('--inputfn',
                       help = "Input file to use as data."
        )
        p.add_argument('--dumpinput',
                       default  = False,
                       action   = 'store_true',
                       help = "Dumps the benchmark input to file."
        )
        p.add_argument('--outputfn',
                       help     = "Output file to store results in."
        )

        g2 = p.add_mutually_exclusive_group()
        g2.add_argument('--target',
                       choices  = ['None', 'numpy', 'bhc',
                                   'numexpr', 'pygpu', 'chapel'],
                       default  = "None",
                       help     = "Enable npbackend using the specified target."
                                  " Disable npbackend using None."
                                  " (default: %(default)s)"
        )
        g2.add_argument('--bohrium',
                       choices  = [True, False],
                       default  = False,
                       type     = t_or_f,
                       help     = "Same as --target=bhc which means:"
                                  " enable npbackend using Bohrium."
                                  " (default: %(default)s)"
        )
        p.add_argument('--no-extmethods',
                       default  = False,
                       action   = 'store_true',
                       help     = "Disable extension methods."
        )

        p.add_argument('--visualize',
                       default  = False,
                       action   = 'store_true',
                       help     = "Enable visualization in script."
        )
        p.add_argument('--verbose',
                       default  = False,
                       action   = 'store_true',
                       help     = "Print out misc. information from script."
        )

        args, unknown = p.parse_known_args()   # Parse the arguments

        #
        # Conveniently expose options to the user
        #
        self.size       = [int(i) for i in args.size.split("*")] if args.size else []
        self.dtype      = eval("bh.%s" % args.dtype)
        self.dumpinput  = args.dumpinput
        self.inputfn    = args.inputfn
        self.outputfn   = args.outputfn

        # Unify the options: 'target' and 'bohrium'
        if args.bohrium:
            self.target = "bhc"
            self.bohrium = True
        elif args.target.lower() != 'none':
            self.target = args.target
            self.bohrium = True
        else:
            self.target = args.target
            self.bohrium = args.bohrium

        self.no_extmethods = args.no_extmethods

        self.visualize  = args.visualize
        self.verbose    = args.verbose

        #
        # Also make them available via the parser and arg objects
        self.p      = p
        self.args   = args

    def start(self):
        bh.flush()
        self.__elapsed = time.time()

    def stop(self):
        bh.flush()
        self.__elapsed = time.time() - self.__elapsed

    def tofile(self, filename, arrays):

        for k in arrays:
            arrays[k] = bh.array(arrays[k], bohrium=False)
        np.savez(filename, **arrays)

    def dump_arrays(self, prefix, arrays):
        """
        Dumps a dict of arrays organized such as:

        arrays = {'lbl1': array1, 'lbl2': array2}

        Into a file using the following naming convention:
        "prefix_lbl1-DTYPE-SHAPE_lbl2-DTYPE-SHAPE"

        The arrays are stored as .npz files.
        """
        names = []
        for k in arrays:
            names.append("%s-%s-%s" % (
                k,
                arrays[k].dtype,
                '*'.join([str(x) for x in (arrays[k].shape)]
            )))
        filename = "%s_%s" % (prefix, '_'.join(names))
        self.tofile(filename, arrays)

    def load_arrays(self, filename=None):

        if not filename:        # Default to the cmd-line parameter
            filename = self.inputfn

        npz = np.load(filename)

        arrays  = {}            # Make sure arrays are in the correct space
        for k in npz:
            arrays[k] = bh.array(npz[k], bohrium=self.bohrium)

        del npz                # We no longer need these

        return arrays

    def load_array(self, filename=None, label='input'):

        if not filename:
            filename = self.inputfn

        return self.load_arrays(filename)[label]

    def pprint(self):
        print("%s - target: %s, bohrium: %s, size: %s, elapsed-time: %f" % (
                self.__script,
                self.target,
                self.bohrium,
                '*'.join([str(s) for s in self.size]),
                self.__elapsed
        ))

    def elapsed(self):
        return self.__elapsed
        

def main():
    B = Benchmark()
    B.start()
    B.stop()
    if B.visualize:
        pprint.pprint(B.args)
    B.pprint()

if __name__ == "__main__":
    main()

#region Copyright
/*
This file is part of Bohrium and copyright (c) 2012 the Bohrium:
team <http://www.bh107.org>.

Bohrium is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation, either version 3
of the License, or (at your option) any later version.

Bohrium is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the
GNU Lesser General Public License along with Bohrium.

If not, see <http://www.gnu.org/licenses/>.
*/
#endregion

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using NumCIL;
using Utilities;

namespace nbody
{
	public static class nBody
    {
		public static void Main (string[] args)
		{
			var opts = Utilities.CommandLineParser.ExtractOptions(args.ToList());
			var manualTempRemoval = Utilities.RunBenchmark.ParseBoolOption(opts, "no-temp-arrays");
		
			Utilities.RunBenchmark.Run(args, 2,
               // Running the benchmark
               (input) => {
					var size = input.sizes[0];
					var iterations = input.sizes[1];
					
					if (manualTempRemoval)
					{
						if (input.type == typeof(double)) {
							var data = new nBodySolverDoubleNoTempArrays.Galaxy(size);
							using (new DispTimer(string.Format("nBodyNoTemp (Double) {0}*{1}", size, iterations)))
							{
								nBodySolverDoubleNoTempArrays.Solve(data, iterations);
								data.Sync();
							}
						} else {
							var data = new nBodySolverFloatNoTempArrays.Galaxy (size);
							using (new DispTimer(string.Format("nBodyNoTemp (Float) {0}*{1}", size, iterations)))
							{
								nBodySolverFloatNoTempArrays.Solve(data, iterations);
								data.Sync();
							}
						}
					}
					else
					{
						if (input.type == typeof(double)) {
							var data = new nBodySolverDouble.Galaxy(size);
							using (new DispTimer(string.Format("nBody (Double) {0}*{1}", size, iterations)))
							{
								nBodySolverDouble.Solve(data, iterations);
								data.Sync();
							}
						} else {
							var data = new nBodySolverFloat.Galaxy (size);
							using (new DispTimer(string.Format("nBody (Float) {0}*{1}", size, iterations)))
							{
								nBodySolverFloat.Solve(data, iterations);
								data.Sync();
							}
						}
					}
				}
			);
		}
    }
}

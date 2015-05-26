﻿
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

using R = NumCIL.Range;

namespace blackscholes
{
	using NumCIL.Double;
	using DATA = System.Double;
	
    public static class BlackScholesSolverDouble
    {
        private static NdArray CND(NdArray X)
        {
            DATA a1 = 0.31938153f, a2 = -0.356563782f, a3 = 1.781477937f, a4 = -1.821255978f, a5 = 1.330274429f;
            var L = X.Abs();
            var K = 1.0f / (1.0f + 0.2316419f * L);
            var w = 1.0f - 1.0f / ((DATA)Math.Sqrt(2 * Math.PI)) * (-L * L / 2.0f).Exp() * (a1 * K + a2 * (K.Pow(2)) + a3 * (K.Pow(3)) + a4 * (K.Pow(4)) + a5 * (K.Pow(5)));
            
            var mask1 = (NdArray)(X < 0);
            var mask2 = (NdArray)(X >= 0);

            w = w * mask2 + (1.0f - w) * mask1;
            return w;
        }

        private static NdArray BlackScholes(bool callputflag, NdArray S, DATA X, DATA T, DATA r, DATA v)
        {
            var d1 = ((S / X).Log() + (r + v * v / 2.0f) * T) / (v * (DATA)Math.Sqrt(T));
            var d2 = d1 - v * (DATA)Math.Sqrt(T);

            if (callputflag)
                return S * CND(d1) - X * (DATA)Math.Exp(-r * T) * CND(d2);
            else
                return X * (DATA)Math.Exp(-r * T) * CND(-d2) - S * CND(-d1);
        }
        
        public static NdArray Create(long size)
        {
            var S = Generate.Random(size);
            S = S * 4.0f - 2.0f + 60.0f; //Price is 58-62
            return S;
        }
        
        public static DATA Sync(NdArray data)
        {
        	return data.Value[0];
        }

        public static DATA Solve(NdArray data, long years)
        {
            DATA X = 65.0f;
            DATA r = 0.08f;
            DATA v = 0.3f;

            DATA day=1.0f/years;
            DATA T = day;

            DATA total = 0.0f;

            for (long i = 0; i < years; i++)
            {
                total += Add.Reduce(BlackScholes(true, data, X, T, r, v)).Value[0] / data.Shape.Length;
                T += day;
            }

            return total;
        }
    }
}

namespace blackscholes
{
	using NumCIL.Float;
	using DATA = System.Single;
	
    public static class BlackScholesSolverSingle
    {
        private static NdArray CND(NdArray X)
        {
            DATA a1 = 0.31938153f, a2 = -0.356563782f, a3 = 1.781477937f, a4 = -1.821255978f, a5 = 1.330274429f;
            var L = X.Abs();
            var K = 1.0f / (1.0f + 0.2316419f * L);
            var w = 1.0f - 1.0f / ((DATA)Math.Sqrt(2 * Math.PI)) * (-L * L / 2.0f).Exp() * (a1 * K + a2 * (K.Pow(2)) + a3 * (K.Pow(3)) + a4 * (K.Pow(4)) + a5 * (K.Pow(5)));
            
            var mask1 = (NdArray)(X < 0);
            var mask2 = (NdArray)(X >= 0);

            w = w * mask2 + (1.0f - w) * mask1;
            return w;
        }

        private static NdArray BlackScholes(bool callputflag, NdArray S, DATA X, DATA T, DATA r, DATA v)
        {
            var d1 = ((S / X).Log() + (r + v * v / 2.0f) * T) / (v * (DATA)Math.Sqrt(T));
            var d2 = d1 - v * (DATA)Math.Sqrt(T);

            if (callputflag)
                return S * CND(d1) - X * (DATA)Math.Exp(-r * T) * CND(d2);
            else
                return X * (DATA)Math.Exp(-r * T) * CND(-d2) - S * CND(-d1);
        }
        
        public static NdArray Create(long size)
        {
            var S = Generate.Random(size);
            S = S * 4.0f - 2.0f + 60.0f; //Price is 58-62
            return S;
        }
        
        public static DATA Sync(NdArray data)
        {
        	return data.Value[0];
        }

        public static DATA Solve(NdArray data, long years)
        {
            DATA X = 65.0f;
            DATA r = 0.08f;
            DATA v = 0.3f;

            DATA day=1.0f/years;
            DATA T = day;

            DATA total = 0.0f;

            for (long i = 0; i < years; i++)
            {
                total += Add.Reduce(BlackScholes(true, data, X, T, r, v)).Value[0] / data.Shape.Length;
                T += day;
            }

            return total;
        }
    }
}


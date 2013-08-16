#region Copyright
/*
This file is part of Bohrium and copyright (c) 2012 the Bohrium
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
using System.Runtime.InteropServices;

//Bohrium basic control types
using bh_intp = System.Int64;
using bh_index = System.Int64;
using bh_type = System.Int64;
using bh_enumbase = System.Int32;
using bh_data_ptr = System.IntPtr;

//Bohrium Signed data types
using bh_bool = System.SByte;
using bh_int8 = System.SByte;
using bh_int16 = System.Int16;
using bh_int32 = System.Int32;
using bh_int64 = System.Int64;

//Bohrium Unsigned data types
using bh_uint8 = System.Byte;
using bh_uint16 = System.UInt16;
using bh_uint32 = System.UInt32;
using bh_uint64 = System.UInt64;

//Bohrium floating point types
using bh_float32 = System.Single;
using bh_float64 = System.Double;

//Bohrium complex types
using bh_complex64 = NumCIL.Complex64.DataType;
using bh_complex128 = System.Numerics.Complex;

namespace NumCIL.Bohrium
{
    /// <summary>
    /// Container class for methods and datatypes that call Bohrium
    /// </summary>
    public static class PInvoke
    {
        /// <summary>
        /// The statically defined maximum Bohrium component name
        /// </summary>
        public const int BH_COMPONENT_NAME_SIZE = 1024;
        /// <summary>
        /// The statically defined maximum number of dimensions
        /// </summary>
        public const int BH_MAXDIM = 16;
        /// <summary>
        /// The statically defined maximum number of operands for built-in Bohrium instructions
        /// </summary>
        public const int BH_MAX_NO_OPERANDS = 3;

        /// <summary>
        /// Cached lookup to see if the process is running 64bit
        /// </summary>
        public static readonly bool Is64Bit = IntPtr.Size == 8;
        /// <summary>
        /// The size of an int pointer
        /// </summary>
        public static readonly int INTP_SIZE = Marshal.SizeOf(typeof(bh_intp));
        /// <summary>
        /// The size of the largest userfunc struct
        /// </summary>
        public static readonly int USERFUNC_SIZE = Marshal.SizeOf(typeof(bh_userfunc_union));
        /// <summary>
        /// The size of the random userfunc struct
        /// </summary>
        public static readonly int RANDOMFUNC_SIZE = Marshal.SizeOf(typeof(bh_userfunc_random));
        /// <summary>
        /// The size of the matmul userfunc struct
        /// </summary>
        public static readonly int MATMULFUNC_SIZE = Marshal.SizeOf(typeof(bh_userfunc_matmul));
        /// <summary>
        /// The size of the plain userfunc struct
        /// </summary>
        public static readonly int PLAINFUNC_SIZE = Marshal.SizeOf(typeof(bh_userfunc_plain));

        /// <summary>
        /// The size of the bh_type type
        /// </summary>
        public static readonly int BH_TYPE_SIZE = Marshal.SizeOf(Enum.GetUnderlyingType(typeof(bh_type)));        
        /// <summary>
        /// The size of the bh_index type
        /// </summary>
        public static readonly int BH_INDEX_SIZE = Marshal.SizeOf(typeof(bh_index));
        /// <summary>
        /// The size of the bh_base type
        /// </summary>
        public static readonly int BH_BASE_SIZE = Marshal.SizeOf(typeof(bh_base));

        /// <summary>
        /// The known component types in Bohrium
        /// </summary>
        public enum bh_component_type : long
        {
            /// <summary>
            /// The bridge component
            /// </summary>
            BH_BRIDGE,
            /// <summary>
            /// The Virtual Execution Manager component
            /// </summary>
            BH_VEM,
            /// <summary>
            /// The Virtual Execution component
            /// </summary>
            BH_VE,
            /// <summary>
            /// An unknown component type
            /// </summary>
            BH_COMPONENT_ERROR
        }

        /// <summary>
        /// The error codes defined in Bohrium
        /// </summary>
        public enum bh_error : long
        {
            /// <summary>
            /// General success
            /// </summary>
            BH_SUCCESS,
            /// <summary>
            /// Fatal error
            /// </summary>
            BH_ERROR,
            /// <summary>
            /// Data type not supported
            /// </summary>
            BH_TYPE_NOT_SUPPORTED,
            /// <summary>
            /// Out of memory
            /// </summary>
            BH_OUT_OF_MEMORY,
            /// <summary>
            /// Instruction not supported
            /// </summary>
            BH_INST_NOT_SUPPORTED,
            /// <summary>
            /// User-defined function not supported
            /// </summary>
            BH_USERFUNC_NOT_SUPPORTED
        }

        /// <summary>
        /// The data types supported by Bohrium
        /// </summary>
        public enum bh_type : long
        {
            /// <summary>
            /// The boolean datatype
            /// </summary>
            BH_BOOL,
            /// <summary>
            /// The signed 8bit datatype
            /// </summary>
            BH_INT8,
            /// <summary>
            /// The signed 16bit datatype
            /// </summary>
            BH_INT16,
            /// <summary>
            /// The signed 32bit datatype
            /// </summary>
            BH_INT32,
            /// <summary>
            /// The signed 64bit datatype
            /// </summary>
            BH_INT64,
            /// <summary>
            /// The unsigned 8bit datatype
            /// </summary>
            BH_UINT8,
            /// <summary>
            /// The unsigned 16bit datatype
            /// </summary>
            BH_UINT16,
            /// <summary>
            /// The unsigned 32bit datatype
            /// </summary>
            BH_UINT32,
            /// <summary>
            /// The unsigned 64bit datatype
            /// </summary>
            BH_UINT64,
            /// <summary>
            /// The 16bit floating point datatype
            /// </summary>
            BH_FLOAT16,
            /// <summary>
            /// The 32bit floating point datatype
            /// </summary>
            BH_FLOAT32,
            /// <summary>
            /// The 64bit floating point datatype
            /// </summary>
            BH_FLOAT64,
            /// <summary>
            /// The 64bit complex datatype
            /// </summary>
            BH_COMPLEX64,
            /// <summary>
            /// The 128bit complex datatype
            /// </summary>
            BH_COMPLEX128,
            /// <summary>
            /// The unknown datatype
            /// </summary>
            BH_UNKNOWN
        }

        /// <summary>
        /// The configuration dictionary for a component
        /// </summary>
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 0)]
        struct dictionary
        {
            /// <summary>
            /// Number of entries in dictionary
            /// </summary>
            public int n;
            /// <summary>
            /// Storage size
            /// </summary>
            public int size;
            /// <summary>
            /// SList of string values
            /// </summary>
            public byte[][] val;
            /// <summary>
            /// List of string keys
            /// </summary>
            public byte[][] key;
            /// <summary>
            /// List of hash values for keys
            /// </summary>
            public uint[] hash;
        }

        /// <summary>
        /// A constant value for a Bohrium operation
        /// </summary>
        [StructLayout(LayoutKind.Sequential)]
        public struct bh_constant
        {
            /// <summary>
            /// The value itself
            /// </summary>
            public bh_constant_value value;
            /// <summary>
            /// The value type
            /// </summary>
            public bh_type type;

            /// <summary>
            /// Constructs a new constant of the specified type
            /// </summary>
            /// <param name="type">The constant type</param>
            /// <param name="v">The constant value</param>
            public bh_constant(bh_type type, object v)
            {
                this.type = type;
                this.value = new bh_constant_value().Set(v);
            }

            /// <summary>
            /// Constructs a new constant using the specified value
            /// </summary>
            /// <param name="v">The constant value</param>
            public bh_constant(object v)
            {
                this.value = new bh_constant_value().Set(v);

                if (v is bh_bool)
                    this.type = bh_type.BH_BOOL;
                else if (v is bh_int16)
                    this.type = bh_type.BH_INT16;
                else if (v is bh_int32)
                    this.type = bh_type.BH_INT32;
                else if (v is bh_int64)
                    this.type = bh_type.BH_INT64;
                else if (v is bh_uint8)
                    this.type = bh_type.BH_UINT8;
                else if (v is bh_uint16)
                    this.type = bh_type.BH_UINT16;
                else if (v is bh_uint32)
                    this.type = bh_type.BH_UINT32;
                else if (v is bh_uint64)
                    this.type = bh_type.BH_UINT64;
                else if (v is bh_float32)
                    this.type = bh_type.BH_FLOAT32;
                else if (v is bh_float64)
                    this.type = bh_type.BH_FLOAT64;
                else if (v is bh_complex64)
                    this.type = bh_type.BH_COMPLEX64;
                else if (v is bh_complex128)
                    this.type = bh_type.BH_COMPLEX128;
                else
                    throw new NotSupportedException();
            }

            /// <summary>
            /// Returns a <see cref="System.String"/> that represents the current <see cref="NumCIL.Bohrium.PInvoke.bh_constant"/>.
            /// </summary>
            /// <returns>A <see cref="System.String"/> that represents the current <see cref="NumCIL.Bohrium.PInvoke.bh_constant"/>.</returns>
            public override string ToString()
			{
				if (this.type == bh_type.BH_BOOL)
					return this.value.bool8.ToString();
				if (this.type == bh_type.BH_INT16)
					return this.value.int16.ToString();
				if (this.type == bh_type.BH_INT32)
					return this.value.int32.ToString();
				if (this.type == bh_type.BH_INT64)
					return this.value.int64.ToString();
				if (this.type == bh_type.BH_UINT8)
					return this.value.uint8.ToString();
				if (this.type == bh_type.BH_UINT16)
					return this.value.uint16.ToString();
				if (this.type == bh_type.BH_UINT32)
					return this.value.uint32.ToString();
				if (this.type == bh_type.BH_UINT64)
					return this.value.uint64.ToString();
				if (this.type == bh_type.BH_FLOAT32)
					return this.value.float32.ToString();
				if (this.type == bh_type.BH_FLOAT64)
					return this.value.float64.ToString();
				if (this.type == bh_type.BH_COMPLEX64)
					return this.value.complex64.ToString();
				if (this.type == bh_type.BH_COMPLEX128)
					return this.value.complex128.ToString();
				else
					throw new NotSupportedException();

			}
		}

		/// <summary>
		/// Struct for typesafe assignment of a constant value
		/// </summary>
		[StructLayout(LayoutKind.Explicit)]
		public struct bh_constant_value
		{
			/// <summary>
			/// The boolean value
			/// </summary>
			[FieldOffset(0)]
			public bh_bool     bool8;
			/// <summary>
			/// The int8 value
            /// </summary>
            [FieldOffset(0)]
            public bh_int8     int8;
            /// <summary>
            /// The int16 value
            /// </summary>
            [FieldOffset(0)]
            public bh_int16    int16;
            /// <summary>
            /// The int32 value
            /// </summary>
            [FieldOffset(0)]
            public bh_int32    int32;
            /// <summary>
            /// The int64 value
            /// </summary>
            [FieldOffset(0)]
            public bh_int64    int64;
            /// <summary>
            /// The uint8 value
            /// </summary>
            [FieldOffset(0)]
            public bh_uint8    uint8;
            /// <summary>
            /// The uin16 value
            /// </summary>
            [FieldOffset(0)]
            public bh_uint16   uint16;
            /// <summary>
            /// The uint32 value
            /// </summary>
            [FieldOffset(0)]
            public bh_uint32   uint32;
            /// <summary>
            /// The uint64 value
            /// </summary>
            [FieldOffset(0)]
            public bh_uint64   uint64;
            /// <summary>
            /// The float32 value
            /// </summary>
            [FieldOffset(0)]
            public bh_float32  float32;
            /// <summary>
            /// The float64 value
            /// </summary>
            [FieldOffset(0)]
            public bh_float64  float64;
            /// <summary>
            /// The complex64 value
            /// </summary>
            [FieldOffset(0)]
            public bh_complex64  complex64;
            /// <summary>
            /// The complex128 value
            /// </summary>
            [FieldOffset(0)]
            public bh_complex128  complex128;

            /// <summary>
            /// Sets the value of this constant
            /// </summary>
            /// <param name="v">The value to set</param>
            /// <returns>A constant struct representing the value</returns>
            public bh_constant_value Set(bh_bool v) { this.bool8 = v; return this; }
            //public bh_constant Set(bh_int8 v) { this.int8 = v; return this; }
            /// <summary>
            /// Sets the value of this constant
            /// </summary>
            /// <param name="v">The value to set</param>
            /// <returns>A constant struct representing the value</returns>
            public bh_constant_value Set(bh_int16 v) { this.int16 = v; return this; }
            /// <summary>
            /// Sets the value of this constant
            /// </summary>
            /// <param name="v">The value to set</param>
            /// <returns>A constant struct representing the value</returns>
            public bh_constant_value Set(bh_int32 v) { this.int32 = v; return this; }
            /// <summary>
            /// Sets the value of this constant
            /// </summary>
            /// <param name="v">The value to set</param>
            /// <returns>A constant struct representing the value</returns>
            public bh_constant_value Set(bh_int64 v) { this.int64 = v; return this; }
            /// <summary>
            /// Sets the value of this constant
            /// </summary>
            /// <param name="v">The value to set</param>
            /// <returns>A constant struct representing the value</returns>
            public bh_constant_value Set(bh_uint8 v) { this.uint8 = v; return this; }
            /// <summary>
            /// Sets the value of this constant
            /// </summary>
            /// <param name="v">The value to set</param>
            /// <returns>A constant struct representing the value</returns>
            public bh_constant_value Set(bh_uint16 v) { this.uint16 = v; return this; }
            /// <summary>
            /// Sets the value of this constant
            /// </summary>
            /// <param name="v">The value to set</param>
            /// <returns>A constant struct representing the value</returns>
            public bh_constant_value Set(bh_uint32 v) { this.uint32 = v; return this; }
            /// <summary>
            /// Sets the value of this constant
            /// </summary>
            /// <param name="v">The value to set</param>
            /// <returns>A constant struct representing the value</returns>
            public bh_constant_value Set(bh_uint64 v) { this.uint64 = v; return this; }
            /// <summary>
            /// Sets the value of this constant
            /// </summary>
            /// <param name="v">The value to set</param>
            /// <returns>A constant struct representing the value</returns>
            public bh_constant_value Set(bh_float32 v) { this.float32 = v; return this; }
            /// <summary>
            /// Sets the value of this constant
            /// </summary>
            /// <param name="v">The value to set</param>
            /// <returns>A constant struct representing the value</returns>
            public bh_constant_value Set(bh_float64 v) { this.float64 = v; return this; }
            /// <summary>
            /// Sets the value of this constant
            /// </summary>
            /// <param name="v">The value to set</param>
            /// <returns>A constant struct representing the value</returns>
            public bh_constant_value Set(bh_complex64 v) { this.complex64 = v; return this; }
            /// <summary>
            /// Sets the value of this constant
            /// </summary>
            /// <param name="v">The value to set</param>
            /// <returns>A constant struct representing the value</returns>
            public bh_constant_value Set(bh_complex128 v) { this.complex128 = v; return this; }
            /// <summary>
            /// Sets the value of this constant
            /// </summary>
            /// <param name="v">The value to set</param>
            /// <returns>A constant struct representing the value</returns>
            public bh_constant_value Set(object v)
            {
                if (v is bh_bool)
                    return Set((bh_bool)v);
                else if (v is bh_int16)
                    return Set((bh_int16)v);
                else if (v is bh_int32)
                    return Set((bh_int32)v);
                else if (v is bh_int64)
                    return Set((bh_int64)v);
                else if (v is bh_uint8)
                    return Set((bh_uint8)v);
                else if (v is bh_uint16)
                    return Set((bh_uint16)v);
                else if (v is bh_uint32)
                    return Set((bh_uint32)v);
                else if (v is bh_uint64)
                    return Set((bh_uint64)v);
                else if (v is bh_float32)
                    return Set((bh_float32)v);
                else if (v is bh_float64)
                    return Set((bh_float64)v);
                else if (v is bh_complex64)
                    return Set((bh_complex64)v);
                else if (v is bh_complex128)
                    return Set((bh_complex128)v);

                throw new NotSupportedException();
            }
        }

        /// <summary>
        /// Represents a native data array
        /// </summary>
        [StructLayout(LayoutKind.Explicit)]
        public struct bh_data_array
        {
//Fix compiler reporting these as unused as they are weirdly mapped,
//and only processed from unmanaged code
#pragma warning disable 0414
#pragma warning disable 0169
            [FieldOffset(0)] private bh_bool[]     bool8;
            [FieldOffset(0)] private bh_int8[]     int8;
            [FieldOffset(0)] private bh_int16[]    int16;
            [FieldOffset(0)] private bh_int32[]    int32;
            [FieldOffset(0)] private bh_int64[]    int64;
            [FieldOffset(0)] private bh_uint8[]    uint8;
            [FieldOffset(0)] private bh_uint16[]   uint16;
            [FieldOffset(0)] private bh_uint32[]   uint32;
            [FieldOffset(0)] private bh_uint64[]   uint64;
            [FieldOffset(0)] private bh_float32[]  float32;
            [FieldOffset(0)] private bh_float64[]  float64;
            [FieldOffset(0)] private bh_complex64[] complex64;
            [FieldOffset(0)] private bh_complex128[] complex128;
            [FieldOffset(0)] private IntPtr voidPtr;
#pragma warning restore 0414
#pragma warning restore 0169

            /// <summary>
            /// Sets the array using a managed array
            /// </summary>
            /// <param name="v">The array to marshal</param>
            /// <returns>This array representation</returns>
            public bh_data_array Set(bh_bool[] v) { this.bool8 = v; return this; }
            //public bh_data_array Set(bh_int8[] v) { this.int8 = v; return this; }
            /// <summary>
            /// Sets the array using a managed array
            /// </summary>
            /// <param name="v">The array to marshal</param>
            /// <returns>This array representation</returns>
            public bh_data_array Set(bh_int16[] v) { this.int16 = v; return this; }
            /// <summary>
            /// Sets the array using a managed array
            /// </summary>
            /// <param name="v">The array to marshal</param>
            /// <returns>This array representation</returns>
            public bh_data_array Set(bh_int32[] v) { this.int32 = v; return this; }
            /// <summary>
            /// Sets the array using a managed array
            /// </summary>
            /// <param name="v">The array to marshal</param>
            /// <returns>This array representation</returns>
            public bh_data_array Set(bh_int64[] v) { this.int64 = v; return this; }
            /// <summary>
            /// Sets the array using a managed array
            /// </summary>
            /// <param name="v">The array to marshal</param>
            /// <returns>This array representation</returns>
            public bh_data_array Set(bh_uint8[] v) { this.uint8 = v; return this; }
            /// <summary>
            /// Sets the array using a managed array
            /// </summary>
            /// <param name="v">The array to marshal</param>
            /// <returns>This array representation</returns>
            public bh_data_array Set(bh_uint16[] v) { this.uint16 = v; return this; }
            /// <summary>
            /// Sets the array using a managed array
            /// </summary>
            /// <param name="v">The array to marshal</param>
            /// <returns>This array representation</returns>
            public bh_data_array Set(bh_uint32[] v) { this.uint32 = v; return this; }
            /// <summary>
            /// Sets the array using a managed array
            /// </summary>
            /// <param name="v">The array to marshal</param>
            /// <returns>This array representation</returns>
            public bh_data_array Set(bh_uint64[] v) { this.uint64 = v; return this; }
            /// <summary>
            /// Sets the array using a managed array
            /// </summary>
            /// <param name="v">The array to marshal</param>
            /// <returns>This array representation</returns>
            public bh_data_array Set(bh_float32[] v) { this.float32 = v; return this; }
            /// <summary>
            /// Sets the array using a managed array
            /// </summary>
            /// <param name="v">The array to marshal</param>
            /// <returns>This array representation</returns>
            public bh_data_array Set(bh_float64[] v) { this.float64 = v; return this; }
            /// <summary>
            /// Sets the array using a managed array
            /// </summary>
            /// <param name="v">The array to marshal</param>
            /// <returns>This array representation</returns>
            public bh_data_array Set(bh_complex64[] v) { this.complex64 = v; return this; }
            /// <summary>
            /// Sets the array using a managed array
            /// </summary>
            /// <param name="v">The array to marshal</param>
            /// <returns>This array representation</returns>
            public bh_data_array Set(bh_complex128[] v) { this.complex128 = v; return this; }
            /// <summary>
            /// Sets the array using a managed array
            /// </summary>
            /// <param name="v">The array to marshal</param>
            /// <returns>This array representation</returns>
            public bh_data_array Set(IntPtr v) { this.voidPtr = v; return this; }
            /// <summary>
            /// Sets the array using a managed array
            /// </summary>
            /// <param name="v">The array to marshal</param>
            /// <returns>This array representation</returns>
            public bh_data_array Set(object v) { throw new NotSupportedException(); }
        }

        /// <summary>
        /// A Bohrium component
        /// </summary>
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 0)]
        public struct bh_component
        {
            /// <summary>
            /// The name of the component
            /// </summary>
            [MarshalAs(UnmanagedType.ByValArray, SizeConst=BH_COMPONENT_NAME_SIZE)]
            public byte[] name;
            /// <summary>
            /// The copmponent configuration dictionary
            /// </summary>
            public IntPtr config;
            /// <summary>
            /// A handle to the dll/so that implements the component
            /// </summary>
            public IntPtr lib_handle;
            /// <summary>
            /// The component type
            /// </summary>
            public bh_component_type type;
            /// <summary>
            /// The initialization function
            /// </summary>
            public bh_init init;
            /// <summary>
            /// The shutdown function
            /// </summary>
            public bh_shutdown shutdown;
            /// <summary>
            /// The execute function
            /// </summary>
            public bh_execute execute;
            /// <summary>
            /// The userfunc registration function
            /// </summary>
            public bh_reg_func reg_func;
#if DEBUG
            /// <summary>
            /// Converts the Asciiz name to a string, used for debugging only
            /// </summary>
            public string Name { get { return System.Text.Encoding.ASCII.GetString(this.name.TakeWhile(b => !b.Equals(0)).ToArray()); } }
#endif
        }
        
		/// <summary>
		/// Fake wrapper struct to keep a pointer to bh_ir typesafe
		/// </summary>
		[StructLayout(LayoutKind.Explicit, CharSet = CharSet.Ansi, Pack = 0)]
		    public struct bh_ir_ptr
		{
			/// <summary>
			/// The actual IntPtr value
			/// </summary>
			[FieldOffset(0)]
			internal IntPtr m_ptr;
			
			/// <summary>
			/// A value that represents a null pointer
			/// </summary>
			public static readonly bh_ir_ptr Null = new bh_ir_ptr() { m_ptr = IntPtr.Zero };
			
			/// <summary>
			/// Custom equals functionality
			/// </summary>
			/// <param name="obj">The object to compare to</param>
			/// <returns>True if the objects are equal, false otherwise</returns>
			public override bool Equals(object obj)
			{
				if (obj is bh_ir_ptr)
					return ((bh_ir_ptr)obj).m_ptr == this.m_ptr;
				else
					return base.Equals(obj);
			}
			
			/// <summary>
			/// Custom hashcode functionality
			/// </summary>
			/// <returns>The hash code for this instance</returns>
			public override bh_int32 GetHashCode()
			{
				return m_ptr.GetHashCode();
			}
			
			/// <summary>
			/// Simple compare operator for pointer type
			/// </summary>
			/// <param name="a">One argument</param>
			/// <param name="b">Another argument</param>
			/// <returns>True if the arguments are the same, false otherwise</returns>
			public static bool operator ==(bh_ir_ptr a, bh_ir_ptr b)
			{
				return a.m_ptr == b.m_ptr;
			}
			
			/// <summary>
			/// Simple compare operator for pointer type
			/// </summary>
			/// <param name="a">One argument</param>
			/// <param name="b">Another argument</param>
			/// <returns>False if the arguments are the same, true otherwise</returns>
			public static bool operator !=(bh_ir_ptr a, bh_ir_ptr b)
			{
				return a.m_ptr != b.m_ptr;
			}
		}
			

        /// <summary>
        /// Fake wrapper struct to keep a pointer to bh_array typesafe
        /// </summary>
        [StructLayout(LayoutKind.Explicit, CharSet = CharSet.Ansi, Pack = 0)]
        public struct bh_base_ptr
        {
            /// <summary>
            /// The actual IntPtr value
            /// </summary>
            [FieldOffset(0)]
            internal IntPtr m_ptr;
            
            internal bh_base_ptr(IntPtr ptr)
            {
                m_ptr = ptr;
            }
            
            /// <summary>
            /// Accessor methods to read/write the data pointer
            /// </summary>
            public IntPtr Data
            {
                get
                {
                    if (m_ptr == IntPtr.Zero)
                        throw new ArgumentNullException();

                    return Marshal.ReadIntPtr(m_ptr, BH_TYPE_SIZE + BH_INDEX_SIZE);
                }
                set
                {
                    if (m_ptr == IntPtr.Zero)
                        throw new ArgumentNullException();
                        
                    Marshal.WriteIntPtr(m_ptr, BH_TYPE_SIZE + BH_INDEX_SIZE, value);
                }
            }

            /// <summary>
            /// Gets the type of the array
            /// </summary>
            public bh_type Type
            {
            	get
            	{
					if (m_ptr == IntPtr.Zero)
						throw new ArgumentNullException();

					return (bh_type)Marshal.ReadInt64(m_ptr, 0);
            	}
                set
                {
                    if (m_ptr == IntPtr.Zero)
                        throw new ArgumentNullException();
                        
                    Marshal.WriteInt64(m_ptr, 0, (long)value);
                }
            }

            /// <summary>
            /// Gets the type of the array
            /// </summary>
            public bh_index Length
            {
                get
                {
                    if (m_ptr == IntPtr.Zero)
                        throw new ArgumentNullException ();

                    return (bh_index)Marshal.ReadInt64 (m_ptr, BH_TYPE_SIZE);
                }
                set
                {
                    if (m_ptr == IntPtr.Zero)
                        throw new ArgumentNullException();
                        
                    Marshal.WriteInt64(m_ptr, BH_TYPE_SIZE, value);
                }
            }

			/// <summary>
			/// Gets the ptr value.
			/// </summary>
			public long PtrValue {
				get { return m_ptr.ToInt64(); }
			}

            /// <summary>
            /// A value that represents a null pointer
            /// </summary>
            public static readonly bh_base_ptr Null = new bh_base_ptr() { m_ptr = IntPtr.Zero };

            /// <summary>
            /// Free's the array view, but does not de-reference it with the VEM
            /// </summary>
            public void Free()
            {
                if (m_ptr == IntPtr.Zero)
                    return;

                bh_component_free_ptr(m_ptr);
                m_ptr = IntPtr.Zero;
            }

            /// <summary>
            /// Custom equals functionality
            /// </summary>
            /// <param name="obj">The object to compare to</param>
            /// <returns>True if the objects are equal, false otherwise</returns>
            public override bool Equals(object obj)
            {
                if (obj is bh_base_ptr)
                    return ((bh_base_ptr)obj).m_ptr == this.m_ptr;
                else
                    return base.Equals(obj);
            }

            /// <summary>
            /// Custom hashcode functionality
            /// </summary>
            /// <returns>The hash code for this instance</returns>
            public override bh_int32 GetHashCode()
            {
                return m_ptr.GetHashCode();
            }

            /// <summary>
            /// Simple compare operator for pointer type
            /// </summary>
            /// <param name="a">One argument</param>
            /// <param name="b">Another argument</param>
            /// <returns>True if the arguments are the same, false otherwise</returns>
            public static bool operator ==(bh_base_ptr a, bh_base_ptr b)
            {
                return a.m_ptr == b.m_ptr;
            }

            /// <summary>
            /// Simple compare operator for pointer type
            /// </summary>
            /// <param name="a">One argument</param>
            /// <param name="b">Another argument</param>
            /// <returns>False if the arguments are the same, true otherwise</returns>
            public static bool operator !=(bh_base_ptr a, bh_base_ptr b)
            {
                return a.m_ptr != b.m_ptr;
            }

            /// <summary>
            /// Returns a human readable string representation of the pointer
            /// </summary>
            /// <returns>A human readable string representation of the pointer</returns>
            public override string ToString()
            {
                return string.Format("(self: {0}, data: {1})", m_ptr, m_ptr == IntPtr.Zero ? "null" : this.Data.ToString());
            }
        }
        
        /// <summary>
        /// Representation of a Bohrium view
        /// </summary>
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 0)]
        public struct bh_view
        {
            /// <summary>
            /// A pointer to the base array
            /// </summary>
            public bh_base_ptr basearray;
            /// <summary>
            /// The number of dimensions in the array
            /// </summary>
            public bh_intp ndim;
            /// <summary>
            /// The data offset
            /// </summary>
            public bh_index start;
            /// <summary>
            /// The dimension sizes
            /// </summary>
            [MarshalAs(UnmanagedType.ByValArray, SizeConst=BH_MAXDIM)]
            public bh_index[] shape;
            /// <summary>
            /// The dimension strides
            /// </summary>
            [MarshalAs(UnmanagedType.ByValArray, SizeConst=BH_MAXDIM)]
            public bh_index[] stride;
            
            /// <summary>
            /// Initializes a new <see cref="NumCIL.Bohrium.PInvoke+bh_view"/> struct.
            /// </summary>
            /// <param name="ptr">Pointer to the base that this view represents</param>
            /// <param name="shape">The shape that this view represents</param>
            public bh_view(bh_base_ptr ptr, Shape shape)
            {
                this.basearray = ptr;
                this.ndim = shape.Dimensions.Length;
                this.start = shape.Offset;
                this.shape = shape.Dimensions.Select(x => x.Length).ToArray();
                this.stride = shape.Dimensions.Select(x => x.Stride).ToArray();
            }
            
            /// <summary>
            /// An empty view instance
            /// </summary>
            public static readonly bh_view EMPTY_VIEW = new bh_view();
        }
        
        /// <summary>
        /// Representation of a Bohrium base array
        /// </summary>
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 0)]
        public struct bh_base
        {
            /// <summary>
            /// The type of data in the array
            /// </summary>
            public bh_type type;
            /// <summary>
            /// The number of elements in the array
            /// </summary>
            public bh_intp nelem;
            /// <summary>
            /// The pointer to data
            /// </summary>
            public bh_data_ptr data;
        }
        /// <summary>
        /// This struct is used to allow us to pass a pointer to different struct types,
        /// because we cannot use inheritance for the bh_userfunc structure to
        /// support the reduce structure. Downside is that the size of the struct
        /// will always be the size of the largest one
        /// </summary>
        [StructLayout(LayoutKind.Explicit, CharSet = CharSet.Ansi, Pack = 0)]
        public struct bh_userfunc_union
        {
            /// <summary>
            /// The plain userfunc
            /// </summary>
            [FieldOffset(0)]
            public bh_userfunc_plain plain;

            /// <summary>
            /// The random userfunc
            /// </summary>
            [FieldOffset(0)]
            public bh_userfunc_random random;

            /// <summary>
            /// The matmul userfunc
            /// </summary>
            [FieldOffset(0)]
            public bh_userfunc_matmul matmul;

            /// <summary>
            /// Constructs a new union representing a plain userfunc
            /// </summary>
            /// <param name="arg">The user defined function</param>
            public bh_userfunc_union(bh_userfunc_plain arg) : this() { plain = arg; }
            /// <summary>
            /// Constructs a new union representing a random userfunc
            /// </summary>
            /// <param name="arg">The user defined function</param>
            public bh_userfunc_union(bh_userfunc_random arg) : this() { random = arg; }
            /// <summary>
            /// Constructs a new union representing a matmul userfunc
            /// </summary>
            /// <param name="arg">The user defined function</param>
            public bh_userfunc_union(bh_userfunc_matmul arg) : this() { matmul = arg; }

            /// <summary>
            /// Implicit operator for creating a union with a plain userfunc
            /// </summary>
            /// <param name="arg">The userfunc</param>
            /// <returns>The union userfunc</returns>
            public static implicit operator bh_userfunc_union(bh_userfunc_plain arg) { return new bh_userfunc_union(arg); }
            /// <summary>
            /// Implicit operator for creating a union with a random userfunc
            /// </summary>
            /// <param name="arg">The userfunc</param>
            /// <returns>The union userfunc</returns>
            public static implicit operator bh_userfunc_union(bh_userfunc_random arg) { return new bh_userfunc_union(arg); }
            /// <summary>
            /// Implicit operator for creating a union with a matmul userfunc
            /// </summary>
            /// <param name="arg">The userfunc</param>
            /// <returns>The union userfunc</returns>
            public static implicit operator bh_userfunc_union(bh_userfunc_matmul arg) { return new bh_userfunc_union(arg); }
        }

        /// <summary>
        /// The random userfunc
        /// </summary>
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 0)]
        public struct bh_userfunc_random
        {
            /// <summary>
            /// The random function id
            /// </summary>
            public bh_intp id;
            /// <summary>
            /// The number of output elements
            /// </summary>
            public bh_intp nout;
            /// <summary>
            /// The number of input elements
            /// </summary>
            public bh_intp nin;
            /// <summary>
            /// The total size of this struct
            /// </summary>
            public bh_intp struct_size;
            /// <summary>
            /// The output operand
            /// </summary>
            public bh_view operand;

            /// <summary>
            /// Creates a new random userfunc
            /// </summary>
            /// <param name="func">The random function id</param>
            /// <param name="op">The output operand</param>
            public bh_userfunc_random(bh_intp func, bh_view op)
            {
                this.id = func;
                this.nout = 1;
                this.nin = 0;
                this.struct_size = RANDOMFUNC_SIZE;
                this.operand = op;
            }
        }

        /// <summary>
        /// The matmul userfunc
        /// </summary>
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 0)]
        public struct bh_userfunc_matmul
        {
            /// <summary>
            /// The matmul function id
            /// </summary>
            public bh_intp id;
            /// <summary>
            /// The number of output operands
            /// </summary>
            public bh_intp nout;
            /// <summary>
            /// The number of input operands
            /// </summary>
            public bh_intp nin;
            /// <summary>
            /// The total size of this struct
            /// </summary>
            public bh_intp struct_size;
            /// <summary>
            /// The output operand
            /// </summary>
            public bh_view operand0;
            /// <summary>
            /// An input operand
            /// </summary>
            public bh_view operand1;
            /// <summary>
            /// Another input operand
            /// </summary>
            public bh_view operand2;

            /// <summary>
            /// Constructs a new matmul userfunc
            /// </summary>
            /// <param name="func">The matmul function id</param>
            /// <param name="op1">The output operand</param>
            /// <param name="op2">An input operand</param>
            /// <param name="op3">Another input operand</param>
            public bh_userfunc_matmul(bh_intp func, bh_view op1, bh_view op2, bh_view op3)
            {
                this.id = func;
                this.nout = 1;
                this.nin = 2;
                this.struct_size = MATMULFUNC_SIZE;
                this.operand0 = op1;
                this.operand1 = op2;
                this.operand2 = op3;
            }
        }

        /// <summary>
        /// A plain userfunc
        /// </summary>
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 0)]
        public struct bh_userfunc_plain
        {
            /// <summary>
            /// The function id
            /// </summary>
            public bh_intp id;
            /// <summary>
            /// The number of output operands
            /// </summary>
            public bh_intp nout;
            /// <summary>
            /// The number of input operands
            /// </summary>
            public bh_intp nin;
            /// <summary>
            /// The total size of the struct
            /// </summary>
            public bh_intp struct_size;
            /// <summary>
            /// The output operand
            /// </summary>
            public bh_view operand0;
            /// <summary>
            /// An input operand
            /// </summary>
            public bh_view operand1;
            /// <summary>
            /// Another input operand
            /// </summary>
            public bh_view operand2;

            /// <summary>
            /// Creates a new plain userfunc
            /// </summary>
            /// <param name="func">The function id</param>
            /// <param name="op">The output operand</param>
            public bh_userfunc_plain(bh_intp func, bh_view op)
            {
                this.id = func;
                this.nout = 1;
                this.nin = 0;
                this.struct_size = PLAINFUNC_SIZE;
                this.operand0 = op;
                this.operand1 = bh_view.EMPTY_VIEW;
                this.operand2 = bh_view.EMPTY_VIEW;
            }

            /// <summary>
            /// Creates a new plain userfunc
            /// </summary>
            /// <param name="func">The function id</param>
            /// <param name="op1">The output operand</param>
            /// <param name="op2">The input operand</param>
            public bh_userfunc_plain(bh_intp func, bh_view op1, bh_view op2)
            {
                this.id = func;
                this.nout = 1;
                this.nin = 0;
                this.struct_size = PLAINFUNC_SIZE;
                this.operand0 = op1;
                this.operand1 = op2;
                this.operand2 = bh_view.EMPTY_VIEW;
            }

            /// <summary>
            /// Creates a new plain userfunc
            /// </summary>
            /// <param name="func">The function id</param>
            /// <param name="op1">The output operand</param>
            /// <param name="op2">An input operand</param>
            /// <param name="op3">Another input operand</param>
            public bh_userfunc_plain(bh_intp func, bh_view op1, bh_view op2, bh_view op3)
            {
                this.id = func;
                this.nout = 1;
                this.nin = 0;
                this.struct_size = PLAINFUNC_SIZE;
                this.operand0 = op1;
                this.operand1 = op2;
                this.operand2 = op3;
            }
        }

        /// <summary>
        /// Represents a Bohrium instruction
        /// </summary>
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 0)]
        public struct bh_instruction : IInstruction
        {
            /// <summary>
            /// The instruction opcode
            /// </summary>
            public bh_opcode opcode;
            /// <summary>
            /// The output operand
            /// </summary>
            public bh_view operand0;
            /// <summary>
            /// An input operand
            /// </summary>
            public bh_view operand1;
            /// <summary>
            /// Another input operand
            /// </summary>
            public bh_view operand2;
            /// <summary>
            /// A constant value assigned to the instruction
            /// </summary>
            public bh_constant constant;
            /// <summary>
            /// Points to the user-defined function when the opcode is BH_USERFUNC
            /// </summary>
            public IntPtr userfunc;

            /// <summary>
            /// Creates a new instruction
            /// </summary>
            /// <param name="opcode">The opcode for the operation</param>
            /// <param name="operand">The output operand</param>
            /// <param name="constant">An optional constant</param>
            public bh_instruction(bh_opcode opcode, bh_base_ptr operand)
            {
                this.opcode = opcode;
                this.operand0 = new bh_view(operand, new Shape(operand.Length));
                this.operand1 = bh_view.EMPTY_VIEW;
                this.operand2 = bh_view.EMPTY_VIEW;
                this.userfunc = IntPtr.Zero;
                this.constant = new bh_constant();
            }
            
            /// <summary>
            /// Creates a new instruction
            /// </summary>
            /// <param name="opcode">The opcode for the operation</param>
            /// <param name="operand">The output operand</param>
            /// <param name="constant">An optional constant</param>
            public bh_instruction(bh_opcode opcode, bh_view operand, PInvoke.bh_constant constant = new PInvoke.bh_constant())
            {
                this.opcode = opcode;
                this.operand0 = operand;
                this.operand1 = bh_view.EMPTY_VIEW;
                this.operand2 = bh_view.EMPTY_VIEW;
                this.userfunc = IntPtr.Zero;
                this.constant = constant;
            }

            /// <summary>
            /// Creates a new instruction
            /// </summary>
            /// <param name="opcode">The opcode for the operation</param>
            /// <param name="operand1">The output operand</param>
            /// <param name="constant">A left-hand-side constant</param>
            /// <param name="operand2">An input operand</param>
            public bh_instruction(bh_opcode opcode, bh_view operand1, PInvoke.bh_constant constant, bh_view operand2)
            {
                this.opcode = opcode;
                this.operand0 = operand1;
                this.operand1 = bh_view.EMPTY_VIEW;
                this.operand2 = operand2;
                this.userfunc = IntPtr.Zero;
                this.constant = constant;
            }

            /// <summary>
            /// Creates a new instruction
            /// </summary>
            /// <param name="opcode">The opcode for the operation</param>
            /// <param name="operand1">The output operand</param>
            /// <param name="operand2">An input operand</param>
            /// <param name="constant">A right-hand-side constant</param>
            public bh_instruction(bh_opcode opcode, bh_view operand1, bh_view operand2, PInvoke.bh_constant constant = new PInvoke.bh_constant())
            {
                this.opcode = opcode;
                this.operand0 = operand1;
                this.operand1 = operand2;
                this.operand2 = bh_view.EMPTY_VIEW;
                this.userfunc = IntPtr.Zero;
                this.constant = constant;
            }

            /// <summary>
            /// Creates a new instruction
            /// </summary>
            /// <param name="opcode">The opcode for the operation</param>
            /// <param name="operand1">The output operand</param>
            /// <param name="operand2">An input operand</param>
            /// <param name="operand3">Another input operand</param>
            /// <param name="constant">A right-hand-side constant</param>
            public bh_instruction(bh_opcode opcode, bh_view operand1, bh_view operand2, bh_view operand3, PInvoke.bh_constant constant = new PInvoke.bh_constant())
            {
                this.opcode = opcode;
                this.operand0 = operand1;
                this.operand1 = operand2;
                this.operand2 = operand3;
                this.userfunc = IntPtr.Zero;
                this.constant = constant;
            }

            /// <summary>
            /// Creates a new instruction
            /// </summary>
            /// <param name="opcode">The opcode for the operation</param>
            /// <param name="operands">A list of operands</param>
            /// <param name="constant">A constant</param>
            public bh_instruction(bh_opcode opcode, IEnumerable<bh_view> operands, PInvoke.bh_constant constant = new PInvoke.bh_constant())
            {
                this.opcode = opcode;
                var en = operands.GetEnumerator();
                if (en.MoveNext())
                {
                    this.operand0 = en.Current;
                    if (en.MoveNext())
                    {
                        this.operand1 = en.Current;
                        if (en.MoveNext())
                            this.operand2 = en.Current;
                        else
                            this.operand2 = bh_view.EMPTY_VIEW;
                    }
                    else
                    {
                        this.operand1 = bh_view.EMPTY_VIEW;
                        this.operand2 = bh_view.EMPTY_VIEW;
                    }
                }
                else
                {
                    this.operand0 = bh_view.EMPTY_VIEW;
                    this.operand1 = bh_view.EMPTY_VIEW;
                    this.operand2 = bh_view.EMPTY_VIEW;
                }
                this.userfunc = IntPtr.Zero;
                this.constant = constant;
            }

            /// <summary>
            /// Constructs a userdefined instruction
            /// </summary>
            /// <param name="opcode">The opcode BH_USERFUNC</param>
            /// <param name="userfunc">A pointer to the userfunc struct</param>
            public bh_instruction(bh_opcode opcode, IntPtr userfunc)
            {
                this.opcode = opcode;
                this.userfunc = userfunc;
                this.operand0 = bh_view.EMPTY_VIEW;
                this.operand1 = bh_view.EMPTY_VIEW;
                this.operand2 = bh_view.EMPTY_VIEW;
                this.constant = new bh_constant();
            }

            /// <summary>
            /// Returns a human readable representation of the instruction
            /// </summary>
            /// <returns>A human readable representation of the instruction</returns>
            public override string ToString()
            {
                return string.Format("{0}({1}, {2}, {3})", this.opcode, operand0, operand1, operand2);
            }

			/// <summary>
			/// Gets the opcode.
			/// </summary>
            bh_opcode IInstruction.OpCode
            {
                get { return opcode; }
            }

            /// <summary>
            /// Gets the userfunc id, number of output operands and number of input operands
            /// </summary>
            public Tuple<long, long, long> UserfuncIdNOutNIn
            {
            	get
            	{
            		if (this.userfunc == IntPtr.Zero)
            			return null;

            		if (Is64Bit)
            		{
            			return new Tuple<long, long, long>(
            				Marshal.ReadInt64(this.userfunc, 0),
							Marshal.ReadInt64(this.userfunc, 8),
							Marshal.ReadInt64(this.userfunc, 16)
						);
            		}
            		else
            		{
						return new Tuple<long, long, long>(
							Marshal.ReadInt32(this.userfunc, 0),
							Marshal.ReadInt32(this.userfunc, 4),
							Marshal.ReadInt32(this.userfunc, 8)
							);
					}
            	}
            }

            /// <summary>
            /// Gets the userfunc arrays.
            /// </summary>
            public bh_base_ptr[] UserfuncArrays
            {
            	get
            	{
            		var tp = this.UserfuncIdNOutNIn;
            		if (tp == null)
            			return null;

            		var nops = tp.Item2 + tp.Item3;
            		var arrays = new bh_base_ptr[nops];
            		for(var i = 0; i < nops; i++)
            			arrays[i].m_ptr = Marshal.ReadIntPtr(this.userfunc, (4 + i) * IntPtr.Size);

            		return arrays;
            	}
            }
        }
        
        public static bh_base_ptr bh_create_base(bh_type type, bh_index size)
        {
            var ptr = new bh_base_ptr(Marshal.AllocHGlobal(BH_BASE_SIZE));
            ptr.Type = type;
            ptr.Length = size;
            ptr.Data = IntPtr.Zero;
            return ptr;
        }

        public static void bh_destroy_base(bh_base_ptr ptr)
        {
            ptr.Length = 0;
            ptr.Data = IntPtr.Zero;
            Marshal.FreeHGlobal(ptr.m_ptr);
            ptr.m_ptr = IntPtr.Zero;
        }
        
        /// <summary>
        /// Delegate for initializing a Bohrium component
        /// </summary>
        /// <param name="self">An allocated component struct that gets filled with data</param>
        /// <returns>A status code</returns>
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate bh_error bh_init(ref bh_component self);
        /// <summary>
        /// Delegate for shutting down a component
        /// </summary>
        /// <returns>A status code</returns>
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate bh_error bh_shutdown();
        /// <summary>
        /// Delegate for execution instructions
        /// </summary>
        /// <param name="count">The number of instructions to execute</param>
        /// <param name="inst_list">The list of instructions to execute</param>
        /// <returns>A status code</returns>
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate bh_error bh_execute(bh_ir_ptr ir);
        /// <summary>
        /// Register a userfunc
        /// </summary>
        /// <param name="fun">The name of the function to register</param>
        /// <param name="id">The id assigned</param>
        /// <returns>A status code</returns>
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate bh_error bh_reg_func(string fun, ref bh_intp id);

        /// <summary>
        /// Setup the root component, which normally is the bridge.
        /// </summary>
        /// <param name="name">The component name</param>
        /// <returns>A new component object</returns>
        [DllImport("libbh", EntryPoint = "bh_component_setup", CallingConvention = CallingConvention.Cdecl, SetLastError = true, CharSet = CharSet.Auto)]
        private extern static IntPtr bh_component_setup_masked(string name);

        /// <summary>
        /// Setup the root component, which normally is the bridge.
        /// </summary>
        /// <returns>A new component object</returns>
        public static bh_component bh_component_setup(out IntPtr unmanaged)
        {
            unmanaged = bh_component_setup_masked(null);
            bh_component r = (bh_component)Marshal.PtrToStructure(unmanaged, typeof(bh_component));
			return r;
        }

        /// <summary>
        /// Retrieves the children components of the parent.
        /// NB: the array and all the children should be free'd by the caller
        /// </summary>
        /// <param name="parent">The parent component (input)</param>
        /// <param name="count">Number of children components</param>
        /// <param name="children">Array of children components (output)</param>
        /// <returns>Error code (BH_SUCCESS)</returns>
        [DllImport("libbh", EntryPoint = "bh_component_children", CallingConvention = CallingConvention.Cdecl, SetLastError = true, CharSet = CharSet.Auto)]
        private extern static bh_error bh_component_children_masked([In] ref bh_component parent, [Out] out bh_intp count, [Out] out IntPtr children);

        /// <summary>
        /// Retrieves the children components of the parent.
        /// NB: the array and all the children should be free'd by the caller
        /// </summary>
        /// <param name="parent">The parent component (input)</param>
        /// <param name="unmanagedData">Unmanaged data</param>
        /// <param name="children">Array of children components (output)</param>
        /// <returns>Error code (BH_SUCCESS)</returns>
        public static bh_error bh_component_children(bh_component parent, out bh_component[] children, out IntPtr unmanagedData)
        {
            //TODO: Errors in setup may cause memory leaks, but we should terminate anyway

            long count = 0;
            children = null;

            bh_error e = bh_component_children_masked(ref parent, out count, out unmanagedData);
            if (e != bh_error.BH_SUCCESS)
                return e;

            children = new bh_component[count];
            for (int i = 0; i < count; i++)
            {
                IntPtr cur = Marshal.ReadIntPtr(unmanagedData, Marshal.SizeOf(typeof(bh_intp)) * i);
                children[i] = (bh_component)Marshal.PtrToStructure(cur, typeof(bh_component));
            }

            return e;
        }


        /// <summary>
        /// Frees the component
        /// </summary>
        /// <param name="component">The component to free</param>
        /// <returns>Error code (BH_SUCCESS)</returns>
        [DllImport("libbh", SetLastError = true, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Auto)]
        public extern static bh_error bh_component_free([In] ref bh_component component);

        /// <summary>
        /// Frees the component
        /// </summary>
        /// <param name="component">The component to free</param>
        /// <returns>Error code (BH_SUCCESS)</returns>
        [DllImport("libbh", SetLastError = true, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Auto)]
        public extern static bh_error bh_component_free(IntPtr component);

        /// <summary>
        /// Frees the component
        /// </summary>
        /// <param name="component">The component to free</param>
        /// <returns>Error code (BH_SUCCESS)</returns>
        [DllImport("libbh", SetLastError = true, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Auto)]
        public extern static bh_error bh_component_free_ptr([In] IntPtr component);

        /// <summary>
        /// Retrieves an user-defined function
        /// </summary>
        /// <param name="self">The component</param>
        /// <param name="func">Name of the function e.g. myfunc</param>
        /// <param name="ret_func">Pointer to the function (output), Is NULL if the function doesn't exist</param>
        /// <returns>Error codes (BH_SUCCESS)</returns>
        [DllImport("libbh", SetLastError = true, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Auto)]
        public extern static bh_error bh_component_get_func([In] ref bh_component self, [In] string func,
                               [Out] IntPtr ret_func);

        /// <summary>
        /// Set the data pointer for the array.
        /// Can only set to non-NULL if the data ptr is already NULL
        /// </summary>
        /// <param name="array">The array in question</param>
        /// <returns>Error code (BH_SUCCESS, BH_ERROR)</returns>
        [DllImport("libbh", SetLastError = true, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Auto)]
        public extern static bh_error bh_data_malloc([In] bh_base_ptr array);

        /// <summary>
        /// Set the data pointer for the array.
        /// Can only set to non-NULL if the data ptr is already NULL
        /// </summary>
        /// <param name="array">The array in question</param>
        /// <returns>Error code (BH_SUCCESS, BH_ERROR)</returns>
        [DllImport("libbh", SetLastError = true, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Auto)]
        public extern static bh_error bh_data_free([In] bh_base_ptr array);

		/// <summary>
		/// Validates the given types for the operation and returns true if the operation is supported with the given types, and returns false otherwise
		/// </summary>
		/// <param name="opcode">The operation to check</param>
		/// <param name="outtype">The output type of the operation</param>
		/// <param name="inputtype1">One inputtype</param>
		/// <param name="inputtype2">The other inputtype</param>
		/// <param name="constanttype">The constant type</param>
		[DllImport("libbh", SetLastError = true, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Auto)]
		public extern static bool bh_validate_types(bh_opcode opcode, bh_type outtype, bh_type inputtype1, bh_type inputtype2, bh_type constanttype);

		/// <summary>
		/// Attempts to convert the inputtypes to support the operation.
		/// Returns true if there is a valid conversion, and false otherwise.
		/// If there is a possible conversion the types will be updated to the desired types,
		/// and these types can be used with IDENTITY to perform the conversion
		/// </summary>
		/// <param name="opcode">The operation to check</param>
		/// <param name="outtype">The output type of the operation</param>
		/// <param name="inputtype1">One inputtype</param>
		/// <param name="inputtype2">The other inputtype</param>
		/// <param name="constanttype">The constant type</param>
		[DllImport("libbh", SetLastError = true, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Auto)]
		public extern static bool bh_get_type_conversion(bh_opcode opcode, bh_type outtype, ref bh_type inputtype1, ref bh_type inputtype2, ref bh_type constanttype);

		/// <summary>
		/// Gets the number of operands required for the opcode
		/// </summary>
		/// <param name="opcode">The Opcode to query</param>
		[DllImport("libbh", SetLastError = true, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Auto)]
		public extern static int bh_operands(bh_opcode opcode);

		/// <summary>
		/// Gets the number of operands required for the opcode
		/// </summary>
		/// <param name="inst">The instruction to examine</param>
		[DllImport("libbh", SetLastError = true, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Auto)]
		public extern static int bh_operands_in_instruction(bh_instruction inst);

		/// <summary>
		/// Creates a new graph storage element
 		/// </summary>
		/// <param name="bhir">A pointer to the result</param>
		/// <param name="instructions">The initial instruction list, can be null if instruction_count is 0</param>
 		/// <param name="instruction_count">The number of instructions in the list</param>
		[DllImport("libbh", SetLastError = true, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Auto)]
		public extern static bh_error bh_ir_create(bh_ir_ptr bhir, bh_intp instruction_count, bh_instruction[] instructions);
		
		/// <summary>
		/// Destroys the instance and releases all resources
		/// </summary>
		/// <param name="bhir">The bh_ir instance to destroy</param>
		[DllImport("libbh", SetLastError = true, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Auto)]
		public extern static void bh_ir_destroy(bh_ir_ptr bhir);
        
        /// <summary>
        /// Allocates space for a bh_ir struct, used to allow bridges to create the struct without knowing the size of the struct
        /// </summary>        
        [DllImport("libbh", SetLastError = true, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Auto)]
        public extern static bh_ir_ptr bh_ir_malloc();      

        /// <summary>
        /// Allocates space for a bh_ir struct, used to allow bridges to create the struct without knowing the size of the struct
        /// </summary>
        /// <param name="bhir">The pointer to free</param>>
        [DllImport("libbh", SetLastError = true, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Auto)]
        public extern static void bh_ir_free(bh_ir_ptr bhir);
	}
}

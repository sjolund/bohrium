#define VIEW_BOOK_KEEPING

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
using NumCIL.Generic;
using System.Runtime.InteropServices;

namespace NumCIL.Bohrium
{
    /// <summary>
    /// Basic wrapper implementation of a Bohrium VEM
    /// </summary>
    public class VEM : IDisposable
    {
        /// <summary>
        /// Singleton VEM instance
        /// </summary>
        private static VEM _instance = null;

        /// <summary>
        /// Accessor for singleton VEM instance
        /// </summary>
        public static VEM Instance
        {
            get
            {
                if (_instance == null)
                    _instance = new VEM();

                return _instance;
            }
        }
        
        /// <summary>
        /// Lock object for ensuring single threaded access to the VEM
        /// </summary>
        private object m_executelock = new object();

        /// <summary>
        /// Lock object for protecting the release queue
        /// </summary>
        private object m_releaselock = new object();

        /// <summary>
        /// A reference to the Bohrium component for "self" aka the bridge
        /// </summary>
        private PInvoke.bh_component m_component;
        /// <summary>
        /// The unmanaged copy of the component
        /// </summary>
        private IntPtr m_componentPtr;
        /// <summary>
        /// A reference to the chpVB VEM
        /// </summary>
        private PInvoke.bh_component[] m_childs;
        /// <summary>
        /// The unmanaged copy of the childs array
        /// </summary>
        private IntPtr m_childsPtr;
        
        /// <summary>
        /// The current list of instructions
        /// </summary>
        private InstructionList m_list;

        /// <summary>
        /// GC Handles for managed data
        /// </summary>
        private Dictionary<PInvoke.bh_base_ptr, GCHandle> m_managedHandles = new Dictionary<PInvoke.bh_base_ptr, GCHandle>();

		/// <summary>
		/// Gets a value indicating whether this instance is disposed.
		/// </summary>
		/// <value><c>true</c> if this instance is disposed; otherwise, <c>false</c>.</value>
		public bool IsDisposed { get; private set; }

        /// <summary>
        /// Constructs a new VEM
        /// </summary>
        public VEM()
        {
//Disable "Unreachable code" warning
#pragma warning disable 0162
            if (bh_opcode.BH_ADD == bh_opcode.BH_SUBTRACT)
                throw new Exception("This version of NumCIL.Bohrium contains invalid opcodes!");
#pragma warning restore

            m_component = PInvoke.bh_component_setup(out m_componentPtr);
            PInvoke.bh_error e = PInvoke.bh_component_children(m_component, out m_childs, out m_childsPtr);
            if (e != PInvoke.bh_error.BH_SUCCESS)
                throw new BohriumException(e);

            if (m_childs.Length > 1)
                throw new BohriumException(string.Format("Unexpected number of child nodes: {0}", m_childs.Length));

            e = m_childs[0].init(ref m_childs[0]);
            if (e != PInvoke.bh_error.BH_SUCCESS)
                throw new BohriumException(e);
        }

        /// <summary>
        /// Helper to get the id of a userdefined function
        /// </summary>
        /// <param name="name">The name of the userfunc to find</param>
        /// <returns>The ID of the userdefined function or 0</returns>
        private long GetUserFuncId(string name)
        {
            //The exception only happens with the debugger attached
            long id = 0;
            try
            {
                PInvoke.bh_error e = m_childs[0].reg_func(name, ref id);
                if (e != PInvoke.bh_error.BH_SUCCESS)
                    id = 0;
            }
            catch { id = 0; }
            
            return id;
        }

        /// <summary>
        /// Invokes garbage collection and flushes all pending cleanup messages
        /// </summary>
        public void Flush()
        {
            GC.Collect();
            Execute();
        }
        
        /// <summary>
        /// Executes a sync operation on the array
        /// </summary>
        /// <param name="array">The array to sync with</param>
        /// <typeparam name="T">The data type of the element</typeparam>
        public void Sync<T>(NdArray<T> array)
        {
            m_list.AddInstruction(bh_opcode.BH_SYNC, null, array);
        }

        /// <summary>
        /// Executes a sync operation on the array
        /// </summary>
        /// <param name="array">The array to sync with</param>
        /// <typeparam name="T">The data type of the element</typeparam>
        public void Sync<T>(IDataAccessor<T> data)
        {
            m_list.AddInstruction(bh_opcode.BH_SYNC, null, data);
            Execute();
        }

        /// <summary>
        /// Executes a sync operation on the array
        /// </summary>
        /// <param name="array">The array to sync with</param>
        /// <param name="handle">The associated GC handle, if any</param>
        /// <param name="synchronous">True if the free is executed immediately, false otherwise</param>
        /// <typeparam name="T">The data type of the element</typeparam>
        public void Free(PInvoke.bh_base_ptr ptr, GCHandle handle, bool synchronous = false)
        {
            m_list.AddInstruction(bh_opcode.BH_FREE, null, ptr);
            if (handle.IsAllocated)
                m_list.AddHandle(handle);
            if (synchronous)
                Execute();
        }

        /// <summary>
        /// Executes a sync operation on the array
        /// </summary>
        /// <param name="array">The array to sync with</param>
        /// <param name="synchronous">True if the free is executed immediately, false otherwise</param>
        /// <typeparam name="T">The data type of the element</typeparam>
        public void Free<T>(IDataAccessor<T> data, bool synchronous = false)
        {
            m_list.AddInstruction(bh_opcode.BH_FREE, null, data);
            if (synchronous)
                Execute();
        }
        
        /// <summary>
        /// Adds an instruction to the current list of instructions
        /// </summary>
        /// <param name="opcode">The opcode to add</param>
        /// <param name="opcode">The constant to assign to the operation</param>
        /// <param name="operands">The instruction operands</param>
        public void AddInstruction<T>(bh_opcode opcode, PInvoke.bh_constant? constant, params NdArray<T>[] operands) 
        {
            m_list.AddInstruction(opcode, constant, operands);
        }
        
        /// <summary>
        /// Creates a new instruction that converts from Tb to Ta
        /// </summary>
        /// <typeparam name="Ta">The output element datatype</typeparam>
        /// <typeparam name="Tb">The input element datatype</typeparam>
        /// <param name="supported">A list of accumulated instructions</param>
        /// <param name="opcode">The instruction opcode</param>
        /// <param name="typea">The Bohrium datatype for the output</param>
        /// <param name="output">The output operand</param>
        /// <param name="in1">An input operand</param>
        /// <param name="in2">Another input operand</param>
        /// <returns>A new instruction</returns>
        public void AddConversionInstruction<Ta, Tb>(NumCIL.Bohrium.bh_opcode opcode, PInvoke.bh_type typea, NdArray<Ta> output, NdArray<Tb> in1, NdArray<Tb> in2)
        {
            m_list.AddConversionInstruction(opcode, typea, output, in1, in2);
        }
        
		/// <summary>
        /// Executes a list of instructions
        /// </summary>
        /// <param name="inst_list">The list of instructions to execute</param>
        public void Execute()
        {
            var handles = m_managedHandles;
            m_managedHandles = new Dictionary<PInvoke.bh_base_ptr, GCHandle>();
            
            var instructions = m_list;
            m_list = new NumCIL.Bohrium.InstructionList();

            try
            {
                lock (m_executelock)
                    if (instructions != null)
                        ExecuteWithoutLocks(instructions);
            }
            finally
            {
                if (handles != null)
                    lock (m_releaselock)
                    {
                        foreach (var kp in handles)
                            kp.Value.Free();
                    }
            }
        }

        /// <summary>
        /// Internal execution handler, runs without locking of any kind
        /// </summary>
        /// <param name="instList">The list of instructions to execute</param>
        /// <param name="errorIndex">A return value for the instruction that caused an error</param>
        private void ExecuteWithoutLocks(InstructionList instructions)
        {
            try
            {
                using (IR batch = new IR(instructions))
                    batch.Execute(m_childs[0]);
            }
            finally
            {
                for (var i = 0L; i < instructions.Length; i++)
                {
                    bh_opcode opcode;
                    IntPtr userfunc;
                    PInvoke.bh_interop_instructionlist_get_opcode(instructions.Pointer, i, out opcode);
                    PInvoke.bh_interop_instructionlist_get_userfunc(instructions.Pointer, i, out userfunc);
                    if (opcode == bh_opcode.BH_DISCARD)
                    {
                        PInvoke.bh_base_ptr basearray;
                        PInvoke.bh_interop_instructionlist_get_operandbase(instructions.Pointer, i, 0, out basearray);
                        PInvoke.bh_destroy_base(basearray);
                    }
                    if (userfunc != IntPtr.Zero)
                        Marshal.FreeHGlobal(userfunc);                        
                }                
            }
        }
        
        /// <summary>
        /// Creates a base array with uninitialized memory
        /// </summary>
        /// <typeparam name="T">The data type for the array</typeparam>
        /// <param name="size">The size of the generated base array</param>
        /// <returns>The pointer to the base array descriptor</returns>
        public PInvoke.bh_base_ptr CreateBaseArray<T>(long size)
        {
            return CreateBaseArray(MapType(typeof(T)), size);
        }

        /// <summary>
        /// Maps the element type to the Bohrium datatype
        /// </summary>
        /// <param name="t">The element type to look up</param>
        /// <returns>The Bohrium datatype</returns>
        public static PInvoke.bh_type MapType(Type t)
        {
            if (t == typeof(bool))
                return PInvoke.bh_type.BH_BOOL;
            else if (t == typeof(sbyte))
                return PInvoke.bh_type.BH_INT8;
            else if (t == typeof(short))
                return PInvoke.bh_type.BH_INT16;
            else if (t == typeof(int))
                return PInvoke.bh_type.BH_INT32;
            else if (t == typeof(long))
                return PInvoke.bh_type.BH_INT64;
            else if (t == typeof(byte))
                return PInvoke.bh_type.BH_UINT8;
            else if (t == typeof(ushort))
                return PInvoke.bh_type.BH_UINT16;
            else if (t == typeof(uint))
                return PInvoke.bh_type.BH_UINT32;
            else if (t == typeof(ulong))
                return PInvoke.bh_type.BH_UINT64;
            else if (t == typeof(float))
                return PInvoke.bh_type.BH_FLOAT32;
            else if (t == typeof(double))
                return PInvoke.bh_type.BH_FLOAT64;
            else if (t == typeof(NumCIL.Complex64.DataType))
                return PInvoke.bh_type.BH_COMPLEX64;
            else if (t == typeof(System.Numerics.Complex))
                return PInvoke.bh_type.BH_COMPLEX128;
            else
                throw new BohriumException(string.Format("Unsupported data type: " + t.FullName));
        }

        /// <summary>
        /// Creates a Bohrium base array or view descriptor
        /// </summary>
        /// <param name="type">The Bohrium type of data in the array</param>
        /// <param name="size">The size of the base array</param>
        /// <returns>The pointer to the base array descriptor</returns>
        public PInvoke.bh_base_ptr CreateBaseArray(PInvoke.bh_type type, long size)
        {
            // We do not need to lock, 
            // but keep it in case we go back
            // to allocating the data from Bohrium
            lock (m_executelock)
                return PInvoke.bh_create_base(type, size, IntPtr.Zero);
        }

		/// <summary>
		/// Releases all resources held
		/// </summary>
		public void Dispose()
		{
			Dispose(true);
		}
		
        /// <summary>
        /// Releases all resources held
        /// </summary>
        private void Dispose(bool disposing)
		{
			if (IsDisposed)
				return;
        	
			if (disposing)
				GC.SuppressFinalize(this);
        		
			//Ensure all views are collected
			GC.Collect();
			GC.WaitForPendingFinalizers();
            
			//From here on, we no longer accept new discards
			IsDisposed = true;
			
			lock (m_executelock)
			{
				if (m_childs != null)
				{
					for (int i = 0; i < m_childs.Length; i++)
						m_childs[i].shutdown();
					
					if (m_childsPtr != IntPtr.Zero)
					{
						for (int i = 0; i < m_childs.Length; i++)
						{
							IntPtr cur = Marshal.ReadIntPtr(m_childsPtr, Marshal.SizeOf(typeof(IntPtr)) * i);
							PInvoke.bh_component_free(cur);
							cur += Marshal.SizeOf(typeof(IntPtr));
						}
						
						m_childsPtr = IntPtr.Zero;
					}
					
					m_childs = null;
				}
				
				if (m_componentPtr != IntPtr.Zero)
				{
					PInvoke.bh_component_free(m_componentPtr);
					m_component.config = IntPtr.Zero;
				}
				
				m_componentPtr = IntPtr.Zero;
			}
		}

        /// <summary>
        /// Finalizes the VEM and shuts down the Bohrium components
        /// </summary>
        ~VEM()
        {
            Dispose(false);
        }

        /// <summary>
        /// Returns a value indicating if a value is a scalar
        /// </summary>
        /// <typeparam name="T">The type of data in the array</typeparam>
        /// <param name="ar">The array to examine</param>
        /// <returns>True if the alue can be represented as a Bohrium constant, false otherwise</returns>
        internal static bool IsScalar<T>(NdArray<T> ar)
        {
            if (ar.DataAccessor.Length == 1)
                if (ar.DataAccessor.GetType() == typeof(DefaultAccessor<T>))
                    return true;
                else if (ar.DataAccessor.GetType() == typeof(BohriumAccessor<T>) && ar.DataAccessor.IsAllocated)
                    return true;

            return false;
        }
        
        /// <summary>
        /// Determines whether the instruction has valid types
        /// </summary>
        /// <returns><c>true</c> if the instruction has valid types otherwise, <c>false</c>.</returns>
        /// <param name="inst">The instruction to validate</param>
        public bool IsValidInstruction(bh_opcode opcode, PInvoke.bh_type outputtype, PInvoke.bh_type input1, PInvoke.bh_type input2, PInvoke.bh_type constantType )
		{
			var nops = PInvoke.bh_operands(opcode);
			if (nops == 1)
				return PInvoke.bh_validate_types(opcode, outputtype, PInvoke.bh_type.BH_UNKNOWN, PInvoke.bh_type.BH_UNKNOWN, PInvoke.bh_type.BH_UNKNOWN);
			
			
			if (nops == 2 || nops == 3)
				return PInvoke.bh_validate_types(opcode, outputtype, input1, input2, constantType);

			throw new Exception(string.Format("Unexpected number of operands for opcode {1}: {0}", opcode, nops));
        }
        
        /*public IInstruction[] GetConversionSequence(IInstruction inst)
		{
			var i = (PInvoke.bh_instruction)inst;
			var out_type = i.operand0.Type;
			var nops = PInvoke.bh_operands(inst.OpCode);
			if (nops == 1)
				return null;
			
			var input1 = i.operand1 == PInvoke.bh_array_ptr.Null ? PInvoke.bh_type.BH_UNKNOWN : i.operand1.Type;
			var input2 = i.operand2 == PInvoke.bh_array_ptr.Null ? PInvoke.bh_type.BH_UNKNOWN : i.operand2.Type;
			var scalar = i.constant.type;
			
			if (nops == 2 || nops == 3)
			{
				var orig1 = input1;
				var orig2 = input2;
				var origs = scalar;
				
				if(!PInvoke.bh_get_type_conversion(inst.OpCode, out_type, ref input1, ref input2, ref scalar))
					return null;
					
				var conversions = new List<IInstruction>();
				if (input1 != orig1)
					conversions.Add(CreateConversionInstruction);
				if (input2 != orig2)
					conversions.Add(CreateConversionInstruction);
				if (origs != scalar)
					throw new Exception("Scalar conversion requested to match types, this is an error in the library");
			}
			
			throw new Exception(string.Format("Unexpected number of operands for opcode {1}: {0}", inst.OpCode, nops));
		}*/
    }
}

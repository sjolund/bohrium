using System;
using NumCIL.Generic;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;

namespace NumCIL.Bohrium
{
    /// <summary>
    /// Wrapper class for keeping a reference to the externally managed instruction list
    /// </summary>
    public sealed class InstructionList : IDisposable
    {
        private long m_length = 0;
        private long m_allocated = 100;
        private PInvoke.bh_instruction_ptr m_ptr;
        private List<GCHandle> m_handles = new List<GCHandle>();
        
        /// <summary>
        /// Initializes a new instance of the <see cref="NumCIL.Bohrium.InstructionList"/> class.
        /// </summary>
        public InstructionList()
        {
            PInvoke.bh_interop_instructionlist_create(m_allocated, ref m_ptr);
        }
        
        /// <summary>
        /// Gets the number of instructions in the list
        /// </summary>
        /// <value>The number of instructions in the list</value>
        public long Length { get { return m_length; } }

        /// <summary>
        /// Accesses the internal pointer
        /// </summary>        
        internal PInvoke.bh_instruction_ptr Pointer { get { return m_ptr; } }
                                
        /// <summary>
        /// Grows the list to fit one more element
        /// </summary>
        private void GrowIfRequired()
        {
            if (m_length == m_allocated)
            {
                var newsize = m_allocated * 2;
                var e = PInvoke.bh_interop_instructionlist_resize (ref m_ptr, newsize);
                if (e != PInvoke.bh_error.BH_SUCCESS)
                    throw new BohriumException (e);
                
                m_allocated = newsize;
            }
        }
        
        public void AddInstruction(bh_opcode opcode, PInvoke.bh_constant? constant)        
        {
            GrowIfRequired();
            m_length++;
            PInvoke.bh_interop_instructionlist_set(
                m_ptr,
                m_length-1,
                opcode,
                constant.HasValue ? constant.Value : new PInvoke.bh_constant(),
                IntPtr.Zero);
                
        }
        
        public void AddHandle(GCHandle handle)
        {
            m_handles.Add(handle);
        }

        /// <summary>
        /// Creates a new instruction
        /// </summary>
        /// <typeparam name="T">The type of data used in the instruction</typeparam>
        /// <param name="opcode">The instruction opcode</param>
        /// <param name="operand">The output operand</param>
        /// <param name="constant">An optional constant value</param>
        /// <returns>The new instruction</returns>
        public void AddInstruction<T>(bh_opcode opcode, PInvoke.bh_constant? constant, IDataAccessor<T> accessor)
        {
            AddInstruction(opcode, constant);
            PInvoke.bh_interop_instructionlist_set_view(
                m_ptr,
                m_length-1,
                0,
                BaseArrayKeeper.CreateBaseArrayKeeper(accessor).BasePointer,
                1,
                0,
                new long[] { accessor.Length },
                new long[] { 1 }
            );
        }

                /// <summary>
        /// Creates a new instruction
        /// </summary>
        /// <typeparam name="T">The type of data used in the instruction</typeparam>
        /// <param name="opcode">The instruction opcode</param>
        /// <param name="operand">The output operand</param>
        /// <param name="constant">An optional constant value</param>
        /// <returns>The new instruction</returns>
        public void AddInstruction(bh_opcode opcode, PInvoke.bh_constant? constant, PInvoke.bh_base_ptr ptr)
        {
            AddInstruction(opcode, constant);
            PInvoke.bh_interop_instructionlist_set_view(
                m_ptr,
                m_length-1,
                0,
                ptr,
                1,
                0,
                new long[] { 1 },
                new long[] { 1 }
            );
        }
        
        /// <summary>
        /// Creates a new instruction
        /// </summary>
        /// <typeparam name="T">The type of data used in the instruction</typeparam>
        /// <param name="opcode">The instruction opcode</param>
        /// <param name="operand">The output operand</param>
        /// <param name="constant">An optional constant value</param>
        /// <returns>The new instruction</returns>
        public void AddInstruction<T>(bh_opcode opcode, PInvoke.bh_constant? constant, params NdArray<T>[] operands)
        {
            bool constantUsed = constant.HasValue;
            var i = 0;

            AddInstruction (opcode, constant);
            foreach (var n in operands)
            {
                if (n != null)
                {
                    if (!constantUsed && VEM.IsScalar(n))
                    {
                        constantUsed = true;
                        PInvoke.bh_interop_instructionlist_set_constant(m_ptr, m_length - 1, i, new PInvoke.bh_constant(n.DataAccessor[0]));
                    }   
                    else
                    {
                        var keeper = BaseArrayKeeper.CreateBaseArrayKeeper (n.DataAccessor);
                        PInvoke.bh_interop_instructionlist_set_view(
                            m_ptr,
                            m_length - 1,
                            i,
                            keeper.BasePointer,
                            n.Shape.Dimensions.Length,
                            n.Shape.Offset,
                            n.Shape.Dimensions.Select (x => x.Length).ToArray (),
                            n.Shape.Dimensions.Select (x => x.Stride).ToArray ()
                        );
                    }
                }
                else if (!constantUsed && constant.HasValue)
                {
                    constantUsed = true;
                    PInvoke.bh_interop_instructionlist_set_constant(m_ptr, m_length - 1, i, constant.Value);
                }
                else
                {
                    PInvoke.bh_interop_instructionlist_set_view_empty(m_ptr, m_length - 1, i);
                }
                
                i++;
            }
        }

        private void SetOperandForInstruction<T>(NdArray<T> data, int operandIndex)
        {
            PInvoke.bh_interop_instructionlist_set_view(
                m_ptr,
                m_length - 1,
                operandIndex,
                BaseArrayKeeper.CreateBaseArrayKeeper(data.DataAccessor).BasePointer,
                data.Shape.Dimensions.Length,
                data.Shape.Offset,
                data.Shape.Dimensions.Select(x => x.Length).ToArray(),
                data.Shape.Dimensions.Select(x => x.Stride).ToArray()
            );
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
            if (VEM.IsScalar(in1))
            {
                AddInstruction<Ta>(opcode, new PInvoke.bh_constant(in1.DataAccessor[0]), output.DataAccessor);
                if (in2 != null)
                    SetOperandForInstruction<Tb>(in2, 2);
            }
            else if (in2 != null && VEM.IsScalar(in2))
            {
                AddInstruction<Ta>(opcode, new PInvoke.bh_constant(in2.DataAccessor[0]), output.DataAccessor);
                if (in1 != null)
                    SetOperandForInstruction(in1, 1);
            }
            else
            {
                AddInstruction<Ta>(opcode, null, output.DataAccessor);
                if (in1 != null)
                    SetOperandForInstruction<Tb>(in1, 1);
                if (in2 != null)
                    SetOperandForInstruction<Tb>(in2, 2);
            }
        }
                                
        /// <summary>
        /// Releases all resource used by the <see cref="NumCIL.Bohrium.InstructionList"/> object.
        /// </summary>
        /// <remarks>Call <see cref="Dispose"/> when you are finished using the <see cref="NumCIL.Bohrium.InstructionList"/>. The
        /// <see cref="Dispose"/> method leaves the <see cref="NumCIL.Bohrium.InstructionList"/> in an unusable state.
        /// After calling <see cref="Dispose"/>, you must release all references to the
        /// <see cref="NumCIL.Bohrium.InstructionList"/> so the garbage collector can reclaim the memory that the
        /// <see cref="NumCIL.Bohrium.InstructionList"/> was occupying.</remarks>
        public void Dispose()
        {
            Dispose(true);
        }
        
        /// <summary>
        /// Releases unmanaged resources and performs other cleanup operations before the
        /// <see cref="NumCIL.Bohrium.InstructionList"/> is reclaimed by garbage collection.
        /// </summary>
        ~InstructionList()
        {
            Dispose(false);
        }
        
        /// <summary>
        /// Dispose the specified disposing.
        /// </summary>
        /// <param name="disposing">If set to <c>true</c> disposing.</param>
        private void Dispose(bool disposing)
        {
            if (disposing)
                GC.SuppressFinalize(this);
            
            if (m_ptr != PInvoke.bh_instruction_ptr.Null)
                try { PInvoke.bh_interop_instructionlist_destroy(m_ptr); }
                finally { m_ptr = PInvoke.bh_instruction_ptr.Null; }
        }
    }
}


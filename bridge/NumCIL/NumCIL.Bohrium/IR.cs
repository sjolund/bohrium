using System;

namespace NumCIL.Bohrium
{
	/// <summary>
	/// Wrapper class to represent the BH IR
	/// </summary>
	internal class IR : IDisposable
	{
		/// <summary>
		/// The actual IntPtr value
		/// </summary>
		private PInvoke.bh_ir_ptr m_ptr;
		
		/// <summary>
		/// Creates a new IR instance
		/// </summary>
		public IR()
			: this(null)
		{
		}

		/// <summary>
		/// Creates a new IR instance
		/// </summary>
		/// <param name="instructions">The initial instruction list in the IR batch</param>
		public IR(PInvoke.bh_instruction[] instructions)
        {
            try
            {
                m_ptr = PInvoke.bh_ir_malloc ();
                var res = PInvoke.bh_ir_create (m_ptr, instructions == null ? 0 : instructions.Length, instructions);
                if (res != PInvoke.bh_error.BH_SUCCESS)
                    throw new BohriumException (res);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
                if (m_ptr != PInvoke.bh_ir_ptr.Null)
                    PInvoke.bh_ir_free(m_ptr);
            }
		}
				
		/// <summary>
		/// Executes the current batch
		/// </summary>
		public void Execute(PInvoke.bh_component component)
		{
			var res = component.execute(m_ptr);
			if (res != PInvoke.bh_error.BH_SUCCESS)
				throw new BohriumException(res);
		}

		/// <summary>
		/// Disposes all resources
		/// </summary>
		/// <param name="disposing">Set to <c>true</c> if called from dispose, false otherwise</param>
		protected void Dispose(bool disposing)
		{
			if (m_ptr == PInvoke.bh_ir_ptr.Null)
				return;
				
			if (disposing)
				GC.SuppressFinalize(disposing);
			
			try
			{
				PInvoke.bh_ir_destroy(m_ptr);
                PInvoke.bh_ir_free(m_ptr);
			}
			finally
			{
				m_ptr = PInvoke.bh_ir_ptr.Null;
			}
		}
		
		/// <summary>
		/// Releases all resource used by the <see cref="NumCIL.Bohrium.IR"/> object.
		/// </summary>
		public void Dispose()
		{
			Dispose(true);
		}
		
		/// <summary>
		/// Releases unmanaged resources and performs other cleanup operations before the <see cref="NumCIL.Bohrium.IR"/> is
		/// reclaimed by garbage collection.
		/// </summary>
		~IR()
		{
			Dispose(false);
		}
	}
}


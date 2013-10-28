using System;
using NumCIL;
using System.Runtime.InteropServices;
using NumCIL.Generic;

namespace NumCIL.Bohrium
{
    public class BaseArrayKeeper : IDisposable
    {
        private PInvoke.bh_base_ptr m_ptr;
        private GCHandle m_handle;
            
        private BaseArrayKeeper(PInvoke.bh_base_ptr ptr)
        {
            if (ptr == PInvoke.bh_base_ptr.Null)
                throw new Exception("Cannot assign to null pointer");
            m_ptr = ptr;
        }
        
        public PInvoke.bh_base_ptr BasePointer { get { return m_ptr; } }
        
        public void Dispose()
        {
            Dispose(true);
        }
        
        ~BaseArrayKeeper()
        {
            Dispose(false);
        }
        
        private void Dispose(bool disposing)
        {
            if (m_ptr == PInvoke.bh_base_ptr.Null)
                return;
        
            if (disposing)
                GC.SuppressFinalize(this);
                
            VEM.Instance.Free(m_ptr, m_handle, false);
            
            m_ptr = PInvoke.bh_base_ptr.Null;
            m_handle = new GCHandle();
        }
        
        public bool HasHandle { get { return m_handle.IsAllocated; } }
        
        public void Pin(Array data)
        {
            if (this.HasHandle || m_ptr.Data != IntPtr.Zero)
                throw new Exception("Cannot re-assign");
                
            m_handle = GCHandle.Alloc(data, GCHandleType.Pinned);
            m_ptr.Data = m_handle.AddrOfPinnedObject();
        }
        
        public void SetDataPtr(IntPtr ptr)
        {
            if (m_ptr.Data != IntPtr.Zero)
                throw new Exception("Cannot re-assign");
            m_ptr.Data = ptr;
        }
        
        public static BaseArrayKeeper CreateBaseArrayKeeper<T>(IDataAccessor<T> accessor)
        {
            if (!(accessor.Tag is BaseArrayKeeper))
            {
                var bac = new BaseArrayKeeper (VEM.Instance.CreateBaseArray<T>(accessor.Length));
                accessor.Tag = bac;

                if (accessor.IsAllocated)
                {
                    if (accessor is BohriumAccessor<T>)
                        bac.Pin((accessor as BohriumAccessor<T>).InternalData);
                    else if (accessor is IUnmanagedDataAccessor<T>)
                        bac.SetDataPtr((accessor as IUnmanagedDataAccessor<T>).Pointer);
                    else
                        bac.Pin(accessor.AsArray());
                }
            }
                
            return accessor.Tag as BaseArrayKeeper;
        }
    }
}


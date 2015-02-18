using System;

namespace NumCIL.Bohrium
{
	public static class Disposer
	{
		public static T Create<T>(T item) 
			where T : IDisposable
		{
			return new DisposerImpl<T>(item).Child;
		}
	}

	public class DisposerImpl<T>
		where T : IDisposable
	{

		public T Child { get; private set; }
		private bool m_hasDisposed = false;

		public DisposerImpl(T child)
		{
			Child = child;
		}

		~DisposerImpl()
		{
			if (!m_hasDisposed)
			{
				m_hasDisposed = true;
				Child.Dispose();
			}
		}
	}
}


using System;
using System.Runtime.InteropServices;
using System.Security;

namespace CitizenFX.Core
{
	internal class PushRuntime : IDisposable
	{
		private readonly IntPtr m_runtime;
		private readonly IScriptRuntime m_actualRuntime;
		private static IScriptRuntimeHandler ms_runtimeHandler;
		private static IntPtr ms_runtimeHandlerIface;

		private static FastMethod<Action<IntPtr, IntPtr>> ms_pushMethod;
		private static FastMethod<Action<IntPtr, IntPtr>> ms_popMethod;

		[SecuritySafeCritical]
		public PushRuntime(IScriptRuntime runtime)
		{
			EnsureRuntimeHandler();

#if !IS_FXSERVER
			var comRuntime = Marshal.GetComInterfaceForObject(runtime, typeof(IScriptRuntime));
			ms_pushMethod.method(ms_runtimeHandlerIface, comRuntime);

			m_runtime = comRuntime;
#else
			ms_runtimeHandler.PushRuntime(runtime);

			m_actualRuntime = runtime;
#endif
		}

		public void Dispose()
		{
#if IS_FXSERVER
			ms_runtimeHandler.PopRuntime(m_actualRuntime);
#else
			ms_popMethod.method(ms_runtimeHandlerIface, m_runtime);
#endif
		}

		[SecuritySafeCritical]
		private static void EnsureRuntimeHandler()
		{
			if (ms_runtimeHandler == null)
			{
				ms_runtimeHandler = InternalManager.CreateInstance<IScriptRuntimeHandler>(new Guid(0xc41e7194, 0x7556, 0x4c02, 0xba, 0x45, 0xa9, 0xc8, 0x4d, 0x18, 0xad, 0x43));

#if !IS_FXSERVER
				ms_pushMethod = new FastMethod<Action<IntPtr, IntPtr>>("PushRuntime", ms_runtimeHandler, typeof(IScriptRuntimeHandler), 0);
				ms_popMethod = new FastMethod<Action<IntPtr, IntPtr>>("PopRuntime", ms_runtimeHandler, typeof(IScriptRuntimeHandler), 2);

				ms_runtimeHandlerIface = Marshal.GetComInterfaceForObject(ms_runtimeHandler, typeof(IScriptRuntimeHandler));
#endif
			}
		}
	}
}

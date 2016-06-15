#pragma once
#include <Windows.h>
#include <xutility>


///////////////////////////////////////////////////////////////////////////////////////////////
//
//                  ���Ƶ�һ���������̷߳�װ��
//
//////////////////////////////////////////////////////////////////////////////////////////////



class Thread
{
public:
	HANDLE hThread;

	template<class _Ptr>
	//dwCreationFlags��ο�CreateThread
	Thread(_Ptr&& _ptr, DWORD dwCreationFlags = 0)
	{
		hThread = (HANDLE)_beginthreadex(NULL, 0, [](void * UserData) ->unsigned
		{
			auto Ptr = (_Ptr*)UserData;

			auto hr=CoInitialize(NULL);

			(*Ptr)();

			if (SUCCEEDED(hr))
				CoUninitialize();

			delete Ptr;

			return 0;

		}, (void*)new _Ptr(std::move(_ptr)), dwCreationFlags, NULL);
	}

	template<class _Ptr>
	Thread(COINIT ,_Ptr&& _ptr,  DWORD dwCreationFlags = 0)
	{
		hThread = (HANDLE)_beginthreadex(NULL, 0, [](void * UserData) ->unsigned
		{
			auto Ptr = (_Ptr*)UserData;

			auto hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

			
			(*Ptr)();

			if (SUCCEEDED(hr))
				CoUninitialize();

			delete Ptr;

			return 0;

		}, (void*)new _Ptr(std::move(_ptr)), dwCreationFlags, NULL);
	}

	//���Ƹ��ƹ���Ⱥ���
	Thread(Thread const&);
	Thread& operator=(Thread const&);

	Thread()
		:hThread(NULL)
	{

	}
	Thread(Thread &&Item)
		:hThread(Item.Detach())
	{
	}

	~Thread()
	{
		if (hThread)
		{
			CloseHandle(hThread);
		}
	}

	HANDLE Detach()
	{
		HANDLE TempThread = hThread;
		hThread = NULL;
		return TempThread;
	}

	//�ָ��߳�����
	DWORD ResumeThread()
	{
		return ::ResumeThread(hThread);
	}

	//�����߳�
	DWORD SuspendThread()
	{
		return ::SuspendThread(hThread);
	}

	//�ȴ��̣߳�dwMilliseconds��ͬ��WaitForSingleObject
	DWORD WaitThread(DWORD dwMilliseconds = INFINITE)
	{
		return WaitForSingleObject(hThread, dwMilliseconds);
	}
};
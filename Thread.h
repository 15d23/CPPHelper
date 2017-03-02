#pragma once
#include <Windows.h>
#include <xutility>
#include "CriticalSectionHelper.h"
#include <atlutil.h>

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
	Thread(_Ptr& _ptr, DWORD dwCreationFlags = 0)
	{
		hThread = (HANDLE)_beginthreadex(NULL, 0, [](void * UserData) ->unsigned
		{
			auto Ptr = (_Ptr*)UserData;


			(*Ptr)();


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


#define Task Thread

class CATLThreadPoolWorker
{
public:
	_beginthreadex_proc_type Call;
	void* UserData;

	//HANDLE hWorkEvent;

	volatile LONG Count;
	BOOL bCancelled;
	CATLThreadPoolWorker(_beginthreadex_proc_type _Call, void* _UserData)
		: Call(_Call)
		, UserData(_UserData)
		, Count(0)
		, bCancelled(FALSE)
		//, hWorkEvent(CreateEvent(NULL,TRUE,TRUE,NULL))
	{
	}

	template<class _Ptr>
	//dwCreationFlags��ο�CreateThread
	CATLThreadPoolWorker(_Ptr&& _ptr)
		: UserData(&_ptr)
		, Count(0)
		, bCancelled(FALSE)
	{
		Call=[](void * UserData) ->unsigned
		{
			auto Ptr = (_Ptr*)UserData;


			(*Ptr)();

			return 0;

		};
	}

	//��ʼִ������
	void John()
	{
		if(!bCancelled)
			Call(UserData);

		InterlockedDecrement(&Count);
	}
};

class CATLThreadPoolWorkerInternal
{
public:
	typedef CATLThreadPoolWorker* RequestType;

	static BOOL Initialize(void *pvParam)
	{
		return TRUE;
	}

	static void Terminate(void* pvParam)
	{
	}

	static void Execute(
		_In_ typename RequestType request,
		_In_ void *pvWorkerParam,
		_In_ OVERLAPPED *pOverlapped)
	{
		ATLASSERT(request != NULL);

		request->John();
	}

	static BOOL GetWorkerData(DWORD /*dwParam*/, void ** /*ppvData*/)
	{
		return FALSE;
	}
};


class CThreadPool2: public CThreadPool<CATLThreadPoolWorkerInternal>
{
public:

	BOOL QueueRequest(_In_ typename CATLThreadPoolWorkerInternal::RequestType request)
	{
		InterlockedIncrement(&request->Count);
		
		if (CThreadPool<CATLThreadPoolWorkerInternal>::QueueRequest(request))
		{
			return TRUE;
		}
		else
		{
			//�ύ����ʧ��

			InterlockedDecrement(&request->Count);
			return FALSE;
		}
	}
};
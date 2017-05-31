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

	HANDLE hWorkEvent;

	volatile LONG Count;
	BOOL bCancelled;
	CATLThreadPoolWorker(_beginthreadex_proc_type _Call, void* _UserData)
		: Call(_Call)
		, UserData(_UserData)
		, Count(0)
		, bCancelled(FALSE)
		, hWorkEvent(CreateEvent(NULL,TRUE,TRUE,NULL))
	{
	}

	template<class _Ptr>
	//dwCreationFlags��ο�CreateThread
	CATLThreadPoolWorker(_Ptr&& _ptr)
		:CATLThreadPoolWorker(
			[](void * UserData) ->unsigned
			{
				auto Ptr = (_Ptr*)UserData;


				(*Ptr)();

				return 0;

			},
			&_ptr)
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

		Release();
	}

	//�����������
	void AddRef()
	{
		if (InterlockedIncrement(&Count)==1)
		{
			ResetEvent(hWorkEvent);
		}
	}

	//������������
	void Release()
	{
		if (InterlockedDecrement(&Count) == 0)
		{
			SetEvent(hWorkEvent);
		}
	}

	DWORD __fastcall Wait(_In_ DWORD dwMilliseconds= INFINITE)
	{
		return WaitForSingleObject(hWorkEvent,dwMilliseconds);
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
		request->AddRef();
		
		if (CThreadPool<CATLThreadPoolWorkerInternal>::QueueRequest(request))
		{
			return TRUE;
		}
		else
		{
			//�ύ����ʧ��

			request->Release();
			return FALSE;
		}
	}

	//����For��ʹ��ָ��
	template<class Item, class _Ptr>
	void For(Item* pItems, LONG Count, _Ptr& _ptr)
	{
		volatile LONG Index = -1;

		CATLThreadPoolWorker Work([&Index, pItems,&_ptr]()
		{
			_ptr(pItems[InterlockedIncrement(&Index)]);
		});

		for (long i = 0; i != Count; ++i)
		{
			Work.John();
		}

		Work.Wait();
	}

	//����For��ʹ�õ�����
	template<class iterator, class _Ptr>
	void For(iterator Begin, iterator End, _Ptr& _ptr)
	{
		CCriticalSection Lock;
		iterator pItems = Begin;
		CATLThreadPoolWorker Work([&pItems,&Lock, &_ptr]()
		{
			Lock.Lock();

			auto Item = *(pItems++);
			Lock.Unlock();

			_ptr(Item);
		});

		for (; Begin!= End; ++Begin)
		{
			Work.John();
		}

		Work.Wait();
	}
};
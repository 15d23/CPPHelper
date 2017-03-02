#pragma once
#include <Windows.h>


//�Զ����г�ʼ�����Զ��ͷ��ٽ���
class CCriticalSection : private CRITICAL_SECTION
{
public:
	BOOL IsInitialize;
	CCriticalSection()
		:IsInitialize(InitializeCriticalSectionEx(this, 0, CRITICAL_SECTION_NO_DEBUG_INFO))
	{
	}

	//���ø��ƹ���
	CCriticalSection(const CCriticalSection&) = delete;

	//���ø�ֵ
	CCriticalSection& operator=(const CCriticalSection&) = delete;


	~CCriticalSection()
	{
		if(IsInitialize)
			DeleteCriticalSection(this);
	}


	void Lock()
	{
		if(IsInitialize)
			EnterCriticalSection(this);
	}

	void Unlock()
	{
		if (IsInitialize)
			LeaveCriticalSection(this);
	}
};

template<class T>
class CLockHelper
{
	
public:
	T* pLock;

	CLockHelper(T* _pLock)
		:pLock(_pLock)
	{
		pLock->Lock();
	}

	~CLockHelper()
	{
		pLock->Unlock();
	}
};

typedef CLockHelper<CCriticalSection> CCriticalLockHelper;


//��д��
class CSRWLock : SRWLOCK
{
public:
	CSRWLock()
	{
		InitializeSRWLock(this);
	}

	~CSRWLock()
	{
	}


	void ReadLock()
	{
		AcquireSRWLockShared(this);
	}

	void ReadUnlock()
	{
		ReleaseSRWLockShared(this);
	}

	void WriteLock()
	{
		AcquireSRWLockExclusive(this);
	}

	void WriteUnLock()
	{
		ReleaseSRWLockExclusive(this);
	}
};
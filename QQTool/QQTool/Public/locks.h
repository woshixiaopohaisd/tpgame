/**
 *	 File  :  locks.h
 *   Brief :  一个支持事件和信号量锁，利用引用计数的方式
 *   Author:  Expter 
 *	 Creat Date:  [2009/11/11]


 *   使用方式类似Auto_Ptr,声明一个csectionlock对象即可。
 */

#pragma once
#include <windows.h>

namespace sync
{
	/// 
	/// 一个锁的接口函数
	///
	class Super_lock
	{
	public:
		Super_lock(void);
		virtual ~Super_lock(void);

	public:
		virtual bool open() = 0;
		virtual void close()= 0;
		virtual bool enter()= 0;
		virtual void leave()= 0;

	};


	class scope_guard
	{
	public:
		scope_guard( Super_lock & _lock);
		~scope_guard();

	private:
		Super_lock & lock;

	};


	/// 事件锁
	class eventlock 
		: public Super_lock
	{
		struct state_type
		{
			enum 
			{
				none,
				timeout,
			};
		};

		eventlock();
		~eventlock();

		bool open();
		void close();
		bool enter();
		void leave();

	private:
		HANDLE handle_;
		DWORD  timeout_;
		int    state_;
	};

	/// 临界区
	class csectionlock
		:public Super_lock
	{
	public:
		csectionlock();
		~csectionlock();

		bool open();
		void close();
		bool enter();
		void leave();

	private:
		bool  is_ok;
		::CRITICAL_SECTION cs_;
	};

}

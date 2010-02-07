/*
*	File  :
*   Brief :
*   Author:  Expter 
*	Creat Date:  [2010/2/4]
*/

#pragma  once

#include "LuaFnRegister.h"

namespace tp_script
{
	
	class  CLuaInterface
	{

 	public:
		/// �ű�����
		CLuaScript		m_LuaParse;

		/// Luaע����
		CLuaFnRegister  m_LuaFnReg;

		/// �ű��б�
		//TableList		m_ScriptTable;

		/// ִ�нű���object
		void			*m_pOwner;

	public:

		CLuaInterface();

		~CLuaInterface();

		void  Init();

		void  Destroy();

		template< typename owner>
		void  SetOwner(owner * powner) { m_pOwner = powner; }

		template< typename owner>
		owner* GetOwner()			   { return m_pOwner ;  }
	    
		bool  FindSymbol( lua_State* L , char* funcname );

		bool  PrefixCall( lua_State* L , char** funcname );

		int   ExeScript( int  sid , char* funcname );

		int   ExeScript( int  sid , char* funcname , int Param0 = 0);


	private:

		int   ExeFile( char * filename , char* funcanme , bool bload );

	};

}
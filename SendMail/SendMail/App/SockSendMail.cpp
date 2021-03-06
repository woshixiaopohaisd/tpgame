#include "stdafx.h"
#include "SockSendMail.h"
#include "../Public/BaseDef.h"
#include "../Public/Singleton.h"
#include "../Public/STLFun.h"
#include "../Public/Base.h"
#include "../Public/Tools.h"

bool  SocketSendToMail::Send( size_t index )
{
	size_t tsize = GetInstObj(MailLoginInfo).m_Vec.size();
	if ( tsize <= m_SendID )
	{
		return false ;
	}				  

	
	tagSend		 m_Send     = GetInstObj(MailLoginInfo).m_Vec[ m_SendID ];

	tagSendInfo& m_SendInfo = GetInstObj(tagSendInfo);

	tagSendRole& SendRole	= GetInstObj(tagSendRole);

	tagMailSub&  sMailSub    = GetInstObj(tagMailSub);
	if ( sMailSub.VecMailSub.size() <= 0 )
		return false;

	MailSubNode& MailSub     = sMailSub.VecMailSub[ Random( sMailSub.VecMailSub.size() ) ] ;

	char	     Buf[256] = { 0 };

	static long sendnum = 0;
	sendnum ++;

	if( !CreateSocket() )
		return false;

	if( !m_Socket.Connect(m_Send.smtp.c_str(), 25 ) )
	{
		m_Socket.Close();
		return FALSE;
	}

	if( !CheckResponse( "220" , Buf ) )
	{
		m_Socket.Close();
		return false;
	}

	if (!CheckAccount(m_Send.smtp,m_Send.user,m_Send.pword) )
	{
		ReleaseSocket();
		return false;
	}

	//// 验证通过发送数据

	if(m_list.size()<=0) return false;

	string strTmp;

	//发送MAIL FROM:<abc@xyz.com>
	strTmp="MAIL FROM:<"+m_Send.login+">\r\n";
	if(m_Socket.Send(strTmp.c_str(),strTmp.length()) == SOCKET_ERROR)
	{
		int elen = WSAGetLastError();
		ReleaseSocket();
		return false;
	}
	if(!CheckResponse("250", Buf )) return false;

	//发送RCPT To:<abc@xyz.com>
	for ( std::list<std::string>::iterator itr = m_list.begin() ; itr != m_list.end() ; itr ++ )
	{
		strTmp="RCPT To:<"+(*itr)+">\r\n";

		if(m_Socket.Send(strTmp.c_str(),strTmp.length()) == SOCKET_ERROR)
		{
			int elen = WSAGetLastError();
			ReleaseSocket();
			return false;
		}
		if(!CheckResponse("250", Buf )) 
			continue;
	}

	//发送"DATA\r\n"
	if(m_Socket.Send("DATA\r\n",strlen("DATA\r\n")) == SOCKET_ERROR)
	{
		int elen = WSAGetLastError();
		ReleaseSocket();
		return false;
	}
	if(!CheckResponse("354", Buf )) return false;


	strTmp="MIME-Version: 1.0\r\n";

	//"Mail From:SenderName<xxx@mail.com>\r\n"
	strTmp+="From: "+m_Send.name+"<"+m_Send.login+">\r\n";

	strTmp+="To: ";//friends<"+m_Send.login;
	//发送RCPT To:<abc@xyz.com>
	for ( std::list<std::string>::iterator itr = m_list.begin() ; itr != m_list.end() ; itr ++ )
	{
		strTmp+="'firends'<"+(*itr)+">,";
	}
 	strTmp+="\r\n";

	//"Subject: 邮件主题\r\n"	  subject,SendRole.sReplace.size(),SendRole
	string  subject = "";
	subject = ttos( MailSub.sMailSub,MailSub.lReNum,SendRole.sReplace.size(),
		SendRole.NoChangeRand,SendRole.Rand1,SendRole.Rand2,SendRole.Rand3);

	strTmp+="Subject: "+subject+"\r\n";

	if ( m_Send.login.find("@yahoo") != string::npos )
		SendYahoo( strTmp , m_SendInfo.Context );
	else if( m_Send.login.find("@tom") != string::npos )
		SendTom( strTmp , m_SendInfo.Context );
	else
		SendOther( strTmp , m_SendInfo.Context );

	
	if(!CheckResponse("250", Buf )) return false;

	logSuce << " Send Ok.. Mail Addr " << m_Send.login  <<"\r\n";

	if(m_Socket.Send("QUIT\r\n",strlen("QUIT\r\n") ) == SOCKET_ERROR)
	{
		ReleaseSocket();
		return false;
	}

	if(!CheckResponse("221", Buf )) return false;

	ReleaseSocket();
	return true;
}

bool  SocketSendToMail::CheckAccount(std::string ip,std::string name,std::string pass)
{
	CBase base64;

	char Buf[1024];
	//向服务器发送"HELO "+服务器名
	string strTmp="HELO SmtpServer\r\n";
	if(m_Socket.Send( strTmp.c_str(),strTmp.length() )  == SOCKET_ERROR)	
	{
		ReleaseSocket();
		return false;
	}
	if(!CheckResponse("250", Buf )) return false;

	if(m_Socket.Send("AUTH LOGIN\r\n",strlen("AUTH LOGIN\r\n")) == SOCKET_ERROR)
	{
		ReleaseSocket();
		return false;
	}
	if(!CheckResponse("334", Buf )) return false;

	//发送经base64编码的用户名
	string strUserName=base64.Encode((unsigned char *)name.c_str(),name.length())+"\r\n";

	if( m_Socket.Send(strUserName.c_str(),strUserName.length() ) ==  SOCKET_ERROR) 
	{
		ReleaseSocket();
		return false;
	}
	if(!CheckResponse("334", Buf )) return false;

	//发送经base64编码的密码
	string strPassword=base64.Encode((unsigned char *)pass.c_str(),pass.length())+"\r\n";

	if( m_Socket.Send(strPassword.c_str(),strPassword.length() ) ==  SOCKET_ERROR) 
	{
		ReleaseSocket();
		return false;
	}
	if(!CheckResponse("235", Buf )) return false;
	return true;
}


bool  SocketSendToMail::SendYahoo(std::string &strTmp,std::string & context)
{
	strTmp+="Content-Type: multipart/mixed;boundary='=====f0g_ftlTZ385rJVM22x_wrf9tPFVaIG5j3_TP====='\r\n\r\n";

	//将邮件内容发送出去
	if(m_Socket.Send(strTmp.c_str(),strTmp.length() ) == SOCKET_ERROR)
	{
		ReleaseSocket();

		return false;	
	}

	strTmp="--=====f0g_ftlTZ385rJVM22x_wrf9tPFVaIG5j3_TP=====\r\n" ;
	strTmp+="Content-Type: text/html; charset=gb2312\r\n" ;

	strTmp+="Content-Transfer-Encoding: 8bit\r\n";

	//邮件内容
	strTmp+=context+"\r\n";
	strTmp+="--=====f0g_ftlTZ385rJVM22x_wrf9tPFVaIG5j3_TP=====--\r\n";

	strTmp+="\r\n.\r\n";

	//将邮件内容发送出去
	if(m_Socket.Send(strTmp.c_str(),strTmp.length() ) == SOCKET_ERROR)
	{
		ReleaseSocket();

		return false;	
	}
	return true;
}

bool  SocketSendToMail::SendOther(std::string& strTmp,std::string & context)
{	  
	
	//邮件主体
	strTmp+="--";
	strTmp+="boundary";
	strTmp+="\r\n";
	strTmp+="Content-type:text/html; Charset=gb2312\r\n";
	strTmp+="Content-Transfer-Encoding: 8bit\r\n\r\n";
	strTmp+=context+"\r\n";

	//将邮件内容发送出去
	if(m_Socket.Send(strTmp.c_str(),strTmp.length() ) == SOCKET_ERROR)
	{
		ReleaseSocket();

		return false;	
	}

	//界尾
	strTmp="--";
	strTmp+="boundary";
	strTmp+="--\r\n.\r\n";


	//将邮件内容发送出去
	if(m_Socket.Send(strTmp.c_str(),strTmp.length() ) == SOCKET_ERROR)
	{
		ReleaseSocket();

		return false;	
	}

	return true;
}

bool SocketSendToMail::SendTom(std::string& strTmp,std::string & context)
{
	strTmp+="X-Mailer: Free Mail to Send by oohaha\r\nDate: ";

	strTmp+=nowtime()+" +0800\r\n";

	strTmp+="Content-Transfer-Encoding: Quoted-Printable\r\n";

	strTmp+="Content-Type: text/html; charset=\"gb2312\"\r\n\r\n" ;

	char str[1024];
	EncodeQuoted((unsigned char*)context.c_str() , str, context.length() , 1024 );

	//邮件内容
	strTmp+=context;
	strTmp+="\r\n\r\n";

	strTmp+="\r\n.\r\n";

	//strTmp="Subject: =?GB2312?Q?=B7=A2=CB=CD=C3=FB?=\r\nSender: \"=?GB2312?Q?juanfangw389?=\" <juanfangw389@tom.com>\r\nFrom: \"=?GB2312?Q?juanfangw389?=\" <juanfangw389@tom.com>\r\nDate: Fri, 19 Nov 2010 00:33:12 +0800\r\nTo: \"=?GB2312?Q?haha?=\" <283899487@qq.com>,\r\n\"=?GB2312?Q?haha?=\" <tangpeng918@126.com>\r\nX-Priority: 3\r\nX-MSMail-Priority: Normal\r\nContent-Transfer-Encoding: Quoted-Printable\r\nMIME-Version: 1.0\r\nX-Mailer: JMail 4.3.0 Free Version by Dimac\r\nContent-Type: text/plain;\r\ncharset=\"GB2312\"\r\n\r\n=D3=CA=BC=FE=B7=A2=CB=CD=C4=DA=C8=DD=B2=E2=CA=D4=D2=BB=B9=FE=D5=FD=CE=C4!\r\n\r\n\r\n.\r\n";
	//将邮件内容发送出去
	if(m_Socket.Send(strTmp.c_str(),strTmp.length() ) == SOCKET_ERROR)
	{
		ReleaseSocket();

		return false;	
	}
	return true;
}	
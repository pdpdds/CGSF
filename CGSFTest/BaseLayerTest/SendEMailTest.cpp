#include "stdafx.h"
#include "SendEMailTest.h"
#include "CSMTP.h"

//#pragma comment(lib, "libeay32.lib")
//#pragma comment(lib, "ssleay32.lib")

SendEMailTest::SendEMailTest(void)
{
}


SendEMailTest::~SendEMailTest(void)
{
}

bool SendEMailTest::Run()
{
	bool bError = false;

	try
	{
		CSmtp mail;

		//#define test_gmail_tls

#if defined(test_gmail_tls)
		//mail.SetSMTPServer("smtp.gmail.com",587);
		//mail.SetSecurityType(CSmtp::USE_TLS);
#elif defined(test_gmail_ssl)
		mail.SetSMTPServer("smtp.gmail.com",465);
		mail.SetSecurityType(CSmtp::USE_SSL);
#elif defined(test_hotmail_TLS)
		mail.SetSMTPServer("smtp.live.com",25);
		mail.SetSecurityType(CSmtp::USE_TLS);
#elif defined(test_aol_tls)
		mail.SetSMTPServer("smtp.aol.com",587);
		mail.SetSecurityType(CSmtp::USE_TLS);
#elif defined(test_yahoo_ssl)
		mail.SetSMTPServer("plus.smtp.mail.yahoo.com",465);
		mail.SetSecurityType(CSmtp::USE_SSL);
#endif

		mail.SetSMTPServer("smtp.daum.net",465);
		mail.SetSecurityType(USE_SSL);

		mail.SetLogin("***"); //daum id
		mail.SetPassword("***"); //daum password

		mail.SetSenderName("***"); //자기이름
		mail.SetSenderMail("***");//SenderMail
		mail.SetReplyTo("***");//SenderMail
		mail.SetSubject("The message");
		mail.AddRecipient("***"); //ReceiverMail
		mail.SetXPriority(XPRIORITY_HIGH);
		mail.SetXMailer("The Bat! (v3.02) Professional");
		mail.AddMsgLine("Hello,");
		mail.AddMsgLine("");
		mail.AddMsgLine("...");
		mail.AddMsgLine("How are you today?");
		mail.AddMsgLine("");
		mail.AddMsgLine("Regards");
		mail.ModMsgLine(5,"regards");
		mail.DelMsgLine(2);
		mail.AddMsgLine("User");

		//mail.AddAttachment("../test1.jpg");
		//mail.AddAttachment("c:\\test2.exe");
		//mail.AddAttachment("c:\\test3.txt");
		mail.Send();
	}
	catch(ECSmtp e)
	{
		std::cout << "Error: " << e.GetErrorText().c_str() << ".\n";
		bError = true;
	}
	if(!bError)
		std::cout << "Mail was send successfully.\n";

	return true;
}

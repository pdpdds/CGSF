#ifndef __TEST_CLIENT_H__
#define __TEST_CLIENT_H__

#include "serviceDef.h"
#include "proxy.hpp"

#include "service_err.hpp"
#include <iostream>

// For mem leak detection
/*
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
*/
class TestArgImpl : public TestArg
{
public:
	virtual void Serialize(CPstream& ar)
	{
		TestArg::Serialize(ar);

		if (ar.isLoading())
			ar >> _argStr;
		else
			ar << _argStr;
	}
public:
	TestArgImpl() : _argStr("000000000"){}
	virtual void doSomething()
	{
		std::cout << "* Changing the member _argStr to 123456789." << std::endl;
		_argStr = "123456789";
	}
	virtual void show() { std::cout << "* TestArgImpl member: _argStr = " << _argStr.data() << std::endl; }
private:
	std::string _argStr;

	declare_serial(TestArgImpl)
};

class TestInterfaceImpl : public TestInterface
{
public:
	TestInterfaceImpl() :_str(""){}
	virtual std::string& echo(std::string& str)
	{
		std::cout << "[Server], std::string& echo(std::string& str): " << endl;
		cout << "...param1:" << str << std::endl;
		_str = str;
		str = "莫";
		cout << "param1= 莫" << endl;

		cout << "...returned:" << string(_str + "1234").c_str() << endl;
		return *new string(_str + "1234");
	}

	virtual void echo1(TestArg& testArg)
	{
		std::cout << "[Server],void echo1(TestArg& testArg):" << endl;
		cout << "...invoke virtual void show()....output:" << endl;
		testArg.show();
		std::cout << "...invoke doSomething()....output:" << endl;
		testArg.doSomething();
	}

	virtual string echo2(string str)
	{
		cout << "echo2(): received input string:" << str.c_str() << endl;
		cout << "echo2(): Return string value type: 你好7777" << endl;
		return "你好7777";
	}

	virtual TestArg1 echo3()
	{
		TestArg1 arg1;

		arg1.ll = 2222;
		return arg1;
	}

	double echo4(unsigned long & ll, float& ff)
	{
		cout << "echo4(): received param1 = " << ll << ", param2 = " << ff << endl;
		cout << "echo4(): changed param1 to 3333, param2 to 0.9999, returned value: 0.7777" << endl;
		ll = 3333;
		ff = 0.9999;
		return 0.7777;
	}

	virtual char* echo5(char * cp, int * ip, char  cc)
	{
		cout << "[Server]: short echo5 (char * cp, int * ip, char & cc):" << endl;
		cout << "...param1:" << cp << ",param2:" << *ip << ",param3:" << cc;
		cout << "...Changed param1[0] to \'A\'." << endl;
		string tmp(cp, strlen(cp));
		tmp.at(0) = 'A';
		strncpy(cp, tmp.data(), tmp.size());

		cout << "...Changed param1[1] to 123'." << endl;
		*ip = 123;
		cout << "...Changed param1[2] to \'X\'." << endl;
		cc = 'X';

		char * retStr = new char[100];
		strcpy(retStr, "你好，世界！");
		return retStr;
	}

	virtual double * echo6(float* ff, TestArg1 arg, unsigned short* ss, long* ll)
	{
		cout << "[Server]:ouble * echo6(float* ff, TestArg1 arg, unsigned short* ss, long* ll):" << endl;
		cout << "...param0:" << *ff << ",param1 (TestArg::ll):" << arg.ll << ",param2:" << *ss << ",param3:" << *ll;

		*ff = 99.99;
		arg.ll = 999;
		*ss = 99;
		*ll = 9999;
		cout << "...Changed value:  *ff = 99.99, arg.ll = 999, *ss = 99, *ll = 9999" << endl;
		cout << "...Returned: 88.88" << endl;
		return new double(88.88);
	}
	virtual double * echo7(float* ff, TestArg1 arg, unsigned short* ss, long* ll, string& str1)
	{
		cout << "[Server]: double * echo7(float* ff, TestArg1 arg, unsigned short* ss, long* ll, string& str1):" << endl;
		cout << "...param4:" << str1.c_str() << endl;
		return echo6(ff, arg, ss, ll);
	}
	virtual TestArg * echo8(float* ff, TestArg1 arg, unsigned short* ss, long* ll, string& str1, bool* bb)
	{
		cout << "[Server]: double * echo8(float* ff, TestArg1 arg, unsigned short* ss, long* ll, string& str1, bool* bb):" << endl;
		cout << "...param5:" << *bb << endl;
		cout << "...Change param5 to false." << endl;
		*bb = false;
		return NULL;
	}
	virtual TestArg& echo9(float* ff, TestArg1 arg, unsigned short* ss, long* ll, string& str1, string& str2, bool bb)
	{
		cout << "[Server]:double * echo9(float* ff, TestArg1 arg, unsigned short* ss, long* ll, string& str1, string& str2, bool bb):" << endl;
		cout << "[Server]: param0=" << (ff ? *ff : 0.0) << ",param1: TestArg1(" << arg.ll << "),param2:" << (ss ? *ss : 0)
			<< ",param3:" << (ll ? *ll : 0) << ",param4:"
			<< str1 << ",param5:" << str2 << ",param6:" << bb << endl;
		return *(new TestArgImpl);
	}
	virtual double* echo10(float* ff, TestArg1 arg, unsigned short* ss, long* ll, string* str, string& str2, int & ii, TestArg* bb)
	{
		cout << "[Server]:" << endl;
		cout << "...float ff = " << *ff << endl;
		cout << "...unsigned short *ss = " << *ss << endl;
		cout << "...TestArg1 arg=" << arg.ll << endl;
		cout << "...long ll = " << *ll << endl;
		cout << "...string str = " << *str << endl;
		cout << "...string str2 = " << str2 << endl;
		cout << "...int ii = " << ii << endl;
		cout << "...TestArg* arg = " << endl;
		bb->show();
		bb->doSomething();

		cout << "...Changed str to \"uuuu\"" << endl;
		*str = "uuuu";

		return new double(66.66);
	}


	virtual ~TestInterfaceImpl(){}
private:
	std::string _str;


};



//BOOST_CLASS_EXPORT_GUID(TestArgImpl, "TestArgImpl")
#endif
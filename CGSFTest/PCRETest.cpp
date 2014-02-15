#include "stdafx.h"
#include "PCRETest.h"

#include <stdio.h>
#include <string.h>     
#include <cassert>
#include <vector>
#include <fstream>
#include <iostream> 
#include <sstream>  

#include "pcrecpp.h"
using namespace std;

#ifdef _DEBUG
#pragma comment(lib, "pcred.lib")
#pragma comment(lib, "pcrecppd.lib")
#else
#pragma comment(lib, "pcre.lib")
#pragma comment(lib, "pcrecpp.lib")
#endif
	
PCRETest::PCRETest(void)
{
}


PCRETest::~PCRETest(void)
{
}

bool PCRETest::Run()
{
	int i;
	string s;
	pcrecpp::RE re("(\\w+):(\\d+)");
	if (re.error().length() > 0) {
		cout << "PCRE compilation failed with error:" << re.error() << "\n";
	}
	if (re.PartialMatch("root:1234", &s, &i))
		cout << s << " : " << i << "\n";

	return true;
}

// BigNumTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "BigNum.h"

#include <conio.h>

using namespace std;

//각 자리당 최대 8자리를 잡고 있으므로 Normalize
//32자리의 계산이 가능

int _tmain(int argc, _TCHAR* argv[])
{
	// Example of using BigNum
	//ULONGLONG no1[] = {0,0,0,123496789123};//{1111,1111,1111,12345678912};
	//ULONGLONG no2[] = {00000,0,22222222,99999922222222};
	ULONGLONG no1[] = {0,0,1212,1234};//{1111,1111,1111,12345678912};
	ULONGLONG no2[] = {00000,0,2222,5678};
	
	BigNum b1( no1);
	BigNum b2 (no2);
	BigNum b3 = b2 - b1;
	BigNum b4 = b1 * b2;
	BigNum b5 = b1 * 4;

	b1.Normalize();
	b2.Normalize();
	cout << "\nno1	    = ";
	b1.display();
	cout << "\nno2         = ";
	b2.display();
	cout << "\nno2 - no1   = ";
	b3.display();

	cout << "\nno1 * no2   = ";
	b4.display();
	
	cout << "\nno1 *   4   = ";
	b5.display();
	
	

	_getwch();
	return 0;
};

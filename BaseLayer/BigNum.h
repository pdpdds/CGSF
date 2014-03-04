#pragma once

#include <windows.h>
#include <iostream>

// Higher digit arithmetic (more than 20 digit)
// using ULONGLONG datatype
/*
	(a1x + b1) (a2x + b2) = a1a2x2 + (a1b2+b2a1)x+b1b2	
*/



/*

	ULONGLONG range 0 to 18446744073709551616 

*/

#define BASE 100000000
#define MAX  4

using namespace std;


class BigNum
{
private :
	ULONGLONG no[MAX];  // abcd -> no[0]=a,no[1]=b,no[2]=c,no[3]=d

public :
	
	BigNum()
	{
		for(int i=0 ; i<MAX; i++)
			no[i] = 0;
		
	}
	BigNum(ULONGLONG no_[],int ind=MAX)
	{
		for (int i = 0; i<ind && i < MAX;i++)
			no[i] = no_[i];
	}


	
	void display()
	{
		int dig[MAX];
		int digit = Calc_Digit((long)BASE);
		dig[0] = Calc_Digit(no[0]);
		dig[1] = Calc_Digit(no[1]);
		dig[2] = Calc_Digit(no[2]);
		dig[3] = Calc_Digit(no[3]);
		//cout<< dig <<endl;
		
		if(no[0] != 0)
		{
			//for(int i=1; i<(digit-dig[0]); i++) cout<< "0";
			cout<<no[0]<<" " ;
		}	
	
		if(no[1] != 0 )
		{
			if ( !( no[0] == 0))
				for(int i=1; i<(digit-dig[1]); i++) cout<< "0";
			cout<<no[1]<<" " ;
		}
		
		if(no[2] != 0)
		{
			if ( !( no[0] == 0 && no[1] == 0))
				for(int i=1; i<(digit-dig[2]); i++) cout<< "0";
			cout<<no[2]<<" " ;
		}
		
		if(no[3] != 0)
		{
			if ( !(no[0] == 0 && no[1] == 0 && no[2] == 0))
				for(int i=1; i<(digit-dig[3]); i++) cout<< "0";
			cout<<no[3]<<" " ;
		}
				
		cout<< endl;

	}

	friend BigNum operator + (BigNum b1, BigNum b2 )
	{
		BigNum b3;
		b1.Normalize();
		b2.Normalize();

		for( int i=0;i<4;i++)
		{
			b3.no[i] = b1.no[i] + b2.no[i];
		}
		
		for( int i=0;i<3;i++)
		{
			if ( b3.no[i] < b2.no[i])
			 b3.no[i] = b3.no[i] + 1;
		}
		b3.Normalize();
		return b3;
	}

	friend BigNum operator - (BigNum b1, BigNum b2 )
	{
		BigNum b3;
		b1.Normalize();
		b2.Normalize();

		for( int i=0;i<4;i++)
		{
			b3.no[i] = b1.no[i] - b2.no[i];
		}
		
		for( int i=0;i<3;i++)
		{
			if ( b3.no[i] < b2.no[i])
			 b3.no[i] = b3.no[i] - 1;
		}
		b3.Normalize();
		return b3;
	}

	friend BigNum operator * (BigNum b1, BigNum b2 )
	{
		BigNum b3;
		b1.Normalize();
		b2.Normalize();
		
		b3.no[3] = b2.no[3]*b1.no[3];
		b3.no[2] = b2.no[2]*b1.no[3] + b2.no[3]*b1.no[2];
		b3.no[1] = b2.no[1]*b1.no[3] + b2.no[2]*b1.no[2] + b2.no[1]*b1.no[3];
		b3.no[0] = b2.no[0]*b1.no[3] + b2.no[1]*b1.no[2] + b2.no[2]*b1.no[1] + b2.no[3]*b1.no[0];


		
		b3.Normalize();
		return b3;
	}
	
	friend BigNum operator * (BigNum b1, int mul )
	{
		BigNum b2;
		b1.Normalize();
		
		b2.no[3] = b1.no[3]*mul;
		b2.no[2] = b1.no[2]*mul;
		b2.no[1] = b1.no[1]*mul;
		b2.no[0] = b1.no[0]*mul;


		
		b2.Normalize();
		return b2;
	}

	static int Calc_Digit(long no)
	{
		int i=0;
		while ( no > 0)
		{
			no = no/10;
			i++;
		}
		if(i==0) i=1;
		return i;
	}

	static int Calc_Digit(ULONGLONG no)
	{
		int i=0;
		while ( no > 0)
		{
			no = no/10;
			i++;
		}
		if(i==0) i=1;
		return i;
	}

	static BigNum Normalize(BigNum bigNum)
	{
		BigNum b(bigNum);
		for(int i=3; i>0;i--)
		{
			if (b.no[i] > BASE)
			{
				b.no[i-1] = b.no[i-1] + b.no[i]/BASE;
				b.no[i] = b.no[i]%BASE ;
			}
		}
		b.no[0] = b.no[0]%BASE ;
		return b;
	}

	void Normalize()
	{	
		for(int i=3; i>0;i--)
		{
			if (no[i] > BASE)
			{
				no[i-1] = no[i-1] + no[i]/BASE;
				no[i] = no[i]%BASE ;
			}
		}
		no[0] = no[0]%BASE ;
	}
};
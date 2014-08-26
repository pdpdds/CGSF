/********************************************************************
	File :			DataBuffer.cpp
	Creation date :	2010/6/27
		
	License :			Copyright 2010 Ahmed Charfeddine, http://www.pushframework.com

				   Licensed under the Apache License, Version 2.0 (the "License");
				   you may not use this file except in compliance with the License.
				   You may obtain a copy of the License at
				
					   http://www.apache.org/licenses/LICENSE-2.0
				
				   Unless required by applicable law or agreed to in writing, software
				   distributed under the License is distributed on an "AS IS" BASIS,
				   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
				   See the License for the specific language governing permissions and
				   limitations under the License.
	
	
*********************************************************************/
#include "StdAfx.h"
#include "DataBuffer.h"

#pragma warning(disable : 4244)
#pragma warning(disable : 4267)

DataBuffer::DataBuffer(unsigned int uMaxSize)
{
	uSize = 0;
	Allocate(uMaxSize);
}

DataBuffer::DataBuffer()
{
	uSize = 0;
	pBuffer = NULL;
}

void DataBuffer::Allocate( unsigned int uMaxSize )
{
	this->uMaxSize = uMaxSize;
	pBuffer = new char[uMaxSize];
}

DataBuffer::~DataBuffer(void)
{
	delete[] pBuffer;
}
bool DataBuffer::Append( char* _pBuffer, unsigned int _uSize )
{
	if(getRemainingSize() < _uSize)
		return false;

	//Copy the data :
	CopyMemory(pBuffer+uSize,_pBuffer,_uSize);

	//Update the size of the buffer.
	uSize+=_uSize;

	return true;
}

void DataBuffer::Pop( int _uSize )
{
	MoveMemory(pBuffer,pBuffer+_uSize,(uSize-_uSize));
	uSize-=_uSize;
}

unsigned int DataBuffer::GetDataSize()
{
	return uSize;
}

unsigned int DataBuffer::GetMaxDataSize()
{
	return uMaxSize;
}

char* DataBuffer::GetBuffer()
{
	return pBuffer;
}

unsigned int DataBuffer::getRemainingSize()
{
	return uMaxSize - uSize;
}

void DataBuffer::GrowSize( unsigned int growBy )
{
	uSize += growBy;
}

char DataBuffer::getAt( int offset )
{
	return pBuffer[offset];
}
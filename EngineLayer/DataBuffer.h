/********************************************************************
	File :			DataBuffer.h
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
#ifndef DataBuffer__INCLUDED
#define DataBuffer__INCLUDED

#pragma once

class DataBuffer
{
public:
	DataBuffer(unsigned int uMaxSize);
	DataBuffer();
	~DataBuffer(void);

	void Allocate(unsigned int uMaxSize);
	bool Append(char* _pBuffer, unsigned int _uSize);
	void Pop(int _uSize);
	unsigned int GetDataSize();
	unsigned int GetMaxDataSize();
	char* GetBuffer();
	void GrowSize(unsigned int growBy);
	unsigned int getRemainingSize();
	char getAt(int offset);
private:
	char* pBuffer;
	unsigned int uSize;
	unsigned int uMaxSize;
};

#endif
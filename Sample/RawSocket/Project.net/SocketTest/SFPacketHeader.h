
#ifndef _SFPACKETHEADER_H
#define _SFPACKETHEADER_H

#pragma pack(push, 1)

typedef struct tag_SFPacketHeader
{
	unsigned short packetID;
	unsigned int  packetOption;
	unsigned int  dataCRC;
	unsigned short dataSize;

	tag_SFPacketHeader()
	{
		Clear();
	}


	void Clear()
	{
		packetID = 0;
		packetOption = 0;
		dataCRC = 0;
		dataSize = 0;
	}

}SFPacketHeader;


#pragma pack(pop)

#endif
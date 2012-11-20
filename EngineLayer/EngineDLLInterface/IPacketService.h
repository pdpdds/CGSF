#ifndef IPACKETSERVICE_H_
#define IPACKETSERVICE_H_

class IPacketService
{
  public:  
	IPacketService(){}
    virtual ~IPacketService() {};

	virtual bool OnData(int Serial, char* pData, unsigned short Length) = 0;

protected:

private:
};


#endif  // IPACKETSERVICE_H_
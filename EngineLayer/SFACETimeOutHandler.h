#pragma once

class SFACETimeOutHandler : public ACE_Handler
{
public:
	SFACETimeOutHandler(void);
	virtual ~SFACETimeOutHandler(void);

	 virtual void handle_time_out (const ACE_Time_Value &tv,
                                const void *arg)
    {
		SFPacket* pCommand = PacketPoolSingleton::instance()->Alloc();
		pCommand->SetPacketType(SFCommand_Timer);
		pCommand->SetOwnerSerial(-1);

		LogicGatewaySingleton::instance()->PushPacket(pCommand);

		printf("%d\n", arg);
      // Print out when timeouts occur.
    //  ACE_DEBUG ((LM_DEBUG, "(%t) %d timeout occurred for %s @ %d.\n",
          //        1,
          //        (char *) arg,
            //      (tv - this->start_time_).sec ()));

    }
};


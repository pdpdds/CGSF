#pragma once

#define SF_GETPACKET_ARG(a,b,c) memcpy(a,b.c_str(), sizeof(c));

#define SFSYSTEM SFEngine<GoogleLog, INetworkEngine>

//#define  SFSYSTEM_SERVER(arg1, arg2, arg3) SFEngine<arg1,arg2,arg3>
//#define  SFSYSTEM_CLIENT SFEngine<SFClient,GoogleLog,SFACEFramework>

//#define  SFSYSTEM_SERVER SFEngine<SFServer,GoogleLog,SFMGFramework>
//#define  SFSYSTEM_CLIENT SFEngine<SFClient,GoogleLog,SFMGFramework>

#define DATABASE_TSS ACE_TSS<SFDatabaseMySQL>

//#define CGSF_PACKET_OPTION  PACKET_OPTION_DATACRC|PACKET_OPTION_ENCRYPTION|PACKET_OPTION_COMPRESS
#define CGSF_PACKET_OPTION  0
#pragma once

#ifdef P2PCLIENT_EXPORTS
#define P2PCLIENT_API __declspec(dllexport)
#else
#define P2PCLIENT_API __declspec(dllimport)
#endif

class P2PManager;

typedef P2PManager (*GETP2PMANAGER_FUNC(void));

#ifdef __cplusplus
extern "C" {
#endif

	P2PCLIENT_API P2PManager* GetP2PManager();

#ifdef __cplusplus
};
#endif
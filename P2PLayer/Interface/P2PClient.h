#pragma once

#ifdef P2PCLIENT_EXPORTS
#define P2PCLIENT_API __declspec(dllexport)
#else
#define P2PCLIENT_API __declspec(dllimport)
#endif

class IP2PManager;

typedef IP2PManager(*GETP2PMANAGER_FUNC(void));

#ifdef __cplusplus
extern "C" {
#endif

	P2PCLIENT_API IP2PManager* GetP2PManager();

#ifdef __cplusplus
};
#endif
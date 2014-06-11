#pragma once

#ifdef P2PSERVER_EXPORTS
#define P2PSERVER_API __declspec(dllexport)
#else
#define P2PSERVER_API __declspec(dllimport)
#endif

typedef int (ACTIVATEP2P_FUNC(void));
typedef int (DEACTIVATEP2P_FUNC(void));

#ifdef __cplusplus
extern "C" {
#endif

	P2PSERVER_API int ActivateP2P(unsigned short usListenPort = 30700);
	P2PSERVER_API int DeactivateP2P();

#ifdef __cplusplus
};
#endif
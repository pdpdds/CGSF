/*Righteous Ninja AKA P.S. Ching codediaries.blogspot.com*/
#ifndef COMMONDATABASEPOOL_CPP
#define COMMONDATABASEPOOL_CPP

#include "CrossHelper.h"
#include <map>

using namespace std;

enum{
	DBPOOL_POOLEDCON_FREE,
	DBPOOL_POOLEDCON_USED,
	DBPOOL_TEMPCON_USED
};


template <typename T>
class CommonDatabasePool{
public:
	virtual int CreatePool(int,int);
	virtual T* GetConnectionFromPool();
	virtual void ReleaseConnectionToPool(T*);
	virtual int DestroyPool();
	static void* KeepPoolAlive(void*);
	unsigned int GetLastSystemError();
	bool IsPoolValid();
	template <typename T> friend  ostream& operator<<(ostream &, CommonDatabasePool &);

protected:
	/*	These have to be implemented in the technology specific Pool 
		MySQL, Oracle, SQLServer etc
	 */
	virtual T* MyGetConcreteConnection()=0;
	virtual void MyReleaseConcreteConnection(T*)=0;
	virtual int MyPingConcreteConnection(T*)=0;
	virtual int MyGetPingTimeout()=0;
	
private:
	unsigned int poolerrorno;
	bool exitkeepalivepool;
	bool ispoolvalid;
	map<T*, short> poolmap;
	int pooledconnections;
	int maxconnections;
	XSEMAPHORE semhandle;
	XTHREADID threadhandle;
	XMUTEX	mutexhandle;
};


template <typename T>
int CommonDatabasePool<T>::CreatePool(int pooled, int max){
	pooledconnections=pooled;
	maxconnections=max;
	ispoolvalid=false;

	T* t;
	for(int i=0 ; i < pooled ; i++){
		if((t = MyGetConcreteConnection())!=0){
			if(MyPingConcreteConnection(t)==0)
				poolmap.insert(make_pair(t, DBPOOL_POOLEDCON_FREE));
			else
				MyReleaseConcreteConnection(t);
		}
	}

	exitkeepalivepool = false;
	if(XSemCreate(&semhandle)==-1)
		return -1;
	if(XMutCreate(&mutexhandle)==-1)
		return -1;
	if(XThreadCreate(&threadhandle,&CommonDatabasePool<T>::KeepPoolAlive,(void*)this)==-1)
		return -1;
	
	if(poolmap.size()==0){
		return -1;
	}
	else{
		ispoolvalid=true;
		return poolmap.size();
	}
}


template <typename T>
T* CommonDatabasePool<T>::GetConnectionFromPool(){
	
	XMutLock(&mutexhandle);
	T* t=0;
	typename map<T*, short>::iterator iter;
	for( iter = poolmap.begin(); iter != poolmap.end(); iter++ ) {
		if(iter->second == DBPOOL_POOLEDCON_FREE){
			iter->second = DBPOOL_POOLEDCON_USED;
			t = iter->first;
			break;
		}
	}
	if (t==0){
		if((int)poolmap.size()< maxconnections){
			if((t = MyGetConcreteConnection())!=0){
				MyPingConcreteConnection(t);
				poolmap.insert(make_pair(t, DBPOOL_TEMPCON_USED));
			}
		}
	}
	XMutRelease(&mutexhandle);
	return t;
}


template <typename T>
void CommonDatabasePool<T>::ReleaseConnectionToPool(T* t){
	
	XMutLock(&mutexhandle);
	typename map<T*, short>::iterator iter = poolmap.find(t);
    if( iter != poolmap.end() ) {
		if(iter->second==DBPOOL_POOLEDCON_USED)
			iter->second=DBPOOL_POOLEDCON_FREE;
		if(iter->second==DBPOOL_TEMPCON_USED){
			MyReleaseConcreteConnection(iter->first);
			poolmap.erase(iter);
		}
    }
	XMutRelease(&mutexhandle);

}

template <typename T>
int CommonDatabasePool<T>::DestroyPool(){
	typename map<T*, short>::iterator iter;
	for( iter = poolmap.begin(); !poolmap.empty()&& iter != poolmap.end(); iter++ ) {
		if(iter->second == DBPOOL_POOLEDCON_FREE){
			MyReleaseConcreteConnection(iter->first);
			poolmap.erase(iter);
		}
	}
	exitkeepalivepool = true;

	if(XSemPost(&semhandle)==-1){
	}
	if(XThreadJoin(&threadhandle)==-1){
	}

	return poolmap.size();
}

template <typename T>
void*  CommonDatabasePool<T>::KeepPoolAlive(void* p){
	CommonDatabasePool<T> * pcommondatabasepool = (CommonDatabasePool<T>*)p;

	while(!pcommondatabasepool->exitkeepalivepool && pcommondatabasepool->ispoolvalid){

		XMutLock(&pcommondatabasepool->mutexhandle);
		typename map<T*, short>::iterator iter;
		for( iter = pcommondatabasepool->poolmap.begin(); !pcommondatabasepool->poolmap.empty() && 
				iter != pcommondatabasepool->poolmap.end(); iter++ ) {
			if(iter->second == DBPOOL_POOLEDCON_FREE)
				if(pcommondatabasepool->MyPingConcreteConnection(iter->first)==-1){
					pcommondatabasepool->ReleaseConnectionToPool(iter->first);
				}
		}
		XMutRelease(&pcommondatabasepool->mutexhandle);

		if(XSemTimedWait(&pcommondatabasepool->semhandle,pcommondatabasepool->MyGetPingTimeout()) ==-1){
			pcommondatabasepool->ispoolvalid=false;
			break;
		}
	}
	return 0;
}

template <typename T>
bool CommonDatabasePool<T>::IsPoolValid(){
	return ispoolvalid;
}

template <typename T>
unsigned int CommonDatabasePool<T>::GetLastSystemError(){
	return XGetLastError();
}

template <typename T>
ostream& operator<<(ostream& os,  CommonDatabasePool<T> & cdp){
	os<<"---Pool is "<<(cdp.IsPoolValid()?"valid":"invalid")<<"----\n";
	os<<"Keepalive every "<<cdp.MyGetPingTimeout()/1000<<" seconds\n";
	typename map<T*, short>::iterator iter;
	for( iter = cdp.poolmap.begin(); iter != cdp.poolmap.end(); iter++ ) {
		os<<"pointer : "<<iter->first<<" : ";
		switch(iter->second){
			case DBPOOL_POOLEDCON_FREE:
				os<<"free\n";
				break;
			case DBPOOL_POOLEDCON_USED:
				os<<"used\n";
				break;
			case DBPOOL_TEMPCON_USED:
				os<<"temp\n";
				break;
		}
	}
	os<<endl;
	return os;
}

#endif

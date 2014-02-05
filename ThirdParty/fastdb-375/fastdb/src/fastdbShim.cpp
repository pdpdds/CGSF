#include "fastdbShim.h"
//#include "platform_specific_include.h"

/*
 * User should replace these functions with platform specific code.
 * Replace printf with platform logging facility.
 */

/*
 * All memory is shared in VxWorks. Naive implementation.
 * Replace allocation with platform dependent allocations.
 */
int shmget(key_t key, size_t size, int shmflg)
{
    int *m_WorkBuffer = (int *)malloc(size);
    printf( "In shmget  size key %p 0x%x %x\n", m_WorkBuffer, size, key);
	return (int)m_WorkBuffer;
}
void *shmat(int shmid, const void *shmaddr, int shmflg)
{
    printf( "In shmat  0x%x \n", shmid);
	return shmid;
}

int shmdt(const void *shmaddr)
{
    printf( "In shmdt  ox%x \n", shmaddr);
	free((void *)shmaddr);
	return 0;
}

int shmctl(int shmid, int cmd, void *buf)
{
	printf( "In shmctl  %p \n", buf);
	return 0;
}

/*
 * This method is added as sqrt method is not available in vxWorks math lib.
 * Note that it uses floating point which may be disabled in the platform.
 */
extern "C" double sqrt(double n)
{
    if(n==0) return 0;
    if(n==1) return 1;
    double guess = n/2;
    double oldguess = 0;
    while(guess!=oldguess)
    { 
        oldguess=guess;
        guess = (guess+n/guess)/2;
    }
    return guess;
}

/*
 * getPageSize is not available in VxWorks.
 */
extern "C" int getpagesize()
{
    int pagesize = 4*1024;
    return pagesize;
}

extern "C" int sysClkRateGet(void);
extern "C" unsigned long long tick64Get();

extern "C" int gettimeofday(struct timeval *tp, void *tzp)
{
    static int clkRate = 0;
    unsigned long long ticks;

    if ( clkRate == 0 )
        clkRate = sysClkRateGet(); /* expensive call, via sysctl in RTP */
    ticks = tick64Get(); /* also via sysctl, but unavoidable */

    /* The tv_sec member of struct timeval is presently of type 'long'.  
     * The tv_sec member of struct timespec is time_t,
     * which is unsigned long.  * We don't want to return a
     * negative result for tv_sec.
     */
    tp->tv_sec = ( long )( ticks / clkRate ) & LONG_MAX;
    tp->tv_usec = ( long )( ticks % clkRate ) * 1000000 / clkRate;

    return 0;
}

/*
 * this is not truly required for running fastdb. This is just a place holder to
 * resolve compilation issue.
 */
int uname (struct utsname *u)
{
     return 0;
}

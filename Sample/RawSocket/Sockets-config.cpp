/**
 **	\file Sockets-config.cpp
 **	\date  2006-09-04
 **	\author grymse@alhem.net
**/
/*
Copyright (C) 2007  Anders Hedstrom

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include <stdio.h>
#include <string.h>
#include "sockets-config.h"
//#include "socket_include.h"
#include <map>
#include <string>
#include <cstdlib>

// ------------------------------------------------------------------
static	bool any_set = false;
static	std::map<std::string, std::string> mmap;
static	bool quiet = false;

// ------------------------------------------------------------------
void show_info()
{
#ifdef HAVE_OPENSSL
		printf("SSL support\n");
#endif
#ifdef IPPROTO_IPV6
		printf("IPv6 support\n");
#endif
#ifdef USE_SCTP
#ifdef IPPROTO_SCTP
		printf("SCTP support\n");
#	ifdef HAVE_SCTP
		printf("  HAVE_SCTP: yes\n");
#	else
		printf("  HAVE_SCTP: no\n");
#	endif
#	ifdef HAVE_KERNEL_SCTP
		printf("  HAVE_KERNEL_SCTP: yes\n");
#	else
		printf("  HAVE_KERNEL_SCTP: no\n");
#	endif
#	ifdef HAVE_SCTP_PRSCTP
		printf("  HAVE_SCTP_PRSCTP: yes\n");
#	else
		printf("  HAVE_SCTP_PRSCTP: no\n");
#	endif
#	ifdef HAVE_SCTP_ADDIP
		printf("  HAVE_SCTP_ADDIP: yes\n");
#	else
		printf("  HAVE_SCTP_ADDIP: no\n");
#	endif
#	ifdef HAVE_SCTP_CANSET_PRIMARY
		printf("  HAVE_SCTP_CANSETPRIMARY: yes\n");
#	else
		printf("  HAVE_SCTP_CANSETPRIMARY: no\n");
#	endif
#	ifdef HAVE_SCTP_SAT_NETWORK_CAPABILITY
		printf("  HAVE_SCTP_SAT_NETWORK_CAPABILITY: yes\n");
#	else
		printf("  HAVE_SCTP_SAT_NETWORK_CAPABILITY: no\n");
#	endif
#	ifdef HAVE_SCTP_MULTIBUF
		printf("  HAVE_SCTP_MULTIBUF: yes\n");
#	else
		printf("  HAVE_SCTP_MULTIBUF: no\n");
#	endif
#	ifdef HAVE_SCTP_NOCONNECT
		printf("  HAVE_SCTP_NOCONNECT: yes\n");
#	else
		printf("  HAVE_SCTP_NOCONNECT: no\n");
#	endif
#	ifdef HAVE_SCTP_EXT_RCVINFO
		printf("  HAVE_SCTP_EXT_RCVINFO: yes\n");
#	else
		printf("  HAVE_SCTP_EXT_RCVINFO: no\n");
#	endif
#else
		printf("No SCTP support\n");
#endif
#endif
}

// ------------------------------------------------------------------
/*
#define HAVE_OPENSSL
//#define ENABLE_IPV6
//#define USE_SCTP
#define NO_GETADDRINFO
//#define ENABLE_POOL
//#define ENABLE_SOCKS4
#define ENABLE_RESOLVER
#define ENABLE_RECONNECT
#define ENABLE_DETACH
#define ENABLE_EXCEPTIONS
//#define ENABLE_XML
*/
void setup_configuration()
{
	mmap["openssl"] =     "HAVE_OPENSSL";
	mmap["ipv6"] =        "ENABLE_IPV6";
	mmap["sctp"] =        "USE_SCTP";
	mmap["no_addrinfo"] = "NO_GETADDRINFO";
	mmap["pool"] =        "ENABLE_POOL";
	mmap["socks4"] =      "ENABLE_SOCKS4";
	mmap["resolver"] =    "ENABLE_RESOLVER";
	mmap["reconnect"] =   "ENABLE_RECONNECT";
	mmap["detach"] =      "ENABLE_DETACH";
	mmap["exceptions"] =  "ENABLE_EXCEPTIONS";
	mmap["xml2"] =        "ENABLE_XML";
}

// ------------------------------------------------------------------
void set(const char *param, bool enable)
{
	std::map<std::string, std::string>::iterator it = mmap.find(param);
	if (it == mmap.end())
	{
		fprintf(stderr, "Option '%s' not found, aborting\n", param);
		exit(-1);
	}
	FILE *fil = fopen("sockets-config.h", "rt");
	if (!fil)
	{
		fprintf(stderr, "Can't open configuration file 'sockets-config.h', aborting\n");
		exit(-2);
	}
	FILE *fil2 = fopen("sockets-config.h.tmp", "wt");
	if (!fil2)
	{
		fprintf(stderr, "Can't write to temporary configuration file, aborting\n");
		exit(-3);
	}
	char slask[1000];
	fgets(slask, 1000, fil);
	while (!feof(fil))
	{
		bool replaced = false;
		while (strlen(slask) && (slask[strlen(slask) - 1] == 13 || slask[strlen(slask) - 1] == 10))
			slask[strlen(slask) - 1] = 0;
		if (!strncmp(slask, "#define ", 8) ||
		    !strncmp(slask, "//#define ", 10))
		{
			int i = (*slask == '#') ? 8 : 10;
			if (!strncmp(slask + i, it -> second.c_str(), it -> second.size()))
			{
				if (!enable)
					fprintf(fil2, "//");
				fprintf(fil2, "#define %s\n", it -> second.c_str());
				replaced = true;
			}
		}
		if (!replaced)
			fprintf(fil2, "%s\n", slask);
		//
		fgets(slask, 1000, fil);
	}
	fclose(fil);
	fclose(fil2);
#ifdef _WIN32
	unlink("sockets-config.h");
#endif
	rename("sockets-config.h.tmp", "sockets-config.h");
	any_set = true;
}

// ------------------------------------------------------------------
void show_configuration()
{
	printf("Library options (sockets-config.h)\n");
	FILE *fil = fopen("sockets-config.h", "rt");
	if (!fil)
	{
		fprintf(stderr, "Can't open configuration file 'sockets-config.h', aborting\n");
		exit(-4);
	}
	char slask[1000];
	fgets(slask, 1000, fil);
	while (!feof(fil))
	{
		while (strlen(slask) && (slask[strlen(slask) - 1] == 13 || slask[strlen(slask) - 1] == 10))
			slask[strlen(slask) - 1] = 0;
		if (!strncmp(slask, "#define ", 8) ||
		    !strncmp(slask, "//#define ", 10))
		{
			int i = (*slask == '#') ? 8 : 10;
			std::string def(&slask[i]);
			for (std::map<std::string, std::string>::iterator it = mmap.begin(); it != mmap.end(); it++)
			{
				if (def == it -> second)
				{
					std::string tmp(it -> first);
					tmp += " ";
					while (tmp.size() < 15)
						tmp += ".";
					printf("%s %s\n", tmp.c_str(), (*slask == '#') ? "ENABLED" : "DISABLED");
				}
			}
		}
		fgets(slask, 1000, fil);
	}
	fclose(fil);
	printf("Build options (Makefile)\n");
	fil = fopen("Makefile", "rt");
	if (!fil)
	{
		fprintf(stderr, "Can't read file: Makefile, aborting\n");
		exit(-7);
	}
	fgets(slask, 1000, fil);
	while (!feof(fil))
	{
		while (strlen(slask) && (slask[strlen(slask) - 1] == 13 || slask[strlen(slask) - 1] == 10))
			slask[strlen(slask) - 1] = 0;
		if (!strncmp(slask, "PLATFORM ", 9) ||
		    !strncmp(slask, "PREFIX ", 7) ||
		    (strstr(slask, "SOCKETS_NAMESPACE") && *slask != '#') ||
		    !strncmp(slask, "NAME ", 5)
		)
			printf("%s\n", slask);
		fgets(slask, 1000, fil);
	}
	fclose(fil);
}

// ------------------------------------------------------------------
void replace_makefile(const std::string& name, const std::string& value)
{
	FILE *fil = fopen("Makefile", "rt");
	if (!fil)
	{
		fprintf(stderr, "Can't read file: Makefile, aborting\n");
		exit(-5);
	}
	FILE *fil2 = fopen("Makefile.tmp", "wt");
	if (!fil2)
	{
		fprintf(stderr, "Can't write to file: Makefile.tmp, aborting\n");
		exit(-6);
	}
	char slask[1000];
	fgets(slask, 1000, fil);
	while (!feof(fil))
	{
		while (strlen(slask) && (slask[strlen(slask) - 1] == 13 || slask[strlen(slask) - 1] == 10))
			slask[strlen(slask) - 1] = 0;
		if (!strncmp(slask, name.c_str(), name.size()) && slask[name.size()] == 32)
		{
			if (name.size() < 6)
				fprintf(fil2, "%s =\t\t%s\n", name.c_str(), value.c_str());
			else
				fprintf(fil2, "%s =\t%s\n", name.c_str(), value.c_str());
		}
		else
		{
			fprintf(fil2, "%s\n", slask);
		}
		fgets(slask, 1000, fil);
	}
	fclose(fil);
	fclose(fil2);
	rename("Makefile.tmp", "Makefile");
	any_set = true;
}

// ------------------------------------------------------------------
/*
# platforms:

#  linux-x86-32
#  linux-x86-64
#  win32-cygwin
#  solaris9-sparc-64
#  macosx
#  solaris8

PLATFORM =	linux-x86-32
*/
void set_platform(const char *platform)
{
	if (!strcmp(platform, "linux-x86-32") ||
	    !strcmp(platform, "linux-x86-64") ||
	    !strcmp(platform, "win32-cygwin") ||
	    !strcmp(platform, "solaris9-sparc-64") ||
	    !strcmp(platform, "macosx") ||
	    !strcmp(platform, "solaris8"))
	{
		replace_makefile("PLATFORM", platform);
	}
}

// ------------------------------------------------------------------
void set_namespace(const char *ns)
{
/*
# uncomment these lines if the library should be in its own namespace
#CFLAGS +=	-DSOCKETS_NAMESPACE=sockets
#CFLAGS +=	-DSOCKETS_NAMESPACE_STR='"sockets"'
*/
	bool found = false;
	bool set = false;
	FILE *fil = fopen("Makefile", "rt");
	if (!fil)
	{
		fprintf(stderr, "Can't read file: Makefile, aborting\n");
		exit(-8);
	}
	FILE *fil2 = fopen("Makefile.tmp", "wt");
	if (!fil2)
	{
		fprintf(stderr, "Can't write to file: Makefile.tmp, aborting\n");
		exit(-9);
	}
	char slask[1000];
	fgets(slask, 1000, fil);
	while (!feof(fil))
	{
		while (strlen(slask) && (slask[strlen(slask) - 1] == 13 || slask[strlen(slask) - 1] == 10))
			slask[strlen(slask) - 1] = 0;
		if (strstr(slask, "SOCKETS_NAMESPACE"))
		{
			if (*slask == '#')
				fprintf(fil2, "%s\n", slask);
			else
				fprintf(fil2, "# %s\n", slask);
			found = true;
		}
		else
		if (found && !set)
		{
			fprintf(fil2, "CFLAGS +=\t-DSOCKETS_NAMESPACE=%s\n", ns);
			fprintf(fil2, "CFLAGS +=\t-DSOCKETS_NAMESPACE_STR='\"%s\"'\n", ns);
			fprintf(fil2, "%s\n", slask);
			set = true;
		}
		else
		{
			fprintf(fil2, "%s\n", slask);
		}
		fgets(slask, 1000, fil);
	}
	fclose(fil);
	fclose(fil2);
	rename("Makefile.tmp", "Makefile");
	any_set = true;
}

// ------------------------------------------------------------------
int main(int argc,char *argv[])
{
	bool info = false;
	bool usage = false;
	setup_configuration();
	for (int i = 1; i < argc; i++)
		if (!strcmp(argv[i], "-info"))
			info = true;
		else
		if (!strncmp(argv[i], "--enable-", 9))
			set(&argv[i][9], true);
		else
		if (!strncmp(argv[i], "--disable-", 10))
			set(&argv[i][10], false);
		else
		if (!strncmp(argv[i], "--platform=", 11))
			set_platform(&argv[i][11]);
		else
		if (!strncmp(argv[i], "--name=", 7))
			replace_makefile("NAME", &argv[i][7]);
		else
		if (!strncmp(argv[i], "--prefix=", 9))
			replace_makefile("PREFIX", &argv[i][9]);
		else
		if (!strncmp(argv[i], "--namespace=", 12))
			set_namespace(&argv[i][12]);
		else
		if (!strcmp(argv[i], "-v"))
			any_set = true;
		else
		if (!strcmp(argv[i], "-q"))
			quiet = true;
		else
		if (*argv[i] == '-')
		{
			fprintf(stderr, "Unknown command line option: %s\n", argv[i]);
			usage = true;
		}
		else
			usage = true;
	if (usage)
	{
		fprintf(stderr, "Usage: %s [-info] [-v] [-h] [--platform=PLATFORM] [--prefix=<path>] [--name=<Name>] [--namespace=<ns>] [--enable-OPTION] [--disable-OPTION]\n", *argv);
		fprintf(stderr, "\n");
		fprintf(stderr, "Options (default value)\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "  -info        Show compiled options\n");
		fprintf(stderr, "  -v           Show configuration options\n");
		fprintf(stderr, "  -q           Don't show extra info\n");
		fprintf(stderr, "  -h           Show this help\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "  --platform=  Select target platform\n");
		fprintf(stderr, "  --prefix=    Set installation path (/usr/local/$NAME)\n");
		fprintf(stderr, "  --name=      Set library name (Sockets)\n");
		fprintf(stderr, "  --namespace= Set C++ namespace for entire library\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "Defined platforms\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "  linux-x86-32\n");
		fprintf(stderr, "  linux-x86-64\n");
		fprintf(stderr, "  win32-cygwin\n");
		fprintf(stderr, "  solaris9-sparc-64\n");
		fprintf(stderr, "  macosx\n");
		fprintf(stderr, "  solaris8\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "Possible OPTION values:\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "  openssl      Enable openssl library\n");
		fprintf(stderr, "  ipv6         Add ipv6 support\n");
		fprintf(stderr, "  sctp         Add SCTP support\n");
		fprintf(stderr, "  no_addrinfo  Do not use addrinfo API for dns requests\n");
		fprintf(stderr, "  pool         Enable client connection pool\n");
		fprintf(stderr, "  socks4       Enable socks4 client support\n");
		fprintf(stderr, "  resolver     Enable asynchronous dns\n");
		fprintf(stderr, "  reconnect    Enable tcp reconnect\n");
		fprintf(stderr, "  detach       Enable detach functionality\n");
		fprintf(stderr, "  exceptions   Enable C++ exceptions on some error conditions\n");
		fprintf(stderr, "  xml2         Enable libxml2 support\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "Use with no options to show platform configuration options in CFLAGS format.\n");
		return 0;
	}
	if (argc > 1 && info)
	{
		show_info();
		return 0;
	}
	if (any_set)
	{
		if (!quiet)
			show_configuration();
		return 0;
	}
	printf(" -D_VERSION='\"%s\"'", _VERSION);

#ifdef LINUX
	printf(" -DLINUX");
#endif
#ifdef MACOSX
	printf(" -DMACOSX");
#endif
#ifdef SOLARIS
	printf(" -DSOLARIS");
#endif
#ifdef SOLARIS8
	printf(" -DSOLARIS8");
#endif
#ifdef _WIN32
	printf(" -D_WIN32");
#endif
#ifdef __CYGWIN__
	printf(" -D__CYGWIN__");
#endif
#ifdef SOCKETS_NAMESPACE
	printf(" -DSOCKETS_NAMESPACE=%s", SOCKETS_NAMESPACE_STR);
#endif
#ifdef _DEBUG
	printf(" -D_DEBUG");
#endif

}



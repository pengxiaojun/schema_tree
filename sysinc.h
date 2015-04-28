/* General operating system abstraction layer
 * isolate application from bloody differences among OSs and compilers
 * RULES: keep it thin, simple and maintainable
 * Creator & Maintainer: Zhang Fan (zhang.h.fan@gmail.com)
 */
#ifndef __SYS_INC_H__
#define __SYS_INC_H__

#include "unitypes.h"

#if defined(__cplusplus)
extern "C" {
#endif

#if defined _WIN32

#	define GR_WINXP_SUPPORT 0

#	if GR_WINXP_SUPPORT
#		ifdef WINVER
#			undef WINVER
#		endif
#		ifdef _WIN32_WINNT
#			undef _WIN32_WINNT
#		endif
#		define WINVER 0x0501
#		define _WIN32_WINNT 0x0501
#		include "winxp.h"
#	endif //GR_WINXP_SUPPORT

// mingw need WINVER > 0x0501 for getaddrinfo/freeaddinfo support
#	if WINVER < 0x0501
#		undef WINVER
#		define WINVER 0x0501
#	endif

#	if defined(_MSC_VER)
#		pragma comment(lib, "ws2_32.lib")
#		pragma comment(lib, "shlwapi.lib")
#	endif
#	include <WinSock2.h>
#	include <WS2tcpip.h>
#	include <process.h>
#	include <shlwapi.h>
/*#	if defined(__cplusplus) && defined(_MSC_VER)
}
#	include <atlbase.h>
extern "C" {
#	endif*/
// damn fucking MSVC C99 compliance
#	define wcstok wcstok_s
#	define wcstoll _wcstoi64
#	define strtok_r strtok_s
#	define localtime_r(a,b) (localtime_s(b,a) == 0 ? b : NULL)
#	define gmtime_r(a,b) (gmtime_s(b,a) == 0 ? b : NULL)
#	define popen(a,b) _popen((a),(b))
#	define pclose(a) _pclose((a))
#	define atoll _atoi64
#	define strtoull _strtoui64
#	define snprintf _snprintf
#	define strcasecmp stricmp
#	define strncasecmp strnicmp
#	define strcasestr strstr /* TODO: */
#	define fseeko _fseeki64
#	define ftello _ftelli64
#	if !defined(ssize_t)
#		define ssize_t long
#	endif
#elif defined unix || defined __unix || defined __unix__ // c++1y might not define 'unix'
#	include <unistd.h>
#	include <signal.h>
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <sys/time.h>
#	include <sys/stat.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <netdb.h>
#	include <dlfcn.h>
#	include <dirent.h>
#	include <fcntl.h>
#	include <sys/mman.h>

#	ifndef closesocket
#		define closesocket close
#	endif
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <netdb.h>
#	include <signal.h>
#endif

#if defined(__GNUC__)
#	if __GNUC__ == 4
#		if __GNUC_MINOR__ > 5
#			pragma GCC diagnostic ignored "-Wwrite-strings"
#			pragma GCC diagnostic ignored "-Wunused-parameter"
#		endif
#	endif
#endif

#if defined _WIN32 || defined __CYGWIN__
#	ifdef GRSO
#		ifdef __GNUC__
#			define GRSO_PUB __attribute__ ((dllexport))
#		else
#			define GRSO_PUB __declspec(dllexport)
#		endif
#	else
#		ifdef __GNUC__
#			define GRSO_PUB __attribute__ ((dllimport))
#		else
#			define GRSO_PUB __declspec(dllimport)
#		endif
#	endif
#	define GRSO_PRI
#else
#	if __GNUC__ >= 4
#		define GRSO_PUB __attribute__ ((visibility ("default")))
#		define GRSO_PRI __attribute__ ((visibility ("hidden")))
#	else
#		define GRSO_PUB
#		define GRSO_PRI
#	endif
#endif
/*
#pragma GCC visibility push(hidden)
...
#pragma GCC visibility pop
*/

#if defined(__cplusplus)
}
#endif

#endif // __SYS_INC_H__


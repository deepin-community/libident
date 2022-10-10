/*
** id_query.c                             Transmit a query to an IDENT server
**
** Author: Peter Eriksson <pen@lysator.liu.se>
*/

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#if HAVE_SYS_SELECT_H
# include <sys/select.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# endif
#endif

#define IN_LIBIDENT_SRC
#include "ident.h"


int id_query (ident_t *id, int lport, int fport, struct timeval *timeout)
{
    RETSIGTYPE (*old_sig)();
    int res;
    char buf[80];
    fd_set ws;
    
    sprintf(buf, "%d , %d\r\n", lport, fport);
    
    if (timeout)
    {
	FD_ZERO(&ws);
	FD_SET(id->fd, &ws);

	res = select(FD_SETSIZE, NULL, &ws, NULL, timeout);
	if (res < 0)
	    return -1;
	
	if (res == 0)
	{
	    errno = ETIMEDOUT;
	    return -1;
	}
    }

    old_sig = signal(SIGPIPE, SIG_IGN);
    
    res = write(id->fd, buf, strlen(buf));
    
    signal(SIGPIPE, old_sig);
    
    return res;
}

/*
** ident.c	High-level calls to the ident lib
**
** Author: Pär Emanuelsson <pell@lysator.liu.se>
** Hacked by: Peter Eriksson <pen@lysator.liu.se>
** Updated by: Rémi Denis-Courmont <rdenis@simphalempin.com>
*/

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#if HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#define IN_LIBIDENT_SRC
#include "ident.h"





/* Do a complete ident query and return result */

IDENT *ident_lookup (int fd, int timeout)
{
    struct sockaddr_storage localaddr, remoteaddr;
    int len;
    
    len = sizeof(remoteaddr);
    if (getpeername(fd, (struct sockaddr*)&remoteaddr, &len) < 0)
	return 0;
    
    len = sizeof(localaddr);
    if (getsockname(fd, (struct sockaddr *)&localaddr, &len) < 0)
	return 0;

    return ident_query_addr((struct sockaddr *)&localaddr,
				(struct sockaddr *)&remoteaddr, timeout);
}


IDENT *ident_query (const struct in_addr *laddr, const struct in_addr *faddr,
			int lport, int rport, int timeout)
{
    struct sockaddr_in lsockaddr, fsockaddr;

    memset(&lsockaddr, 0, sizeof(lsockaddr));
    lsockaddr.sin_family = AF_INET;
#if HAVE_SA_LEN
    lsockaddr.sin_len = sizeof(lsockaddr);
#endif
    lsockaddr.sin_addr.s_addr = laddr->s_addr;
    lsockaddr.sin_port = lport;

    memcpy(&fsockaddr, &lsockaddr, sizeof(fsockaddr));
    fsockaddr.sin_addr.s_addr = faddr->s_addr;
    fsockaddr.sin_port = rport;

    return ident_query_addr((struct sockaddr *)&lsockaddr,
				(struct sockaddr *)&fsockaddr, timeout);
}

IDENT *ident_query_addr (const struct sockaddr *laddr,
				const struct sockaddr *raddr, int timeout)
{
    int res, lport, rport;
    ident_t *id;
    struct timeval timout;
    IDENT *ident = NULL;

    switch (laddr->sa_family)
    {
      case AF_INET:
        /* We assume that both addresses are in the same family. */
        lport = ntohs(((struct sockaddr_in *)laddr)->sin_port);
        rport = ntohs(((struct sockaddr_in *)raddr)->sin_port);
        break;

#ifdef AF_INET6
      case AF_INET6:
        lport = ntohs(((struct sockaddr_in6 *)laddr)->sin6_port);
        rport = ntohs(((struct sockaddr_in6 *)raddr)->sin6_port);
        break;
#endif

      default:
        return NULL;
    }
    
    timout.tv_sec = timeout;
    timout.tv_usec = 0;
    
    id = id_open_addr(laddr, raddr, (timeout) ? &timout : NULL);
    if (id == NULL)
    {
	errno = EINVAL;
	return NULL;
    }
  
    if (timeout)
	res = id_query(id, rport, lport, &timout);
    else
	res = id_query(id, rport, lport, (struct timeval *) 0);
    
    if (res < 0)
    {
	id_close(id);
	return NULL;
    }
    
    ident = (IDENT *) malloc(sizeof(IDENT));
    if (ident == NULL) {
	id_close(id);
	return NULL;
    }
    
    res = id_parse(id, (timeout) ? &timout : NULL, &ident->lport,
		   &ident->fport, &ident->identifier, &ident->opsys,
		   &ident->charset);
    
    if (res != 1)
    {
	free(ident);
	id_close(id);
	return NULL;
    }
    
    id_close(id);
    return ident;			/* At last! */
}


char *ident_id (int fd, int timeout)
{
    IDENT *ident;
    char *id = NULL;
    
    ident = ident_lookup(fd, timeout);
    if (ident && ident->identifier && *ident->identifier)
    {
	id = id_strdup(ident->identifier);
	if (id == NULL)
	    return NULL;
    }

    ident_free(ident);
    return id;
}


void ident_free (IDENT * id)
{
    if (id != NULL)
    {
        if (id->identifier)
	    free(id->identifier);
	if (id->opsys)
	    free(id->opsys);
	if (id->charset)
	    free(id->charset);
	free(id);
    }
}

const char *id_version = PACKAGE_VERSION;


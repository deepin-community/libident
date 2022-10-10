/*
** id_open.c                 Establish/initiate a connection to an IDENT server
**
** Author: Peter Eriksson <pen@lysator.liu.se>
** Fixes: Pär Emanuelsson <pell@lysator.liu.se>
*/

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
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

#if HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif


ident_t *id_open (const struct in_addr *laddr, const struct in_addr *faddr,
			struct timeval *timeout)
{
    struct sockaddr_in lsockaddr, fsockaddr;

    memset(&lsockaddr, 0, sizeof(lsockaddr));
    lsockaddr.sin_family = AF_INET;
#if HAVE_SA_LEN
    lsockaddr.sin_len = sizeof(lsockaddr);
#endif
    lsockaddr.sin_addr.s_addr = laddr->s_addr;

    memcpy(&fsockaddr, &lsockaddr, sizeof(fsockaddr));
    fsockaddr.sin_addr.s_addr = faddr->s_addr;

    return id_open_addr((struct sockaddr *)&lsockaddr,
				(struct sockaddr *)&fsockaddr, timeout);
}


ident_t *id_open_addr (const struct sockaddr *laddr,
			const struct sockaddr *faddr, struct timeval *timeout)
{
    ident_t *id;
    int res, tmperrno, pf;
    struct sockaddr_storage ss_laddr, ss_faddr;
    fd_set rs, ws, es;
    const int on = 1;
    struct linger linger;

    if ((id = (ident_t *) malloc(sizeof(*id))) == NULL)
	return NULL;

    switch (faddr->sa_family)
    {
      case AF_INET:
        pf = PF_INET;
	break;

#ifdef AF_INET6
      case AF_INET6:
	pf = PF_INET6;
	break;
#endif

      default:
        free(id);
        return NULL;
    }

    if ((id->fd = socket(pf, SOCK_STREAM, 0)) < 0)
    {
	free(id);
	return NULL;
    }

    if (timeout)
    {
	if (((res = fcntl(id->fd, F_GETFL, 0)) < 0)
	 || (fcntl(id->fd, F_SETFL, res | FNDELAY) < 0))
	    goto ERROR;
    }

    /* We silently ignore errors if we can't change LINGER */
    /* New style setsockopt() */
    linger.l_onoff = 0;
    linger.l_linger = 0;

    setsockopt(id->fd, SOL_SOCKET, SO_LINGER, (void *)&linger, sizeof(linger));
    setsockopt(id->fd, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on));

    id->buf[0] = '\0';

    memcpy(&ss_laddr, laddr, sizeof(ss_laddr));
    switch (ss_laddr.ss_family)
    {
      case AF_INET:
        ((struct sockaddr_in *)&ss_laddr)->sin_port = 0;
	break;

#ifdef AF_INET6
      case AF_INET6:
        ((struct sockaddr_in6 *)&ss_laddr)->sin6_port = 0;
	break;
#endif
    }


#if HAVE_SA_LEN
    if (bind(id->fd, (struct sockaddr *) &ss_laddr, ss_laddr.ss_len) < 0)
#else
    if (bind(id->fd, (struct sockaddr *) &ss_laddr, sizeof(ss_laddr)) < 0)
#endif
    {
#ifdef DEBUG
	perror("libident: bind");
#endif
	goto ERROR;
    }

    memcpy(&ss_faddr, faddr, sizeof(ss_faddr));
    switch (ss_faddr.ss_family)
    {
      case AF_INET:
        ((struct sockaddr_in *)&ss_faddr)->sin_port = htons(IPPORT_IDENT);
	break;

#ifdef AF_INET6
      case AF_INET6:
        ((struct sockaddr_in6 *)&ss_faddr)->sin6_port = htons(IPPORT_IDENT);
	break;
#endif
    }

    errno = 0;
#if HAVE_SA_LEN
    res = connect(id->fd, (struct sockaddr *)&ss_faddr, ss_faddr.ss_len);
#else
    res = connect(id->fd, (struct sockaddr *)&ss_faddr, sizeof(ss_faddr));
#endif

    if (res < 0 && errno != EINPROGRESS)
    {
#ifdef DEBUG
	perror("libident: connect");
#endif
	goto ERROR;
    }

    if (timeout)
    {
	FD_ZERO(&rs);
	FD_ZERO(&ws);
	FD_ZERO(&es);

	FD_SET(id->fd, &rs);
	FD_SET(id->fd, &ws);
	FD_SET(id->fd, &es);

	if ((res = select(FD_SETSIZE, &rs, &ws, &es, timeout)) < 0)
	{
#ifdef DEBUG
	    perror("libident: select");
#endif
	    goto ERROR;
	}

	if (res == 0)
	{
	    errno = ETIMEDOUT;
	    goto ERROR;
	}

	if (FD_ISSET(id->fd, &es))
	    goto ERROR;

	if (!FD_ISSET(id->fd, &rs) && !FD_ISSET(id->fd, &ws))
	    goto ERROR;
    }

    return id;

  ERROR:
    tmperrno = errno;		/* Save, so close() won't erase it */
    close(id->fd);
    free(id);
    errno = tmperrno;
    return NULL;
}

/*
** ident-tester.c           A small daemon that can be used to test Ident
**                          servers
**
** Author: Peter Eriksson <pen@lysator.liu.se>, 10 Aug 1992
*/

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <errno.h>
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#if HAVE_NETDB_H
# include <netdb.h>
#endif
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#if HAVE_SYSLOG_H
# include <syslog.h>
#endif

#define IN_LIBIDENT_SRC
#include "ident.h"

/*
** Lookups the name of the connecting host, or the IP number as a string.
** buf should be at least NI_MAXHOST bytes-long.
*/
int gethost (const struct sockaddr *addr, socklen_t addrlen, char *buf)
{
  return getnameinfo(addr, addrlen, buf, NI_MAXHOST, NULL, 0, 0);
}

int getport (const struct sockaddr *addr)
{
  switch (addr->sa_family)
  {
    case AF_INET:
      return ntohs(((struct sockaddr_in *)addr)->sin_port);
#ifdef AF_INET6
    case AF_INET6:
      return ntohs(((struct sockaddr_in6 *)addr)->sin6_port);
#endif
  }
  return -1;
}

int
main (int argc, char *argv[])
{
  struct sockaddr_storage laddr, faddr;
  int len, res, lport, fport;
  ident_t *id;
  char *identifier, *opsys, *charset, hs[NI_MAXHOST];


  printf("Welcome to the IDENT server tester, version 2.0\r\n");
  printf("(Linked with libident %s)\r\n\r\n", id_version);

  fflush(stdout);

  len = sizeof(faddr);
  if (getpeername(0, (struct sockaddr *) &faddr, &len))
  {
     perror("getpeername()");
     return 1;
  }

  len = sizeof(laddr);
  if (getsockname(0, (struct sockaddr *) &laddr, &len))
  {
    perror("getsockname()");
    return 1;
  }

  res = gethost((struct sockaddr *)&faddr, sizeof(faddr), hs);
  if (res == EAI_SYSTEM)
  {
    perror("getnameinfo()");
    return 1;
  }
  else if (res != 0)
  {
    fprintf(stderr, "getnameinfo(): %s\r\n", gai_strerror (res));
    return 1;
  }

  printf("Connecting to Ident server at %s...\r\n", hs);
  fflush(stdout);

#ifdef LOG_LOCAL3
  openlog("tidentd", 0, LOG_LOCAL3);
#else
  openlog("tidentd", 0);
#endif

  id = id_open_addr((struct sockaddr *)&laddr, (struct sockaddr *)&faddr,
			NULL);
  if (!id)
  {
      if (errno)
      {
	  int saved_errno = errno;

	  perror("Connection denied");
	  fflush(stderr);

	  errno = saved_errno;
	  syslog(LOG_DEBUG, "Error: id_open_addr(): host=%s, error=%m", hs);
      }
      else
	  printf("Connection denied.\r\n");
      return 0;
  }

  fport = getport((struct sockaddr *)&faddr);
  lport = getport((struct sockaddr *)&laddr);
  printf("Querying for lport %d, fport %d....\r\n", lport, fport);
  fflush(stdout);

  errno = 0;
  if (id_query(id, fport, lport, 0) < 0)
  {
      if (errno)
      {
	  int saved_errno = errno;

	  perror("id_query()");
	  fflush(stderr);

	  errno = saved_errno;
	  syslog(LOG_DEBUG, "Error: id_query(): host=%s, error=%m", hs);

      }
      else
	  printf("Query failed.\r\n");

    return 0;
  }

  printf("Reading response data...\r\n");
  fflush(stdout);

  res = id_parse(id, NULL, &lport, &fport, &identifier, &opsys, &charset);

  switch (res)
  {
    default:
	if (errno)
	{
	    int saved_errno = errno;

	    perror("id_parse()");
	    errno = saved_errno;
	    syslog(LOG_DEBUG, "Error: id_parse(): host=%s, error=%m", hs);
	}
	else
	    printf("Error: Invalid response (empty response?).\r\n");
	break;

    case -2:
	syslog(LOG_DEBUG, "Error: id_parse(): host=%s, Parse Error: %s", hs,
		   identifier ? identifier : "<no information available>");
	if (identifier)
	    printf("Parse error on reply:\r\n  \"%s\"\r\n", identifier);
	else
	    printf("Unidentifiable parse error on reply.\r\n");
	break;

    case -3:
	syslog(LOG_DEBUG, "Error: id_parse(): host=%s, Illegal reply type: %s",
		   hs, identifier);
	printf("Parse error in reply: Illegal reply type: %s\r\n", identifier);
	break;

    case 0:
	syslog(LOG_DEBUG, "Error: id_parse(): host=%s, NotReady", hs);
	printf("Not ready. This should not happen...\r\n");
	break;

    case 2:
	syslog(LOG_DEBUG, "Reply: Error: host=%s, error=%s", hs, identifier);

	printf("Error response is:\r\n");
	printf("   Lport........ %d\r\n", lport);
	printf("   Fport........ %d\r\n", fport);
	printf("   Error........ %s\r\n", identifier);
	break;

    case 1:
      if (charset)
	syslog(LOG_INFO,
	       "Reply: Userid: host=%s, opsys=%s, charset=%s, userid=%s",
	       hs, opsys, charset, identifier);
      else
	syslog(LOG_INFO, "Reply: Userid: host=%s, opsys=%s, userid=%s",
	       hs, opsys, identifier);

      printf("Userid response is:\r\n");
      printf("   Lport........ %d\r\n", lport);
      printf("   Fport........ %d\r\n", fport);
      printf("   Opsys........ %s\r\n", opsys);
      printf("   Charset...... %s\r\n", charset ? charset : "<not specified>");
      printf("   Identifier... %s\r\n", identifier);

      if (id_query(id, getport((struct sockaddr *)&faddr),
                       getport((struct sockaddr *)&laddr), 0) >= 0)
      {
	if (id_parse(id, NULL,
		     &lport, &fport,
		     &identifier,
		     &opsys,
		     &charset) == 1)
	  printf("   Multiquery... Enabled\r\n");
      }
  }

  fflush(stdout);
  return 0;
}

/*
** ident.h
**
** Author: Peter Eriksson <pen@lysator.liu.se>
** Intruder: Pär Emanuelsson <pell@lysator.liu.se>
** Perturbed by: Rémi Denis-Courmont <rdenis@simphalempin.com>
*/

#ifndef __IDENT_H__
# define __IDENT_H__

# ifdef	__cplusplus
extern "C" {
# endif

/*
 * Unfortunately, we can't use HAVE_*_H constants because this header
 * might be used by not-autoconf-igured software. :-(
 */
# include <sys/types.h>
# include <sys/select.h> /* struct timeval (new POSIX) */
# include <sys/time.h> /* struct timeval (old POSIX) */
# include <sys/socket.h> /* struct sockaddr */
# include <netinet/in.h> /* struct in_addr */

# ifndef FD_SETSIZE
#  define FD_SETSIZE 64
# endif

# ifndef IDBUFSIZE
#  define IDBUFSIZE 2048
# endif

# ifndef IPPORT_IDENT
#  define IPPORT_IDENT	113
# endif

typedef struct
{
  int fd;
  char buf[IDBUFSIZE];
} ident_t;

typedef struct {
  int lport;                    /* Local port */
  int fport;                    /* Far (remote) port */
  char *identifier;             /* Normally user name */
  char *opsys;                  /* OS */
  char *charset;                /* Charset (what did you expect?) */
} IDENT;                        /* For higher-level routines */

/* Low-level calls and macros */
# define id_fileno(ID)	((ID)->fd)

extern ident_t *id_open_addr (const struct sockaddr *laddr,
				const struct sockaddr *faddr,
				struct timeval *timeout);
extern ident_t *id_open (const struct in_addr *laddr,
				const struct in_addr *faddr,
				struct timeval *timeout);

extern int id_close (ident_t *id);
  
extern int id_query (ident_t *id, int lport, int fport,
			struct timeval *timeout);

extern int id_parse (ident_t *id, struct timeval *timeout,
			int *lport, int *fport, char **identifier,
			char **opsys, char **charset);
  
/* High-level calls */

extern IDENT *ident_lookup (int fd, int timeout);

extern char *ident_id (int fd, int timeout); // result should be free()d

extern IDENT *ident_query_addr (const struct sockaddr *laddr,
				const struct sockaddr *raddr, int timeout);

extern IDENT *ident_query (const struct in_addr *laddr,
				const struct in_addr *faddr,
				int lport, int rport, int timeout);

extern void ident_free (IDENT *id);

extern const char *id_version;

# ifdef IN_LIBIDENT_SRC

extern char *id_strdup (char *str);
extern char *id_strtok (char *cp, char *cs, char *dc);

# endif

# ifdef	__cplusplus
}
# endif

#endif

/*
** lookup-tester.c	Tests the high-level ident calls.
**
** Author: Pär Emanuelsson <pell@lysator.liu.se>, 28 March 1993
*/

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#define IN_LIBIDENT_SRC
#include "ident.h"

int main (int argc, char *argv[])
{
  IDENT *ident;
  char *user;

  /* 
   * We can't use puts() because it send LF instead of CRLF
   * Note that perror() does not work properly either.
   */
  printf("Welcome to the other IDENT server tester, version 1.1\r\n\r\n");

  printf("Testing ident_lookup...\r\n\r\n");
  fflush(stdout);

  ident = ident_lookup(0, 30);

  if (!ident)
    perror("ident");
  else {
    printf("IDENT response is:\r\n");
    printf("   Lport........ %d\r\n", ident->lport);
    printf("   Fport........ %d\r\n", ident->fport);
    printf("   Opsys........ %s\r\n", ident->opsys);
    printf("   Charset...... %s\r\n",
	   ident->charset ? ident->charset : "<not specified>");
    printf("   Identifier... %s\r\n", ident->identifier);
  }

  ident_free(ident);

  printf("\r\nTesting ident_id...\r\n\r\n");
  fflush(stdout);

  user = ident_id(fileno(stdin), 30);

  if (user)
    printf("IDENT response is identifier = %s\r\n", user);
  else
    perror("IDENT lookup failed");

  fflush(stdout);
  return 0;
}

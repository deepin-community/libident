/*
** id_close.c                            Close a connection to an IDENT server
**
** Author: Peter Eriksson <pen@lysator.liu.se>
*/

#if HAVE_CONFIG_H
# include "config.h"
#endif
#include <stdlib.h>

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#define IN_LIBIDENT_SRC
#include "ident.h"

int id_close (ident_t *id)
{
    int res;
  
    res = close(id->fd);
    free(id);
    
    return res;
}

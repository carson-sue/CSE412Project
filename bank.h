#ifndef BANK_H
#define BANK_H

#include <stdlib.h>
#include <string.h>

#include <libpq-fe.h>

#include "display.h"

#define BUFSIZE 32

int edit_trans_rec(PGresult *, PGconn *);
int edit_cust_rec(PGresult *, PGconn *);
int insert_rec(PGresult *, PGconn *);
int delete_rec(PGresult *, PGconn *);
int view_rec(PGresult *, PGconn *, char);
int viewid_rec(PGresult *, PGconn *, char);

int accounts(PGresult *, PGconn *);
int customer(PGresult *, PGconn *);
int banks(PGresult *, PGconn *);
int transactions(PGresult *, PGconn *);

#endif

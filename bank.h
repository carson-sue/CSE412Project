#ifndef BANK_H
#define BANK_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <libpq-fe.h>

/*the password hashing requires you to install OpenSSL
 * to do this, I used https://stackoverflow.com/a/49578644
 * except I got the tar directly from their website instead
 * of using wget.*/
#include <openssl/sha.h>

/*Carson here. I needed these libraries for uint32_t,
 * htonl, and strlcpy to work on my machine. You might
 * not need them. I've commented them out assuming
 * that that's the case for most people. If you are
 * getting compilation errors involving the above
 * use these libraries.
 * for bsd/string, run sudo apt-get install libbsd-dev
 * to download the libraries and add -lbsd to every gcc
 * command in the makefile. If that doesn't work, then
 * good luck.
#include <arpa/inet.h>
#include <bsd/string.h>*/

#include "display.h"

#define BUFSIZE 32
#define ADMINNME "admin"
#define ADMINPSWD "5849d2075de68c2276306abd21e7114d8"
#define MAX_ATTEMPTS 3

bool login(char *, char *, char *, PGresult *, PGconn *);

int edit_trans_rec(PGresult *, PGconn *);
int edit_cust_rec(PGresult *, PGconn *);
int edit_bank_rec(PGresult *, PGconn *);
int edit_acc_rec(PGresult *, PGconn *);
int insert_trans_rec(PGresult *, PGconn *);
int insert_cust_rec(PGresult *, PGconn *);
int insert_bank_rec(PGresult *, PGconn *);
int insert_acc_rec(PGresult *, PGconn *);
int delete_trans_rec(PGresult *, PGconn *);
int delete_cust_rec(PGresult *, PGconn *);
int delete_bank_rec(PGresult *, PGconn *);
int delete_acc_rec(PGresult *, PGconn *);
int view_rec(PGresult *, PGconn *, char);
int viewid_rec(PGresult *, PGconn *, char);

int accounts(PGresult *, PGconn *);
int customer(PGresult *, PGconn *);
int banks(PGresult *, PGconn *);
int transactions(PGresult *, PGconn *);

int cust_view_acc(PGresult *, PGconn *, uint32_t);
int cust_view_bank(PGresult *, PGconn *, uint32_t);
int cust_view_trans(PGresult *, PGconn *, uint32_t);
int cust_view_info(PGresult *, PGconn *, uint32_t);
int cust_edit_info(PGresult *, PGconn *, uint32_t);

#endif

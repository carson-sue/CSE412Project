#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdlib.h>
#include <time.h>

#include <libpq-fe.h>

void exit_conn(PGconn *);
void exit_success(PGconn *);
void delete_error(PGresult *res, PGconn *conn);
void select_error(PGresult *res, PGconn *conn);
void insert_error(PGresult *res, PGconn *conn);
void display_insertion_menu(PGresult *res);
void display_results(PGresult *res);

#endif

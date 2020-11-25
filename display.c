/*
 * display.c - Error handling and menu display functions.
 */

#include "display.h"

void
exit_conn(PGconn *conn)
{
	PQfinish(conn);
	exit(EXIT_FAILURE);
}

void
exit_success(PGconn *conn)
{
	time_t		t;
	struct tm	tm;

	t = time(NULL);
	tm = *localtime(&t);
	printf("\n=========== %d-%02d-%02d %02d:%02d:%02d =========== \n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	printf("===========  Connection closed  ===========\n");
	PQfinish(conn);
	exit(EXIT_FAILURE);
}

void
delete_error(PGresult *res, PGconn *conn)
{
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "DELETE failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_conn(conn);
	}
}


void
select_error(PGresult *res, PGconn *conn)
{
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		fprintf(stderr, "SELECT failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_conn(conn);
	}
}

void
insert_error(PGresult *res, PGconn *conn)
{
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "INSERT failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_conn(conn);
	}
}

void
display_insertion_menu(PGresult *res)
{
	int nFields;

	/* Print attributes. */
	nFields = PQnfields(res);
	for (int i = 0; i < nFields; i++)
		printf("%s, ", PQfname(res, i));
	printf("\n");
}

void
display_results(PGresult *res)
{
	int nFields;

	/* Print attributes. */
	nFields = PQnfields(res);
	for (int i = 0; i < nFields; i++)
		printf("%-30.25s", PQfname(res, i));
	printf("\n");

	/* Print rows. */
	for (int i = 0; i < PQntuples(res); i++) {
		for (int j = 0; j < nFields; j++)
			printf("%-30.25s", PQgetvalue(res, i, j));
		printf("\n");
	}

	PQclear(res);
}

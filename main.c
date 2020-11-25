/*
 * Main driver program.
 */

#include <stdio.h>

#include "bank.h"
#include "display.h"

int
main(int argc, char *argv[])
{
	const char	*conninfo;
	PGconn		*conn;
	PGresult	*res = NULL;
	time_t		t;
	struct tm	tm;
	char		c;

	/* Specify the name of the database. */
	if (argc > 1)
		conninfo = argv[1];
	else
		conninfo = "dbname = test";

	/* Connect to database. */
	conn = PQconnectdb(conninfo);
	if (PQstatus(conn) != CONNECTION_OK) {
		fprintf(stderr, "Connection to database failed: %s",
				PQerrorMessage(conn));
		exit_conn(conn);
	}
	printf("Connection successful\n");

	/* Display main menu. */
	printf("\n=========== Welcome to ASU Bank =========== ");

	t = time(NULL);
	tm = *localtime(&t);
	printf("\n=========== %d-%02d-%02d %02d:%02d:%02d =========== \n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	do {
		printf("\n===========  Choose an option:  ===========\n");
		printf("\t-a View/edit accounts\n\t-b View banks"
			"\n\t-c View/edit customers\n\t"
			"-t View/edit transactions\n\t-x Exit\n");
		c = getchar();
		getchar();
		if (c == 'a')
			accounts(res, conn);
		else if (c == 'b')
			banks(res, conn);
		else if (c == 'c')
			customer(res, conn);
		else if (c == 't') {
			transactions(res, conn);
		}
		else
			exit_success(conn);

	} while (c != EOF && c != '\n' && c != 'x');

	exit_conn(conn);

	return 0;
}

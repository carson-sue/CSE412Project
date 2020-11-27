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
	int		attempt = 0;
	char		c;
	char 		userType;
	char*		name = (char *) malloc(BUFSIZE*sizeof(char));
	char*		pass = (char *) malloc(BUFSIZE*sizeof(char));

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
		if(attempt > 0)
			printf("The username or password was incorrect. Try agian.\n");

		printf("\nUsername: ");
		scanf("%s", name);
		printf("Password: ");
		scanf("%s", pass);

		attempt++;
	} while(!login(name, pass, &userType, res, conn) && attempt < MAX_ATTEMPTS);
	free(pass);
	getchar();

	if(attempt >= MAX_ATTEMPTS && userType != 'a' && userType != 'c')
	{
		printf("\nToo many failed attempts. Aborting...\n");
		exit_success(conn);
	}

	printf("\nWelcome %s\n", name);
	if(userType == 'a')
	{
		do {
			printf("\n===========  Choose an option:  ===========\n");
			printf("\t-a View/edit accounts\n\t-b View/edit banks"
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
			else if (c == 't') 
				transactions(res, conn);
			else
			exit_success(conn);

		} while (c != EOF && c != '\n' && c != 'x');
	}
	else if(userType == 'c')
	{
		const char *uvalues[1] = {name};
		int ulength[1] = {sizeof(name)};
		int ubinary[1] = {0};
		uint32_t id;
		
		
		res = PQexecParams(conn, "SELECT cusid FROM customer WHERE "
			       "usern = $1::varchar", 1, NULL, uvalues, ulength, ubinary, 0);
		select_error(res, conn);
		
		id = htonl((uint32_t) atoi(PQgetvalue(res, 0, 0)));
		
		do {
			printf("\n===========  Choose an option:  ===========\n");
			printf("\t-a View your accounts\n\t-b View banks locations"
				"\n\t-t View your transactions\n\t-v View your info"
				"\n\t-e Edit your info\n\t-x Exit\n");
			c = getchar();
			getchar();
			if (c == 'a')
				cust_view_acc(res, conn, id);
			else if (c == 'b')
				cust_view_bank(res, conn, id);
			else if (c == 't')
				cust_view_trans(res, conn, id);
			else if (c == 'v') 
				cust_view_info(res, conn, id);
			else if (c == 'e') 
				cust_edit_info(res, conn, id);
			else
			exit_success(conn);

		} while (c != EOF && c != '\n' && c != 'x');
	}
	else
		printf("Something has gone horribly wrong.\n");

	exit_conn(conn);

	return 0;
}

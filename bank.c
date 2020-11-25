/*
 * bank.c - Database transaction functions.
 */

#include "bank.h"

int
edit_trans_rec(PGresult *res, PGconn *conn)
{
	const char	*evalues[3];
	int		elength[3];
	int		ebinary[3] = {0, 0, 1};
	uint32_t	keyid;

	evalues[0] = malloc(BUFSIZE);
	evalues[1] = malloc(BUFSIZE);

	printf("Enter transaction ID to update: ");
	scanf(" %d", &keyid);
	getchar();
	keyid = htonl((uint32_t) keyid);
	evalues[2] = (char *) &keyid;
	elength[2] = sizeof(keyid);

	printf("Enter new merchant: ");
	scanf("%32[^\n]", (char *) evalues[0]);
	getchar();
	elength[0] = strlen(evalues[0]);

	printf("Enter new transaction description: ");
	scanf("%32[^\n]", (char *) evalues[1]);
	getchar();
	elength[1] = strlen(evalues[1]);

	res = PQexecParams(conn, "UPDATE transaction"
			" SET description = $2::varchar(32),"
			" merchant = $1::varchar(32)"
			" WHERE transid = $3::int4", 3, NULL, evalues, elength,
			ebinary, 0);
	delete_error(res, conn);

	printf("Status: %s\n", PQcmdStatus(res));

	free((void *) evalues[0]);
	free((void *) evalues[1]);

	return 0;
}

int
edit_cust_rec(PGresult *res, PGconn *conn)
{
	const char	*evalues[3];
	int		elength[3];
	int		ebinary[3] = {0, 1, 1};
	uint32_t	keyid, bankid;

	evalues[0] = malloc(BUFSIZE);

	printf("Enter customer ID to update: ");
	scanf(" %d", &keyid);
	getchar();
	keyid = htonl((uint32_t) keyid);
	evalues[2] = (char *) &keyid;
	elength[2] = sizeof(keyid);

	printf("Enter new customer name: ");
	scanf("%32[^\n]", (char *) evalues[0]);
	getchar();
	elength[0] = strlen(evalues[0]);

	printf("Enter new bank ID: ");
	scanf(" %d", &bankid);
	getchar();
	bankid = htonl((uint32_t) bankid);
	evalues[1] = (char *) &bankid;
	elength[1] = sizeof(bankid);

	res = PQexecParams(conn, "UPDATE customer"
			" SET bankid = $2::int4,"
			" fullname = $1::varchar(32)"
			" WHERE cusid = $3::int4", 3, NULL, evalues, elength,
			ebinary, 0);
	delete_error(res, conn);

	printf("Status: %s\n", PQcmdStatus(res));

	free((void *) evalues[0]);

	return 0;
}

int
insert_rec(PGresult *res, PGconn *conn)
{
	float		amount;
	int		c, accID, transID;
	char		buf[BUFSIZE * 8];
	char		str[3][BUFSIZE];

	printf("Enter merchant: ");
	scanf("%32[^\n]", (char *) str[0]);
	getchar();

	printf("Enter description: ");
	scanf("%32[^\n]", (char *) str[1]);
	getchar();

	printf("Enter date: ");
	scanf("%32[^\n]", (char *) str[2]);
	getchar();

	printf("Enter transaction amount: ");
	scanf(" %f", &amount);
	getchar();

	printf("Enter transaction ID: ");
	scanf(" %d", &transID);
	getchar();

	printf("Enter account ID: ");
	scanf(" %d", &accID);
	getchar();

	c = snprintf(buf, BUFSIZE * 8, "INSERT INTO transaction VALUES ('%s', '%s', '%s', %f, %d, %d)", str[0], str[1], str[2], amount, transID, accID);

	res = PQexecParams(conn, buf, 0, NULL, NULL, NULL, NULL, 0);
	insert_error(res, conn);
	printf("Status: %s\n", PQcmdStatus(res));
	PQclear(res);

	res = PQexecParams(conn, "SELECT * FROM transaction", 0, NULL, NULL, NULL, NULL, 0);
	select_error(res, conn);
	display_insertion_menu(res);

	return 0;
}

int
view_rec(PGresult *res, PGconn *conn, char t)
{
	int		c, f = 0, s = 0;
	char		str[3][BUFSIZE];
	char		table[BUFSIZE];
	char		sortorder[BUFSIZE];
	char		buf[BUFSIZE * 8];

	strlcpy(sortorder, "ASC", BUFSIZE);

	if (t == 't')
		strlcpy(table, "transaction", BUFSIZE);
	else if (t == 'a')
		strlcpy(table, "account", BUFSIZE);
	else if (t == 'b')
		strlcpy(table, "bankbranch", BUFSIZE);
	else if (t == 'c')
		strlcpy(table, "customer", BUFSIZE);
	else {
		fprintf(stderr, "Invalid type in view_rec");
		exit(EXIT_FAILURE);
	}

	printf("Filter by field? (y/n) ");
	c = getchar(); getchar();

	if (c == 'Y' || c == 'y') {
		f = 1;
		printf("Enter field to filter by: ");
		scanf("%32[^\n]", (char *) str[0]);
		getchar();
		printf("Enter field value: ");
		scanf("%32[^\n]", (char *) str[1]);
		getchar();
	}

	printf("Sort by field? (y/n) ");
	c = getchar(); getchar();

	if (c == 'Y' || c == 'y') {
		s = 1;
		printf("Enter field to sort by: ");
		scanf("%32[^\n]", (char *) str[2]);
		getchar();
		printf("Ascending or descending? (a/d) ");
		c = getchar(); getchar();
		if (c == 'd')
			strlcpy(sortorder, "DESC", BUFSIZE);
	}

	if (f && s) {
		c = snprintf(buf, BUFSIZE * 8, "SELECT * FROM %s WHERE "
			"LOWER(%s) = LOWER('%s') ORDER BY %s %s", table,
			str[0], str[1], str[2], sortorder);
	}
	else if (f) {
		c = snprintf(buf, BUFSIZE * 8, "SELECT * FROM %s WHERE "
			"LOWER(%s) = LOWER('%s')", table, str[0], str[1]);
	}
	else if (s) {
		c = snprintf(buf, BUFSIZE * 8, "SELECT * FROM %s "
				"ORDER BY %s %s", table, str[2], sortorder);
	}
	else
		c = snprintf(buf, BUFSIZE * 8, "SELECT * FROM %s", table);

	res = PQexecParams(conn, buf, 0, NULL, NULL, NULL, NULL, 0);
	select_error(res, conn);
	printf("Status: %s\n", PQcmdStatus(res));
	display_results(res);

	return 0;
}

int
viewid_rec(PGresult *res, PGconn *conn, char type)
{
	uint32_t	keyid;
	const char	*vvalue[1];
	int		vlength[1];
	int		vbinary[1];
	int		c;

	vlength[0] = sizeof(keyid);
	vbinary[0] = 1;

	if (type == 'a')
		printf("Filter by Customer ID? (y/n) ");
	else if (type == 'b' || type == 'c')
		printf("Filter by Bank ID? (y/n) ");
	else {
		fprintf(stderr, "Invalid option in viewid_rec");
		exit(EXIT_FAILURE);
	}
	c = getchar();
	getchar();

	if ((c == 'Y' || c == 'y') && (type == 'b' || type == 'c')) {
		printf("Enter bank ID: ");
		scanf(" %d", &keyid);
		getchar();
		keyid = htonl((uint32_t) keyid);
		vvalue[0] = (char *) &keyid;

		if (type == 'b') {
			res = PQexecParams(conn, "SELECT * FROM bankbranch "
				"WHERE bankid = $1::int4 ", 1, NULL,
				vvalue, vlength, vbinary, 0);
		}
		else {
			res = PQexecParams(conn, "SELECT * FROM customer "
				"WHERE bankid = $1::int4 ", 1, NULL,
				vvalue, vlength, vbinary, 0);
		}
	}
	else if ((c == 'Y' || c == 'y') && type == 'a') {
		printf("Enter customer ID: ");
		scanf(" %d", &keyid);
		getchar();
		keyid = htonl((uint32_t) keyid);
		vvalue[0] = (char *) &keyid;
		res = PQexecParams(conn, "SELECT * FROM account WHERE "
			"cusid = $1::int4", 1, NULL, vvalue, vlength, vbinary, 0);
	}
	else {
		if (type == 'a') {
			res = PQexecParams(conn, "SELECT * FROM account", 0, 
				NULL, NULL, NULL, NULL, 0);
		}
		else if (type == 'b') {
			res = PQexecParams(conn, "SELECT * FROM bankbranch", 0, 
				NULL, NULL, NULL, NULL, 0);
		}
		else if (type == 'c') {
			res = PQexecParams(conn, "SELECT * FROM customer", 0, 
				NULL, NULL, NULL, NULL, 0);
		}
		else {
			fprintf(stderr, "Invalid option in viewid_rec");
			exit(EXIT_FAILURE);
		}
	}

	printf("Status: %s\n", PQcmdStatus(res));
	select_error(res, conn);
	display_results(res);

	return 0;
}

int
delete_rec(PGresult *res, PGconn *conn)
{
	uint32_t	keyid;
	const char	*dvalues[1];
	int		dlength[1];
	int		dbinary[1];

	printf("Enter transaction ID to delete: ");
	scanf(" %d", &keyid);
	getchar();
	keyid = htonl((uint32_t) keyid);
	dvalues[0] = (char *) &keyid;
	dlength[0] = sizeof(keyid);
	dbinary[0] = 1;
	res = PQexecParams(conn, "DELETE FROM transaction WHERE transid = $1::int4", 1, NULL, dvalues, dlength, dbinary, 1);
	printf("Status: %s\n", PQcmdStatus(res));
	delete_error(res, conn);

	return 0;
}

int
accounts(PGresult *res, PGconn *conn)
{
	char	c;

	do {
		printf("\n===========       Accounts      ===========\n");
		printf("Choose an option:\n");
		printf("\n\t-v View bank accounts\n\t"
			"-q Quit to main menu\n\t-x Exit program\n");
		c = getchar();
		getchar();
		if (c == 'v')
			viewid_rec(res, conn, 'a');
		else if (c == 'x')
			exit_success(conn);
	} while (c != EOF && c != '\n' && c != 'q');

	return 0;
}

int
customer(PGresult *res, PGconn *conn)
{
	char	c;

	do {
		printf("\n===========      Customers      ===========\n");
		printf("Choose an option:\n");
		printf("\n\t-e Edit customer information\n\t"
			"-v View customer accounts\n\t"
			"-q Quit to main menu\n\t-x Exit program\n");
		c = getchar();
		getchar();
		if (c == 'v')
			viewid_rec(res, conn, 'c');
		else if (c == 'e')
			edit_cust_rec(res, conn);
		else if (c == 'x')
			exit_success(conn);
	} while (c != EOF && c != '\n' && c != 'q');

	return 0;
}

int
banks(PGresult *res, PGconn *conn)
{
	char	c;

	do {
		printf("\n===========        Banks        ===========\n");
		printf("Choose an option:\n");
		printf("\n\t-v View bank branches\n\t"
			"-q Quit to main menu\n\t-x Exit program\n");
		c = getchar();
		getchar();
		if (c == 'v')
			viewid_rec(res, conn, 'b');
		else if (c == 'x')
			exit_success(conn);
	} while (c != EOF && c != '\n' && c != 'q');

	return 0;
}

int
transactions(PGresult *res, PGconn *conn)
{
	char	c;

	do {
		printf("\n===========     Transactions    ===========\n");
		printf("Choose an option:\n");
		printf("\n\t-d Delete transaction\n\t-e Edit transaction\n\t"
			"-i Insert transaction\n\t-v View transactions\n\t"
			"-q Quit to main menu\n\t-x Exit program\n");
		c = getchar();
		getchar();
		if (c == 'd')
			delete_rec(res, conn);
		else if (c == 'e')
			edit_trans_rec(res, conn);
		else if (c == 'i')
			insert_rec(res, conn);
		else if (c == 'v')
			view_rec(res, conn, 't');
		else if (c == 'x')
			exit_success(conn);
	} while (c != EOF && c != '\n' && c != 'q');

	return 0;
}

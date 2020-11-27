/*
 * bank.c - Database transaction functions.
 */

#include "bank.h"

char* generateHash(char* pass)
{
	unsigned char hash[SHA256_DIGEST_LENGTH];
	char* hashString = (char*) malloc(SHA256_DIGEST_LENGTH * sizeof(char));

	SHA256_CTX context;
	SHA256_Init(&context);
	SHA256_Update(&context, pass, strlen(pass));
	SHA256_Final(hash, &context);
	
	/*This only gets every other character in the hash but I'm sure that's fine
	 * I tried for like 2 hours to get both characters before giving up*/
	for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
		sprintf(hashString+i, "%02x", hash[i]);

	return hashString;
}

bool login(char* name, char* pass, char* userType, PGresult *res, PGconn *conn)
{
	char* hash = generateHash(pass);

	/*Since I wasn't sure if I could modify our tables since
	that would modify our schema, I instead opted to add
	an admin login account that has it's info stored in
	bank.h*/
	if(strcmp(name, ADMINNME) == 0 &&
	   strcmp(hash, ADMINPSWD) == 0)
	{
		*userType = 'a';
		return true;
	}
	else
	{
		const char *evalues[2] = {name, hash};
		int elength[2] = {sizeof(name), sizeof(hash)};
		int ebinary[2] = {0, 0};

		res = PQexecParams(conn, "SELECT usern, phash FROM customer WHERE "
			       "usern = $1::varchar and phash = $2::varchar", 2, NULL, evalues, elength, ebinary, 0);
		select_error(res, conn);

		if(PQntuples(res) == 1)
		{
			*userType = 'c';
			return true;
		}
		else if(PQntuples(res) > 1)
			printf("What\n");
	}

	return false;
}

int
edit_trans_rec(PGresult *res, PGconn *conn)
{
	const char	*evalues[6], *svalues[1], *uvalues[2];
	int		elength[6], slength[1], ulength[2];
	int		ebinary[6] = {0, 0, 0, 0, 1, 1}, sbinary[1] = {1}, ubinary[2] = {0, 1};
	uint32_t	keyid, accid;
	float		amount, balance;

	evalues[0] = malloc(BUFSIZE);
	evalues[1] = malloc(BUFSIZE);
	evalues[2] = malloc(BUFSIZE);
	evalues[3] = malloc(BUFSIZE);
	uvalues[0] = malloc(BUFSIZE);

	printf("Enter transaction ID to update: ");
	scanf(" %d", &keyid);
	getchar();
	keyid = htonl((uint32_t) keyid);
	evalues[5] = (char *) &keyid;
	elength[5] = sizeof(keyid);

	printf("Enter new merchant: ");
	scanf("%32[^\n]", (char *) evalues[0]);
	getchar();
	elength[0] = strlen(evalues[0]);

	printf("Enter new transaction description: ");
	scanf("%32[^\n]", (char *) evalues[1]);
	getchar();
	elength[1] = strlen(evalues[1]);
	
	printf("Enter new date: ");
	scanf("%32[^\n]", (char *) evalues[2]);
	getchar();
	elength[2] = strlen(evalues[2]);
	
	printf("Enter new amount: ");
	scanf("%32[^\n]", (char *) evalues[3]);
	getchar();
	elength[3] = strlen(evalues[3]);
	amount = atof(evalues[3]);
	
	printf("Enter new account id: ");
	scanf(" %d", &accid);
	getchar();
	accid = htonl((uint32_t) accid);
	evalues[4] = (char *) &accid;
	elength[4] = sizeof(accid);
	svalues[0] = (char *) &accid;
	slength[0] = sizeof(accid);
	uvalues[1] = (char *) &accid;
	ulength[1] = sizeof(accid);

	res = PQexecParams(conn, "SELECT balance FROM account "
				"WHERE accid = $1::int4", 1, NULL, svalues,
				slength, sbinary, 0);
	select_error(res, conn);

	if(PQntuples(res) == 1)
	{
		balance = atof(PQgetvalue(res, 0, 0));
	}
	else
	{
		printf("\nERROR: ACCOUNT NOT FOUND. ABORTING EDIT\n");
		return 0;
	}

	svalues[0] = (char *) &keyid;
	slength[0] = sizeof(keyid);

	res = PQexecParams(conn, "SELECT amount FROM transaction "
				"WHERE transid = $1::int4", 1, NULL, svalues,
				slength, sbinary, 0);
	select_error(res, conn);

	if(PQntuples(res) == 1)
	{
		float temp = atof(PQgetvalue(res, 0, 0));
		amount -= temp;
		amount += balance;
		
		snprintf((char *)uvalues[0], BUFSIZE, "%f", amount);
		ulength[0] = strlen(svalues[0]);
	}

	res = PQexecParams(conn, "UPDATE transaction"
			" SET description = $2::varchar(32),"
			" merchant = $1::varchar(32),"
			" date = $3::date,"
			" amount = $4::numeric(10,2),"
			" accid = $5::int4"
			" WHERE transid = $6::int4", 6, NULL, evalues, elength,
			ebinary, 0);
	delete_error(res, conn);

	res = PQexecParams(conn, "UPDATE account"
			" SET balance = $1::numeric(12,2)"
			" WHERE accid = $2::int4", 2, NULL, uvalues,
			ulength, ubinary, 0);
	delete_error(res, conn);

	printf("Status: %s\n", PQcmdStatus(res));

	free((void *) evalues[0]);
	free((void *) evalues[1]);
	free((void *) evalues[2]);
	free((void *) evalues[3]);
	free((void *) uvalues[0]);

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
			" SET fullname = $1::varchar(32),"
			" bankid = $2::int4"
			" WHERE cusid = $3::int4", 3, NULL, evalues, elength,
			ebinary, 0);
	delete_error(res, conn);

	printf("Status: %s\n", PQcmdStatus(res));

	free((void *) evalues[0]);

	return 0;
}

int
edit_bank_rec(PGresult *res, PGconn *conn)
{
	const char	*evalues[7];
	int		elength[7];
	int		ebinary[7] = {0, 0, 0, 0, 1, 0, 0};
	uint32_t	bankid;

	evalues[0] = malloc(BUFSIZE);
	evalues[1] = malloc(BUFSIZE);
	evalues[2] = malloc(BUFSIZE);
	evalues[3] = malloc(BUFSIZE);
	evalues[5] = malloc(BUFSIZE);
	evalues[6] = malloc(BUFSIZE);

	printf("Enter bank state to update: ");
	scanf("%32[^\n]", (char *) evalues[5]);
	getchar();
	elength[5] = strlen(evalues[5]);

	printf("Enter bank address to update: ");
	scanf("%32[^\n]", (char *) evalues[6]);
	getchar();
	elength[6] = strlen(evalues[6]);

	printf("Enter new address: ");
	scanf("%32[^\n]", (char *) evalues[0]);
	getchar();
	elength[0] = strlen(evalues[0]);

	printf("Enter new city: ");
	scanf("%32[^\n]", (char *) evalues[1]);
	getchar();
	elength[1] = strlen(evalues[1]);

	printf("Enter new state: ");
	scanf("%32[^\n]", (char *) evalues[2]);
	getchar();
	elength[2] = strlen(evalues[2]);

	printf("Enter new zipcode: ");
	scanf("%32[^\n]", (char *) evalues[3]);
	getchar();
	elength[3] = strlen(evalues[3]);

	printf("Enter new bank ID: ");
	scanf(" %d", &bankid);
	getchar();
	bankid = htonl((uint32_t) bankid);
	evalues[4] = (char *) &bankid;
	elength[4] = sizeof(bankid);

	res = PQexecParams(conn, "UPDATE bankbranch"
			" SET addr = $1::varchar(50),"
			" city = $2::varchar(30),"
			" state = $3::varchar(2),"
			" zip = $4::varchar(5),"
			" bankid = $5::int4"
			" WHERE state = $6::varchar(2)"
			" and addr = $7::varchar(50)", 7, NULL, evalues, elength, ebinary, 0);
	delete_error(res, conn);

	printf("Status: %s\n", PQcmdStatus(res));

	free((void *) evalues[0]);
	free((void *) evalues[1]);
	free((void *) evalues[2]);
	free((void *) evalues[3]);
	free((void *) evalues[5]);
	free((void *) evalues[6]);
	
	return 0;
}

int
edit_acc_rec(PGresult *res, PGconn *conn)
{
	const char	*evalues[2];
	int		elength[2];
	int		ebinary[2] = {0, 1};
	uint32_t	accid;

	evalues[0] = malloc(BUFSIZE);

	printf("Enter account ID to update: ");
	scanf(" %d", &accid);
	getchar();
	accid = htonl((uint32_t) accid);
	evalues[1] = (char *) &accid;
	elength[1] = sizeof(accid);

	printf("Enter new irate: ");
	scanf("%32[^\n]", (char *) evalues[0]);
	getchar();
	elength[0] = strlen(evalues[0]);

	res = PQexecParams(conn, "UPDATE account"
			" SET irate = $1::numeric(5,2)"
			" WHERE accid = $2::int4", 2, NULL, evalues, elength,
			ebinary, 0);
	delete_error(res, conn);

	printf("Status: %s\n", PQcmdStatus(res));

	free((void *) evalues[0]);

	return 0;
}

int
insert_trans_rec(PGresult *res, PGconn *conn)
{
	float		amount, balance, update;
	int		accID, transID;
	char		buf[BUFSIZE * 8];
	char		str[3][BUFSIZE];
	const char	*svalues[1], *uvalues[2];
	int		slength[1], ulength[2];
	int		sbinary[1] = {1}, ubinary[2] = {0, 1};
	uint32_t	account;

	uvalues[0] = malloc(BUFSIZE);

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

	account = htonl((uint32_t) accID);
	svalues[0] = (char *) &account;
	slength[0] = sizeof(account);
	uvalues[1] = (char *) &account;
	ulength[1] = sizeof(account);
	
	res = PQexecParams(conn, "SELECT balance FROM account "
				"WHERE accid = $1::int4", 1, NULL, svalues,
				slength, sbinary, 0);
	select_error(res, conn);

	if(PQntuples(res) == 1)
	{
		balance = atof(PQgetvalue(res, 0, 0));
		update = amount + balance;
		printf("\nAmount: %f Balance: %f Update: %f\n", amount, balance, update);

		snprintf((char *)uvalues[0], BUFSIZE, "%f", update);
		ulength[0] = strlen(svalues[0]);
	}
	else
	{
		printf("\nERROR: ACCOUNT NOT FOUND. ABORTING INSERT\n");
		return 0;
	}

	snprintf(buf, BUFSIZE * 8, "INSERT INTO transaction VALUES ('%s', '%s', '%s', %f, %d, %d)",
			str[0], str[1], str[2], amount, transID, accID);

	res = PQexecParams(conn, buf, 0, NULL, NULL, NULL, NULL, 0);
	insert_error(res, conn);
	printf("Status: %s\n", PQcmdStatus(res));
	PQclear(res);

	res = PQexecParams(conn, "SELECT * FROM transaction", 0, NULL, NULL, NULL, NULL, 0);
	select_error(res, conn);
	display_insertion_menu(res);
	
	res = PQexecParams(conn, "UPDATE account"
			" SET balance = $1::numeric(12,2)"
			" WHERE accid = $2::int4", 2, NULL, uvalues,
			ulength, ubinary, 0);
	delete_error(res, conn);

	free((void *) uvalues[0]);

	return 0;
}

int
insert_cust_rec(PGresult *res, PGconn *conn)
{
	float		balance, irate;
	int		accID, custID, bankID;
	char		buf[BUFSIZE * 8];
	char		str[3][BUFSIZE];
	char		*hash;
	const char	*evalues[1], *svalues[2];
	int		elength[1], slength[2];
	int		ebinary[1] = {1}, sbinary[2] = {0, 1};
	uint32_t	queryID;

	printf("Enter username: ");
	scanf("%32[^\n]", (char *) str[0]);
	getchar();

	printf("Enter password: ");
	scanf("%32[^\n]", (char *) str[1]);
	getchar();
	
	printf("Enter full name: ");
	scanf("%32[^\n]", (char *) str[2]);
	getchar();

	printf("Enter customer id: ");
	scanf(" %d", &custID);
	getchar();

	printf("Enter bank ID: ");
	scanf(" %d", &bankID);
	getchar();

	printf("Enter account ID: ");
	scanf(" %d", &accID);
	getchar();

	printf("Enter intial balance: ");
	scanf(" %f", &balance);
	getchar();
	
	printf("Enter irate: ");
	scanf(" %f", &irate);
	getchar();

	svalues[0] = str[0];
	slength[0] = strlen(str[0]);

	queryID = htonl((uint32_t) custID);
	svalues[1] = (char *) &queryID;
	slength[1] = sizeof(queryID);
	
	res = PQexecParams(conn, "SELECT * FROM customer "
				"WHERE usern = $1::character(50) "
				"or cusid = $2::int4", 2, NULL, svalues,
				slength, sbinary, 0);
	select_error(res, conn);

	if(PQntuples(res) > 0)
	{
		printf("\nERROR. USERNAME OR CUSTOMER ID IS TAKEN. ABORTING INSERT...\n");
		return 0;
	}
	
	queryID = htonl((uint32_t) bankID);
	evalues[0] = (char *) &queryID;
	elength[0] = sizeof(queryID);
	
	res = PQexecParams(conn, "SELECT * FROM bank "
				"WHERE bankid = $1::int4", 1, NULL, evalues,
				elength, ebinary, 0);
	select_error(res, conn);

	if(PQntuples(res) == 0)
	{
		printf("\nERROR. BANK NOT FOUND. ABORTING INSERT...\n");
		return 0;
	}
	
	queryID = htonl((uint32_t) accID);
	evalues[0] = (char *) &queryID;
	elength[0] = sizeof(queryID);
	
	res = PQexecParams(conn, "SELECT * FROM account "
				"WHERE accid = $1::int4", 1, NULL, evalues,
				elength, ebinary, 0);
	select_error(res, conn);

	if(PQntuples(res) > 0)
	{
		printf("\nERROR. ACCOUNT ID IS TAKEN. ABORTING INSERT...\n");
		return 0;
	}

	hash = generateHash(str[1]);

	snprintf(buf, BUFSIZE * 8, "INSERT INTO customer VALUES ('%s', '%s', '%s', %d, %d)",
		       	str[0], hash, str[2], custID, bankID);

	res = PQexecParams(conn, buf, 0, NULL, NULL, NULL, NULL, 0);
	insert_error(res, conn);
	printf("Status: %s\n", PQcmdStatus(res));
	PQclear(res);

	res = PQexecParams(conn, "SELECT * FROM customer", 0, NULL, NULL, NULL, NULL, 0);
	select_error(res, conn);
	display_insertion_menu(res);

	snprintf(buf, BUFSIZE * 8, "INSERT INTO account VALUES (%f, %f, %d, %d)", balance, irate, accID, custID);

	res = PQexecParams(conn, buf, 0, NULL, NULL, NULL, NULL, 0);
	insert_error(res, conn);
	printf("Status: %s\n", PQcmdStatus(res));
	PQclear(res);

	res = PQexecParams(conn, "SELECT * FROM account", 0, NULL, NULL, NULL, NULL, 0);
	select_error(res, conn);
	display_insertion_menu(res);
	
	return 0;
}

int
insert_bank_rec(PGresult *res, PGconn *conn)
{
	int		bankID;
	char		b;
	char		buf[BUFSIZE * 8];
	char		str[4][BUFSIZE];
	const char	*evalues[1], *svalues[2];
	int		elength[1], slength[2];
	int		ebinary[1] = {1}, sbinary[2] = {0, 0};
	uint32_t	queryID;

	printf("Insert new bank? (y/n) ");
	scanf(" %c", &b);
	getchar();

	printf("Enter bank id: ");
	scanf(" %d", &bankID);
	getchar();

	queryID = htonl((uint32_t) bankID);
	evalues[0] = (char *) &queryID;
	elength[0] = sizeof(queryID);
	
	res = PQexecParams(conn, "SELECT * FROM bank "
				"WHERE bankid = $1::int4", 1, NULL, evalues,
				elength, ebinary, 0);
	select_error(res, conn);
	if(b == 'y')
	{


		if(PQntuples(res) > 0)
		{
			printf("\nERROR. BANK ID IS TAKEN. ABORTING INSERT...\n");
			return 0;
		}

		snprintf(buf, BUFSIZE * 8, "INSERT INTO bank VALUES (%d)", bankID);

		res = PQexecParams(conn, buf, 0, NULL, NULL, NULL, NULL, 0);
		insert_error(res, conn);
		printf("Status: %s\n", PQcmdStatus(res));
		PQclear(res);

		res = PQexecParams(conn, "SELECT * FROM bank", 0, NULL, NULL, NULL, NULL, 0);
		select_error(res, conn);
		display_insertion_menu(res);

		return 0;
	}

	if(PQntuples(res) == 0)
	{
		printf("\nERROR. BANK NOT FOUND. ABORTING INSERT...\n");
		return 0;
	}

	printf("Enter address: ");
	scanf("%32[^\n]", (char *) str[0]);
	getchar();
	svalues[0] = str[0];
	slength[0] = strlen(str[0]);

	printf("Enter city: ");
	scanf("%32[^\n]", (char *) str[1]);
	getchar();
	svalues[1] = str[1];
	slength[1] = strlen(str[1]);
	
	printf("Enter state: ");
	scanf("%32[^\n]", (char *) str[2]);
	getchar();
	
	printf("Enter zip code: ");
	scanf("%32[^\n]", (char *) str[3]);
	getchar();	
	
	res = PQexecParams(conn, "SELECT * FROM bankbranch "
				"WHERE addr = $1::varchar(50) "
				"and city = $2::varchar(30)", 2, NULL, svalues,
				slength, sbinary, 0);
	select_error(res, conn);

	if(PQntuples(res) > 0)
	{
		printf("\nERROR. ADDRESS ALREADY USED. ABORTING INSERT...\n");
		return 0;
	}

	snprintf(buf, BUFSIZE * 8, "INSERT INTO bankbranch VALUES ('%s', '%s', '%s', '%s', %d)", 
			str[0], str[1], str[2], str[3], bankID);

	res = PQexecParams(conn, buf, 0, NULL, NULL, NULL, NULL, 0);
	insert_error(res, conn);
	printf("Status: %s\n", PQcmdStatus(res));
	PQclear(res);

	res = PQexecParams(conn, "SELECT * FROM bankbranch", 0, NULL, NULL, NULL, NULL, 0);
	select_error(res, conn);
	display_insertion_menu(res);

	return 0;
}

int
insert_acc_rec(PGresult *res, PGconn *conn)
{
	float		balance, irate;
	int		accID, custID;
	char		buf[BUFSIZE * 8];
	const char	*evalues[1];
	int		elength[1];
	int		ebinary[1] = {1};
	uint32_t	queryID;

	printf("Enter customer id: ");
	scanf(" %d", &custID);
	getchar();

	printf("Enter account ID: ");
	scanf(" %d", &accID);
	getchar();

	printf("Enter intial balance: ");
	scanf(" %f", &balance);
	getchar();
	
	printf("Enter irate: ");
	scanf(" %f", &irate);
	getchar();

	queryID = htonl((uint32_t) accID);
	evalues[0] = (char *) &queryID;
	elength[0] = sizeof(queryID);
	
	res = PQexecParams(conn, "SELECT * FROM account "
				"WHERE accid = $1::int4", 1, NULL, evalues,
				elength, ebinary, 0);
	select_error(res, conn);

	if(PQntuples(res) > 0)
	{
		printf("\nERROR. ACCOUNT ID IS TAKEN. ABORTING INSERT...\n");
		return 0;
	}
	
	queryID = htonl((uint32_t) custID);
	evalues[0] = (char *) &queryID;
	elength[0] = sizeof(queryID);
	
	res = PQexecParams(conn, "SELECT * FROM customer "
				"WHERE cusid = $1::int4", 1, NULL, evalues,
				elength, ebinary, 0);
	select_error(res, conn);

	if(PQntuples(res) == 0)
	{
		printf("\nERROR. CUSTOMER ID NOT FOUND. ABORTING INSERT...\n");
		return 0;
	}

	snprintf(buf, BUFSIZE * 8, "INSERT INTO account VALUES (%f, %f, %d, %d)", balance, irate, accID, custID);

	res = PQexecParams(conn, buf, 0, NULL, NULL, NULL, NULL, 0);
	insert_error(res, conn);
	printf("Status: %s\n", PQcmdStatus(res));
	PQclear(res);

	res = PQexecParams(conn, "SELECT * FROM account", 0, NULL, NULL, NULL, NULL, 0);
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
delete_trans_rec(PGresult *res, PGconn *conn)
{
	uint32_t	keyid, accid;
	const char	*dvalues[1], *uvalues[2];
	int		dlength[1], ulength[2];
	int		dbinary[1] = {1}, ubinary[2] = {0, 1};
	bool		deleted = false;
	float		balance, amount;

	uvalues[0] = malloc(BUFSIZE);

	printf("Enter transaction ID to delete: ");
	scanf(" %d", &keyid);
	getchar();
	keyid = htonl((uint32_t) keyid);
	dvalues[0] = (char *) &keyid;
	dlength[0] = sizeof(keyid);

	res = PQexecParams(conn, "SELECT amount, accid FROM transaction "
				"WHERE transid = $1::int4", 1, NULL, dvalues,
				dlength, dbinary, 0);
	select_error(res, conn);

	if(PQntuples(res) == 1)
	{
		amount = atof(PQgetvalue(res, 0, 0));
		accid = htonl((uint32_t) atoi(PQgetvalue(res, 0, 1)));
		dvalues[0] = (char *) &accid;
		dlength[0] = sizeof(accid);
		uvalues[1] = (char *) &accid;
		ulength[1] = sizeof(accid);
	}

	res = PQexecParams(conn, "SELECT balance FROM account "
				"WHERE accid = $1::int4", 1, NULL, dvalues,
				dlength, dbinary, 0);
	select_error(res, conn);

	if(PQntuples(res) == 1)
	{
		balance = atof(PQgetvalue(res, 0, 0));
		balance -= amount;
		
		snprintf((char *)uvalues[0], BUFSIZE, "%f", balance);
		ulength[0] = strlen(uvalues[0]);
	}
	else
		deleted = true;

	if(!deleted)
	{
		res = PQexecParams(conn, "UPDATE account"
				" SET balance = $1::numeric(12,2)"
				" WHERE accid = $2::int4", 2, NULL, uvalues,
				ulength, ubinary, 0);
		delete_error(res, conn);
	}

	dvalues[0] = (char *) &keyid;
	dlength[0] = sizeof(keyid);
	dbinary[0] = 1;
	res = PQexecParams(conn, "DELETE FROM transaction WHERE transid = $1::int4", 1, NULL, dvalues, dlength, dbinary, 1);
	printf("Status: %s\n", PQcmdStatus(res));
	delete_error(res, conn);

	free((void *) uvalues[0]);

	return 0;
}

int
delete_cust_rec(PGresult *res, PGconn *conn)
{
	uint32_t	keyid;
	const char	*dvalues[1];
	int		dlength[1];
	int		dbinary[1];

	printf("Enter customer ID to delete: ");
	scanf(" %d", &keyid);
	getchar();
	keyid = htonl((uint32_t) keyid);
	dvalues[0] = (char *) &keyid;
	dlength[0] = sizeof(keyid);
	dbinary[0] = 1;
	
	
	res = PQexecParams(conn, "SELECT accid FROM account WHERE cusid = $1::int4",
				1, NULL, dvalues, dlength, dbinary, 0);
	select_error(res, conn);
			
	/*I didn't want to delete from transaction because of how messy it is but
	it wouldn't let me delete accounts without deleting transactions*/
	int tuple = PQntuples(res);	
	for(int i = 0; i < tuple; i++)
	{
		uint32_t temp = htonl((uint32_t) atoi(PQgetvalue(res, i, 0)));
		dvalues[0] = (char *) &temp;
		dlength[0] = sizeof(temp);
		
		res = PQexecParams(conn, "DELETE FROM transaction WHERE accid = $1::int4", 1, NULL, dvalues, dlength, dbinary, 1);
		delete_error(res, conn);
		
		dvalues[0] = (char *) &keyid;
		dlength[0] = sizeof(keyid);
		
		res = PQexecParams(conn, "SELECT accid FROM account WHERE cusid = $1::int4",
					1, NULL, dvalues, dlength, dbinary, 0);
		select_error(res, conn);
	}
	
	dvalues[0] = (char *) &keyid;
	dlength[0] = sizeof(keyid);
	
	res = PQexecParams(conn, "DELETE FROM account WHERE cusid = $1::int4", 1, NULL, dvalues, dlength, dbinary, 1);
	delete_error(res, conn);
	
	res = PQexecParams(conn, "DELETE FROM customer WHERE cusid = $1::int4", 1, NULL, dvalues, dlength, dbinary, 1);
	printf("Status: %s\n", PQcmdStatus(res));
	delete_error(res, conn);
	
	return 0;
}

int
delete_bank_rec(PGresult *res, PGconn *conn)
{
	const char	*dvalues[2];
	int		dlength[2];
	int		dbinary[2] = {0, 0};
	
	dvalues[0] = malloc(BUFSIZE);
	dvalues[1] = malloc(BUFSIZE);

	printf("Enter bank address to delete: ");
	scanf("%32[^\n]", (char *) dvalues[0]);
	getchar();
	dlength[0] = strlen(dvalues[0]);
	
	printf("Enter bank city to delete: ");
	scanf("%32[^\n]", (char *) dvalues[1]);
	getchar();
	dlength[1] = strlen(dvalues[1]);


	res = PQexecParams(conn, "DELETE FROM bankbranch WHERE addr = $1::varchar(50) "
				"and city = $2::varchar(30)", 2, NULL, dvalues, dlength, dbinary, 1);
	printf("Status: %s\n", PQcmdStatus(res));
	delete_error(res, conn);
	
	free((void *) dvalues[0]);
	free((void *) dvalues[1]);
	
	return 0;
}

int
delete_acc_rec(PGresult *res, PGconn *conn)
{
	uint32_t	keyid;
	const char	*dvalues[1];
	int		dlength[1];
	int		dbinary[1];

	printf("Enter account ID to delete: ");
	scanf(" %d", &keyid);
	getchar();
	keyid = htonl((uint32_t) keyid);
	dvalues[0] = (char *) &keyid;
	dlength[0] = sizeof(keyid);
	dbinary[0] = 1;
	res = PQexecParams(conn, "DELETE FROM account WHERE accid = $1::int4", 1, NULL, dvalues, dlength, dbinary, 1);
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
		printf("\n\t-d Delete account\n\t-e Edit account information\n\t"
			"-i Insert new account\n\t-v View bank accounts\n\t"
			"-q Quit to main menu\n\t-x Exit program\n");
		c = getchar();
		getchar();
		if (c == 'v')
			viewid_rec(res, conn, 'a');
		else if (c == 'e')
			edit_acc_rec(res, conn);
		else if (c =='i')
			insert_acc_rec(res, conn);
		else if (c == 'd')
			delete_acc_rec(res, conn);
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
		printf("\n\t-d Delete customer\n\t-e Edit customer information\n\t"
			"-i Insert new customer\n\t-v View customer accounts\n\t"
			"-q Quit to main menu\n\t-x Exit program\n");
		c = getchar();
		getchar();
		if (c == 'v')
			viewid_rec(res, conn, 'c');
		else if (c == 'e')
			edit_cust_rec(res, conn);
		else if (c == 'i')
			insert_cust_rec(res, conn);
		else if (c == 'd')
			delete_cust_rec(res, conn);
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
		printf("\n\t-d Delete bank branch\n\t-e Edit bank branches information\n\t"
			"-i Insert new bank or bank branch\n\t-v View bank branches\n\t"
			"-q Quit to main menu\n\t-x Exit program\n");
		c = getchar();
		getchar();
		if (c == 'v')
			viewid_rec(res, conn, 'b');
		else if (c == 'e')
			edit_bank_rec(res, conn);
		else if (c == 'i')
			insert_bank_rec(res, conn);
		else if (c == 'd')
			delete_bank_rec(res, conn);
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
			delete_trans_rec(res, conn);
		else if (c == 'e')
			edit_trans_rec(res, conn);
		else if (c == 'i')
			insert_trans_rec(res, conn);
		else if (c == 'v')
			view_rec(res, conn, 't');
		else if (c == 'x')
			exit_success(conn);
	} while (c != EOF && c != '\n' && c != 'q');

	return 0;
}

int
cust_view_acc(PGresult *res, PGconn *conn, uint32_t id)
{
	const char	*cvalues[1];
	int		clength[1];
	int		cbinary[1] = {1};
	
	cvalues[0] = (char *) &id;
	clength[0] = sizeof(id);
	
	res = PQexecParams(conn, "SELECT balance, irate FROM account WHERE "
		       "cusid = $1::int4", 1, NULL, cvalues, clength, cbinary, 0);
	select_error(res, conn);
	printf("Status: %s\n", PQcmdStatus(res));
	display_results(res);
	
	return 0;
}

int
cust_view_bank(PGresult *res, PGconn *conn, uint32_t id)
{
	bool 		city = false;
	char		ans;
	const char	*cvalues[1], *svalues[2], *csvalues[3];
	int		clength[1], slength[2], cslength[3];
	int		cbinary[1] = {1}, sbinary[2] = {1, 0}, csbinary[3] = {1, 0, 0};
	uint32_t 	bankid;
	
	cvalues[0] = (char *) &id;
	clength[0] = sizeof(id);
	
	res = PQexecParams(conn, "SELECT bankid FROM customer WHERE "
		       "cusid = $1::int4", 1, NULL, cvalues, clength, cbinary, 0);
	select_error(res, conn);
	
	bankid = htonl((uint32_t) atoi(PQgetvalue(res, 0, 0)));
	
	printf("Filter by City? (y/n) ");
	ans = getchar();
	getchar();
	
	if(ans != 'y')
	{
		printf("Filter by State? (y/n) ");
		ans = getchar();
		getchar();
	}
	else
	{
		csvalues[1] = malloc(BUFSIZE);
		csvalues[2] = malloc(BUFSIZE);
		
		city = true;
		
		printf("Enter city: ");
		scanf("%32[^\n]", (char *) csvalues[2]);
		getchar();
		cslength[2] = strlen(csvalues[2]);
		
		printf("Enter state: ");
		scanf("%32[^\n]", (char *) csvalues[1]);
		getchar();
		cslength[1] = strlen(csvalues[1]);
	}

	if(city)
	{
		csvalues[0] = (char *) &bankid;
		cslength[0] = sizeof(bankid);
	
		res = PQexecParams(conn, "SELECT addr, city, state, zip FROM bankbranch WHERE "
		       "bankid = $1::int4 and state = $2::varchar(2) and city = $3::varchar(30)",
		        3, NULL, csvalues, cslength, csbinary, 0);
			select_error(res, conn);
			
		free((void *) csvalues[1]);
		free((void *) csvalues[2]);
	}
	else if(ans == 'y')
	{
		svalues[1] = malloc(BUFSIZE);
		
		printf("Enter state: ");
		scanf("%32[^\n]", (char *) svalues[1]);
		getchar();
		slength[1] = strlen(svalues[1]);
		
		svalues[0] = (char *) &bankid;
		slength[0] = sizeof(bankid);
	
		res = PQexecParams(conn, "SELECT addr, city, state, zip FROM bankbranch WHERE "
		       "bankid = $1::int4 and state = $2::varchar(2)", 2, NULL, svalues, slength, sbinary, 0);
			select_error(res, conn);
			
		free((void *) svalues[1]);
	}
	else
	{
		cvalues[0] = (char *) &bankid;
		clength[0] = sizeof(bankid);
	
		res = PQexecParams(conn, "SELECT addr, city, state, zip FROM bankbranch WHERE "
		       "bankid = $1::int4", 1, NULL, cvalues, clength, cbinary, 0);
			select_error(res, conn);
	}
	
	
	printf("Status: %s\n", PQcmdStatus(res));
	display_results(res);
	
	return 0;
}

int
cust_view_trans(PGresult *res, PGconn *conn, uint32_t id)
{
	const char	*cvalues[1];
	int		clength[1];
	int		cbinary[1] = {1};
	uint32_t	accid;
	
	cvalues[0] = (char *) &id;
	clength[0] = sizeof(id);
	
	res = PQexecParams(conn, "SELECT accid FROM account WHERE "
		       "cusid = $1::int4", 1, NULL, cvalues, clength, cbinary, 0);
	select_error(res, conn);
	
	int tuple = PQntuples(res);
	for(int i = 0; i < tuple; i++)
	{
		accid = htonl((uint32_t) atoi(PQgetvalue(res, i, 0)));
		cvalues[0] = (char *) &accid;
		clength[0] = sizeof(accid);
	
		res = PQexecParams(conn, "SELECT merchant, description, date, amount FROM transaction WHERE "
			       "accid = $1::int4", 1, NULL, cvalues, clength, cbinary, 0);
		select_error(res, conn);
		printf("\nStatus: %s\n", PQcmdStatus(res));
		display_results(res);
		
		cvalues[0] = (char *) &id;
		clength[0] = sizeof(id);
	
		res = PQexecParams(conn, "SELECT accid FROM account WHERE "
			       "cusid = $1::int4", 1, NULL, cvalues, clength, cbinary, 0);
		select_error(res, conn);
	}
	
	return 0;
}

int
cust_view_info(PGresult *res, PGconn *conn, uint32_t id)
{
	const char	*cvalues[1];
	int		clength[1];
	int		cbinary[1] = {1};
	
	cvalues[0] = (char *) &id;
	clength[0] = sizeof(id);
	
	res = PQexecParams(conn, "SELECT usern, fullname FROM customer WHERE "
		       "cusid = $1::int4", 1, NULL, cvalues, clength, cbinary, 0);
	select_error(res, conn);
	printf("Status: %s\n", PQcmdStatus(res));
	display_results(res);
	
	return 0;
}

int
cust_edit_info(PGresult *res, PGconn *conn, uint32_t id)
{
	const char	*cvalues[2], *nvalues[3];
	int		clength[2], nlength[3];
	int		cbinary[2] = {1, 0}, nbinary[3] = {0, 0, 1};
	
	cvalues[1] = malloc(BUFSIZE);
	
	printf("Confirm your password: ");
	scanf("%32[^\n]", (char *) cvalues[1]);
	getchar();
	cvalues[1] = generateHash((char *) cvalues[1]);
	clength[1] = strlen(cvalues[1]);
	
	printf("%s\n", cvalues[1]);
	
	cvalues[0] = (char *) &id;
	clength[0] = sizeof(id);
	
	res = PQexecParams(conn, "SELECT * FROM customer WHERE "
		       "cusid = $1::int4 and phash = $2::varchar", 2, NULL, cvalues, clength, cbinary, 0);
	select_error(res, conn);
	
	if(PQntuples(res) == 0)
	{
		printf("\nERROR: INCORRECT PASSWORD. ABORTING...\n");
		free((void *) cvalues[1]);
		return 0;
	}
	
	nvalues[0] = malloc(BUFSIZE);
	nvalues[1] = malloc(BUFSIZE);
	
	printf("Enter new full name: ");
	scanf("%32[^\n]", (char *) nvalues[0]);
	getchar();
	nlength[0] = strlen(nvalues[0]);
	
	printf("Enter new password: ");
	scanf("%32[^\n]", (char *) nvalues[1]);
	getchar();
	nvalues[1] = generateHash((char *) nvalues[1]);
	nlength[1] = strlen(nvalues[1]);
	
	nvalues[2] = cvalues[0];
	nlength[2] = clength[0];
	
	res = PQexecParams(conn, "UPDATE customer"
			" SET fullname = $1::varchar(32),"
			" phash = $2::varchar(60)"
			" WHERE cusid = $3::int4", 3, NULL, nvalues, nlength, nbinary, 0);
	delete_error(res, conn);

	printf("Status: %s\n", PQcmdStatus(res));

	free((void *) cvalues[1]);		
	free((void *) nvalues[0]);
	free((void *) nvalues[1]);
	
	return 0;
}

#!//bin/bash

dropdb bank
createdb bank
psql bank < bankdb_dump

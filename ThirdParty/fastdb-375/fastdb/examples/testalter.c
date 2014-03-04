/*-< TESTALTER.C >---------------------------------------------------*--------*
 * FastDB                    Version 1.0         (1999  GARRET    *     ?  *
 * (Main Memory Database Management System                          *   /\|  *
 *                                                                   *  /  \  *
 *                          Created:     29-Nov-2003 K.A. Knizhnik   * / [] \ *
 *                          Last update: 29-Nov-2003 K.A. Knizhnik   * GARRET *
 *-------------------------------------------------------------------*--------*
 * Test for cli_alter_table
 *-------------------------------------------------------------------*--------*/

#include "cli.h"
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N_RECORDS 100000

typedef struct foo {
    cli_int4_t  x;
    char*       y;
    cli_real8_t z;
} foo;

typedef struct alter_foo {
    cli_int8_t  x;
    char*       y;
    char*       a;
    cli_real4_t z;
    cli_int1_t  b;
} alter_foo;


static cli_field_descriptor foo_descriptor[] = {
    {cli_int4, cli_indexed, "x"},
    {cli_pasciiz, 0, "y"}, 
    {cli_real8, 0, "z"} 
}; 

static cli_field_descriptor alter_foo_descriptor[] = {
    {cli_int8, cli_indexed, "x"},
    {cli_pasciiz, 0, "y"}, 
    {cli_pasciiz, 0, "a"}, 
    {cli_real4, 0, "z"},
    {cli_int1, 0, "b"}
}; 


int main()
{
    char_t* databaseName = _T("testalter");
    char_t* filePath = _T("testalter.fdb");
    int i, rc;
    int session;
    int statement;
    cli_oid_t oid;
    foo f;
    alter_foo af;
    time_t start = time(NULL);

    session = cli_create(databaseName,  filePath, 0, 0, 4*1024*1024, 4*1024*1024, 512*1024, 0);
    if (session < 0) {
	fprintf(stderr, "cli_open failed with code %d\n", session);
	return EXIT_FAILURE;
    }

    rc = cli_create_table(session, 
                          "foo", 
                          sizeof(foo_descriptor)/sizeof(cli_field_descriptor), 
			  foo_descriptor);
    if (rc != cli_ok) { 
	fprintf(stderr, "cli_create_table failed with code %d\n", rc);
	return EXIT_FAILURE;
    } 
    for (i = 0; i < N_RECORDS; i++) { 
        char buf[32];
        sprintf(buf, "%d", i);
        f.x = i;
        f.y = buf;
        f.z = i + (double)i / 10;        
        rc = cli_insert_struct(session, "foo", &f, &oid);
        if (rc != cli_ok) { 
            fprintf(stderr, "cli_insert failed with code %d\n", rc);
            return EXIT_FAILURE;
        }
    }
    printf("Elapsed time for inserting %d records: %d seconds\n", N_RECORDS, (int)(time(NULL) - start));
    start = time(NULL);
    rc = cli_alter_table(session, 
                         "foo", 
                         sizeof(alter_foo_descriptor)/sizeof(cli_field_descriptor), 
                         alter_foo_descriptor);
    if (rc != cli_ok) { 
	fprintf(stderr, "cli_alter_table failed with code %d\n", rc);
	return EXIT_FAILURE;
    } 
    printf("Elapsed time for reformating %d records: %d seconds\n", N_RECORDS, (int)(time(NULL) - start));
    start = time(NULL);    
    statement = cli_prepare_query(session, "select * from foo where x=%i");
    if (statement < 0) { 
        fprintf(stderr, "cli_statement failed with code %d\n", rc);
        return EXIT_FAILURE;
    }
    for (i = 0; i < N_RECORDS; i++) { 
        char buf[32];
        sprintf(buf, "%d", i);
        rc = cli_execute_query(statement, cli_view_only, &af, i);
        if (rc != 1) { 
            fprintf(stderr, "cli_execute_query failed with code %d\n", rc);
            return EXIT_FAILURE;
        }
        rc = cli_get_next(statement);
        if (rc != cli_ok) { 
            fprintf(stderr, "cli_get_next failed with code %d\n", rc);
            return EXIT_FAILURE;
        }
        if (af.x != i || strcmp(buf, af.y) != 0 || *af.a != '\0' || (int)((af.z+0.01)*10) != i*10+i || af.b != 0) { 
            fprintf(stderr, "Conversion error: i=%d, f.x=%ld, f.y=%s, f.a=%s, f.z=%f, f.b=%d\n", i, (long)af.x, af.y, af.a, af.z, af.b);
            return EXIT_FAILURE;
        }
    }
    printf("Elapsed time for %d index searches: %d seconds\n", N_RECORDS, (int)(time(NULL) - start));
    rc = cli_drop_table(session, "foo");
    if (rc != 0) { 
        fprintf(stderr, "cli_drop_table failed with code %d\n", rc);
        return EXIT_FAILURE;
    }
    rc = cli_close(session);
    if (rc != cli_ok) { 
        fprintf(stderr, "cli_close failed with code %d\n", rc);
        return EXIT_FAILURE;	
    }
    printf("*** CLI alter test sucessfully passed!\n");
    return EXIT_SUCCESS;
}
    

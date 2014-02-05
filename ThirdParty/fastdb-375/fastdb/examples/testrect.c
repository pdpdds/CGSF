/*-< TESTRECT.C >----------------------------------------------------*--------*
 * FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
 * (Main Memory Database Management System)                          *   /\|  *
 *                                                                   *  /  \  *
 *                          Created:     13-Jan-2000 K.A. Knizhnik   * / [] \ *
 *                          Last update: 13-Jan-2000 K.A. Knizhnik   * GARRET *
 *-------------------------------------------------------------------*--------*
 * Test rectangle type and spatial indices with FastDB call level interface 
 *-------------------------------------------------------------------*--------*/

#include "cli.h"
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct record {
    int             id;
    cli_rectangle_t r;
} record;

static cli_field_descriptor record_descriptor[] = {
    {cli_autoincrement, cli_hashed, "id"},
    {cli_rectangle, cli_indexed, "r"}
}; 


int main()
{
    char_t* databaseName = _T("testrect");
    char_t* filePath = _T("testrect.fdb");
    int session, statement, rc;
    int i, j, n;
    cli_oid_t oid;
    record rec;
    cli_rectangle_t r;

    session = cli_create(databaseName, filePath, 0, 0, 4*1024*1024, 4*1024*1024, 512*1024, 0);
    if (session < 0) { 
        fprintf(stderr, "cli_create failed with code %d\n", session);
        return EXIT_FAILURE;
    }

    rc = cli_create_table(session, "record", sizeof(record_descriptor)/sizeof(cli_field_descriptor), 
			  record_descriptor);
    if (rc != cli_ok && rc != cli_table_already_exists) { 
	fprintf(stderr, "cli_create_table failed with code %d\n", rc);
	return EXIT_FAILURE;
    } 

    statement = cli_prepare_query(session, "select * from record where r overlaps %R");
    if (statement < 0) { 
        fprintf(stderr, "cli_prepare_query failed with code %d\n", rc);
        return EXIT_FAILURE;
    }	

    while (1) { 
        int ch;
        printf("Commands: I - insert, S - search, Q - quit\n>");        
        while (isspace(ch = getchar()));
        switch (ch) { 
          case 'I':
          case 'i':
            printf("Input new rectangle coordinates:\n");
            for (i = 0; i < RECTANGLE_DIMENSION; i++) { 
                scanf("%d%d", &rec.r.boundary[i*2], &rec.r.boundary[i*2+1]);
            }
            rc = cli_insert_struct(session, "record", &rec, &oid);
            if (rc != cli_ok) { 
                fprintf(stderr, "cli_insert failed with code %d\n", rc);
                return EXIT_FAILURE;
            }
            if ((rc = cli_commit(session)) != cli_ok) { 
                fprintf(stderr, "cli_commit failed with code %d\n", rc);
                return EXIT_FAILURE;	
            }
            continue;
          case 's':
          case 'S':
            printf("Input rectangle coordinates:\n");
            for (i = 0; i < RECTANGLE_DIMENSION; i++) { 
                scanf("%d%d", &r.boundary[i*2], &r.boundary[i*2+1]);
            }
            rc = cli_execute_query(statement, cli_view_only, &rec, &r);
            if (rc < 0) { 
                fprintf(stderr, "cli_execute_query failed with code %d\n", rc);
                return EXIT_FAILURE;	
            }
            n = rc;
            for (i = 0; i < n; i++) { 
                rc = cli_get_next(statement);
                if (rc != cli_ok) { 
                    fprintf(stderr, "cli_get_next failed with code %d\n", rc);
                    return EXIT_FAILURE;	
                }
                printf("Record %d ", rec.id);
                for (j = 0; j < RECTANGLE_DIMENSION; j++) { 
                    printf("[%d,%d]", r.boundary[j*2], r.boundary[j*2+1]);
                }
                printf("\n");
            }
            continue;
          case 'q':
          case 'Q':
            if ((rc = cli_free(statement)) != cli_ok) { 
                fprintf(stderr, "cli_free 3 failed with code %d\n", rc);
                return EXIT_FAILURE;	
            }
            if ((rc = cli_close(session)) != cli_ok) { 
                fprintf(stderr, "cli_close failed with code %d\n", rc);
                return EXIT_FAILURE;	
            }
            printf("Terminated!\n");
            return EXIT_SUCCESS;
        }
    }
}

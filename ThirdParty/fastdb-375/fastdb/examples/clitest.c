/*-< CLITEST.C >-----------------------------------------------------*--------*
 * FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
 * (Main Memory Database Management System)                          *   /\|  *
 *                                                                   *  /  \  *
 *                          Created:     13-Jan-2000 K.A. Knizhnik   * / [] \ *
 *                          Last update: 13-Jan-2000 K.A. Knizhnik   * GARRET *
 *-------------------------------------------------------------------*--------*
 * Test for FastDB call level interface 
 * Spawn "subsql  clitest.sql" to start CLI server. 
 *-------------------------------------------------------------------*--------*/

#include "cli.h"
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct varbinary { 
    void* data;
    int   size;
} varbinary;

typedef struct  person { 
    char        name[64];
    cli_int8_t  salary;

    char*       address;
    cli_real8_t weight;
    cli_int4_t  n_subordinates;
    cli_oid_t*  subordinates;
    varbinary   blob;
} person;

void free_varbinary(varbinary* vb) 
{
     if (vb->data != NULL) { 
        free(vb->data);
        vb->data = NULL;
    }
}
   
void* CLI_CALLBACK_CC set_varbinary(int var_type, void* var_ptr, int len)
{
    varbinary* vb = (varbinary*)var_ptr;
    free_varbinary(vb);
    vb->size = len;
    return vb->data = malloc(len);
}

void* CLI_CALLBACK_CC get_varbinary(int var_type, void* var_ptr, int* len)
{
    varbinary* vb = (varbinary*)var_ptr;
    *len = vb->size;
    return vb->data;
}

void* CLI_CALLBACK_CC set_subordinates(int var_type, void* var_ptr, int len)
{
    person* p = (person*)var_ptr;
    if (p->subordinates != NULL) { 
        free(p->subordinates);
    }
    p->n_subordinates = len;
    p->subordinates = (cli_oid_t*)malloc(len*sizeof(cli_oid_t));
    return p->subordinates;
}

void* CLI_CALLBACK_CC get_subordinates(int var_type, void* var_ptr, int* len)
{
    person* p = (person*)var_ptr;
    *len = p->n_subordinates;
    return p->subordinates;
}

static cli_field_descriptor person_descriptor[] = {
    {cli_asciiz, cli_hashed, "name"},
    {cli_int8, cli_indexed, "salary"},
    {cli_pasciiz, 0, "address"}, 
    {cli_real8, 0, "weight"}, 
    {cli_array_of_oid, 0, "subordinates", "persons"},
    {cli_array_of_int1, 0, "blob"}
}; 


int main()
{
    char* serverURL = "localhost:6100";
    char_t* databaseName = _T("clitest");
    char_t* filePath = _T("clitest.fdb");
    int session, statement, statement2, rc, len;
    int i, n, salary;
    int table_created = 0;
    char name[256];
    char address[256];
    cli_oid_t oid;
    person p;

    session = cli_open(serverURL, 10, 1);
    if (session == cli_bad_address) { 
	session = cli_create(databaseName, filePath, 0, 0, 4*1024*1024, 4*1024*1024, 512*1024, 0);
    }
    if (session < 0) { 
        fprintf(stderr, "cli_open failed with code %d\n", session);
        return EXIT_FAILURE;
    }

    rc = cli_create_table(session, "persons", sizeof(person_descriptor)/sizeof(cli_field_descriptor), 
			  person_descriptor);
    if (rc == cli_ok) { 
	table_created = 1;
        rc = cli_alter_index(session, "persons", "salary", cli_indexed); 
        if (rc != cli_ok) { 
            fprintf(stderr, "cli_alter_index failed with code %d\n", rc);
            return EXIT_FAILURE;
        }
        rc = cli_alter_index(session, "persons", "name", cli_indexed); 
        if (rc != cli_ok) { 
            fprintf(stderr, "cli_alter_index 2 failed with code %d\n", rc);
            return EXIT_FAILURE;
        }
    } else if (rc != cli_table_already_exists && rc != cli_not_implemented) { 
	fprintf(stderr, "cli_create_table failed with code %d\n", rc);
	return EXIT_FAILURE;
    } 

    statement = cli_statement(session, "insert into persons");
    if (statement < 0) { 
        fprintf(stderr, "cli_statement failed with code %d\n", statement);
        return EXIT_FAILURE;
    }

    if ((rc=cli_column(statement, "name", cli_asciiz, NULL, p.name)) != cli_ok
        || (rc=cli_column(statement, "salary", cli_int8, NULL, &p.salary)) != cli_ok
        || (rc=cli_column(statement, "address", cli_pasciiz, &len, &p.address)) != cli_ok
        || (rc=cli_column(statement, "weight", cli_real8, NULL, &p.weight)) != cli_ok
        || (rc=cli_array_column(statement, "subordinates", cli_array_of_oid, &p, 
                                set_subordinates, get_subordinates)) != cli_ok        
        || (rc=cli_array_column(statement, "blob", cli_array_of_int1, &p.blob, set_varbinary, get_varbinary)) != cli_ok)
    {
        fprintf(stderr, "cli_column 1 failed with code %d\n", rc);
        return EXIT_FAILURE;
    }

    strcpy(p.name, "John Smith");
    p.salary = 75000;
    p.address = "1 Guildhall St., Cambridge CB2 3NH, UK";
    p.weight = 80.3;
    p.n_subordinates = 0;
    p.subordinates = NULL;

    p.blob.size = 100;
    p.blob.data = malloc(p.blob.size);
    memset(p.blob.data, 1, p.blob.size);

    rc = cli_insert(statement, &oid);
    free_varbinary(&p.blob);
    if (rc != cli_ok) { 
        fprintf(stderr, "cli_insert failed with code %d\n", rc);
        return EXIT_FAILURE;
    }

    strcpy(p.name, "Joe Cooker");
    p.salary = 100000;
    p.address = "Outlook drive, 15/3";
    p.weight = 80.3;
    p.n_subordinates = 1;
    p.subordinates = &oid;

    p.blob.size = 200;
    p.blob.data = malloc(p.blob.size);
    memset(p.blob.data, 2, p.blob.size);

    rc = cli_insert(statement, NULL);
    free_varbinary(&p.blob);
    if (rc != cli_ok) { 
        fprintf(stderr, "cli_insert 2 failed with code %d\n", rc);
        return EXIT_FAILURE;
    }

    rc = cli_free(statement);
    if (rc != cli_ok) { 
        fprintf(stderr, "cli_free failed with code %d\n", rc);
        return EXIT_FAILURE;
    }
    
    p.subordinates = NULL;
    statement = cli_statement(session, 
                              "select * from persons where "
                              "length(subordinates) < %subordinates and salary > %salary");
    if (statement < 0) { 
        fprintf(stderr, "cli_statement 2 failed with code %d\n", rc);
        return EXIT_FAILURE;
    }
    p.address = address;
    len = sizeof(address);
    if ((rc=cli_column(statement, "name", cli_asciiz, NULL, p.name)) != cli_ok
        || (rc=cli_column(statement, "salary", cli_int8, NULL, &p.salary)) != cli_ok
        || (rc=cli_column(statement, "address", cli_pasciiz, &len, &p.address)) != cli_ok
        || (rc=cli_column(statement, "weight", cli_real8, NULL, &p.weight)) != cli_ok
        || (rc=cli_array_column(statement, "subordinates", cli_array_of_oid, &p, 
                                set_subordinates, get_subordinates)) != cli_ok
        || (rc=cli_array_column(statement, "blob", cli_array_of_int1, &p.blob, set_varbinary, get_varbinary)) != cli_ok)
    {
        fprintf(stderr, "cli_column 2 failed with code %d\n", rc);
        return EXIT_FAILURE;
    }
    if ((rc = cli_parameter(statement, "%subordinates", cli_int4, &n)) != cli_ok 
        || (rc = cli_parameter(statement, "%salary", cli_int4, &salary)) != cli_ok)
    {
        fprintf(stderr, "cli_parameter failed with code %d\n", rc);
        return EXIT_FAILURE;
    }
    n = 2;
    salary = 90000;
    rc = cli_fetch(statement, cli_view_only);
    if (rc != 1) { 
        fprintf(stderr, "cli_fetch 1 returns %d instead of 1\n", rc);
        return EXIT_FAILURE;
    }
    n = 10;
    salary = 50000;
    rc = cli_fetch(statement, cli_for_update);
    if (rc != 2) { 
        fprintf(stderr, "cli_fetch 2 returns %d instead of 2\n", rc);
        return EXIT_FAILURE;
    }
    statement2 = cli_statement(session, "select * from persons where current = %oid");
    if (statement2 < 0) { 
        fprintf(stderr, "cli_statement 3 failed with code %d\n", rc);
        return EXIT_FAILURE;
    }
 
    if ((rc=cli_column(statement2, "name", cli_asciiz, NULL, name)) != cli_ok) { 
        fprintf(stderr, "cli_column 3 failed with code %d\n", rc);
        return EXIT_FAILURE;
    }
    if ((rc = cli_parameter(statement2, "%oid", cli_oid, &oid)) != cli_ok) { 
        fprintf(stderr, "cli_parameter 3 failed with code %d\n", rc);
        return EXIT_FAILURE;
    }	

    while ((rc = cli_get_next(statement)) == cli_ok) { 
        printf("%s\t%ld\t%f\t%s\t%d*%d\n", p.name, (long)p.salary, p.weight, p.address, *(char*)p.blob.data, p.blob.size);
        if (p.n_subordinates > 0) { 
            printf("Manages:\n");
            for (i = 0; i < p.n_subordinates; i++) { 
                oid = p.subordinates[i];
                rc = cli_fetch(statement2, cli_view_only); 
                if (rc != 1) { 
                    fprintf(stderr, "cli_fetch by oid failed with code %d\n", rc);
                    return EXIT_FAILURE;
                }	
                if ((rc = cli_get_first(statement2)) != cli_ok) { 
                    fprintf(stderr, "cli_get_first failed with code %d\n", rc);
                    return EXIT_FAILURE;
                }
                printf("\t%s\n", name);
            }
        }
        len = sizeof(address);
        p.salary = p.salary*90/100;
        rc = cli_update(statement);
        if (rc != cli_ok) { 
            fprintf(stderr, "cli_update failed with code %d\n", rc);
            return EXIT_FAILURE;
        }
    }
    if (rc != cli_not_found) { 
        fprintf(stderr, "cli_get_next failed with code %d\n", rc);
        return EXIT_FAILURE;
    }
    free_varbinary(&p.blob);
    if ((rc = cli_free(statement)) != cli_ok ||
        (rc = cli_free(statement2)) != cli_ok)
    {
        fprintf(stderr, "cli_free 2 failed with code %d\n", rc);
        return EXIT_FAILURE;	
    }
    if ((rc = cli_commit(session)) != cli_ok) { 
        fprintf(stderr, "cli_commit failed with code %d\n", rc);
        return EXIT_FAILURE;	
    }
    statement = cli_statement(session, "select * from persons order by salary");
    if (statement < 0) { 
        fprintf(stderr, "cli_statement 4 failed with code %d\n", rc);
        return EXIT_FAILURE;
    }	
    if ((rc=cli_column(statement, "salary", cli_int4, NULL, &salary)) != cli_ok) { 
        fprintf(stderr, "cli_column 4 failed with code %d\n", rc);
        return EXIT_FAILURE;
    }
    rc = cli_fetch(statement, cli_for_update);
    if (rc != 2) { 
        fprintf(stderr, "cli_fetch 4 failed with code %d\n", rc);
        return EXIT_FAILURE;
    }	
    printf("New salaries:\n");
    while ((rc = cli_get_prev(statement)) == cli_ok) { 
        printf("\t%d\n", salary);
    }
    if (rc != cli_not_found) { 
        fprintf(stderr, "cli_get_prev failed with code %d\n", rc);
        return EXIT_FAILURE;
    }
    if ((rc = cli_remove(statement)) != cli_ok) {
        fprintf(stderr, "cli_remove failed with code %d\n", rc);
        return EXIT_FAILURE;
    }
    if ((rc = cli_free(statement)) != cli_ok) { 
        fprintf(stderr, "cli_free 3 failed with code %d\n", rc);
        return EXIT_FAILURE;	
    }
    if (table_created) { 
	rc = cli_drop_table(session, "persons");
	if (rc != cli_ok) { 
	    fprintf(stderr, "cli_drop_table failed with code %d\n", rc);
	    return EXIT_FAILURE;
	}
    }    

    if ((rc = cli_close(session)) != cli_ok) { 
        fprintf(stderr, "cli_close failed with code %d\n", rc);
        return EXIT_FAILURE;	
    }
    printf("*** CLI test sucessfully passed!\n");
    return EXIT_SUCCESS;
}
	


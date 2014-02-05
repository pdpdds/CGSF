//-< TESTDB.CPP >----------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     27-Mar-2000   K.A. Knizhnik  * / [] \ *
//                          Last update: 27-Mar-2000   K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Sample of using indexed joins
//-------------------------------------------------------------------*--------*

#include "fastdb.h"
#include <stdio.h>

USE_FASTDB_NAMESPACE

struct Detail { 
    char const* name;
    double      weight;
    
    TYPE_DESCRIPTOR((KEY(name, INDEXED), FIELD(weight)));
};

struct Supplier { 
    char const* company;
    char const* address;

    TYPE_DESCRIPTOR((KEY(company, INDEXED), FIELD(address)));
};

struct Shipment { 
    dbReference<Detail>   detail;
    dbReference<Supplier> supplier;
    int4                  price;
    int4                  quantity;
    dbDateTime            delivery;

    TYPE_DESCRIPTOR((KEY(detail, HASHED), KEY(supplier, HASHED), 
                     FIELD(price), FIELD(quantity), FIELD(delivery)));
};

REGISTER(Detail);
REGISTER(Supplier);
REGISTER(Shipment);

char* input(char const* prompt, char* buf, size_t buf_size)
{
    char* p;
    do { 
        printf(prompt);
        *buf = '\0';
        fgets(buf, (int)buf_size, stdin);
        p = buf + strlen(buf);
    } while (p <= buf+1); 
    
    if (*(p-1) == '\n') {
        *--p = '\0';
    }
    return buf;
}

int inputInt(char const* prompt) { 
    char buf[64];
    int val;
    *buf = '\0';
    do { 
        input(prompt, buf, sizeof buf);
    } while (sscanf(buf, "%d", &val) != 1);
    return val;
}

double inputReal(char const* prompt) { 
    char buf[64];
    double val;
    *buf = '\0';
    do { 
        input(prompt, buf, sizeof buf);
    } while (sscanf(buf, "%lf", &val) != 1);
    return val;
}


dbDateTime inputDate(char const* prompt) { 
    char buf[64];
    int d, m, y;
    *buf = '\0';
    do { 
        input(prompt, buf, sizeof buf);
    } while (sscanf(buf, "%d-%d-%d\n", &d, &m, &y) != 3);
    return dbDateTime(y, m, d);
}

int main() 
{
    const int maxStrLen = 256;

    dbDatabase db;

    char buf[maxStrLen];
    char name[maxStrLen];
    char company[maxStrLen];
    char address[maxStrLen];

    dbQuery q1, q2, q3, q4;
    q1 = "company=", company; 
    q2 = "name=", name; 
    q3 = "detail.name like",name,"and supplier.company like",company,
        "and supplier.address like",address,"order by price";
    q4 = "supplier.company like",company,"and supplier.address like",address,
        "and detail.name like",name,"order by",dbDateTime::ascent("delivery");

    dbCursor<Detail>   details;
    dbCursor<Shipment> shipments;
    dbCursor<Supplier> suppliers;

    Supplier  supplier;
    Detail    detail;
    Shipment  shipment;

    if (db.open(_T("testjoin"))) {
        while (true) { 
            printf(
"\n\n    MENU:\n\
1. Add supplier\n\
2. Add detail\n\
3. New shipment\n\
4. Find suppliers of the detail\n\
5. Find details shipped by supplier\n\
6. Exit\n");
            switch (inputInt(">> ")) { 
              case 1:
                supplier.company = input("Suppiler company: ", company, sizeof company);
                supplier.address = input("Suppiler address: ", address, sizeof address);
                insert(supplier);
                printf("Supplier added\n");
                break;
              case 2:
                detail.name = input("Detail name: ", name, sizeof name);
                detail.weight = inputReal("Detail weight: ");
                insert(detail);
                printf("Detail added\n");
                break;
              case 3: 
                input("Company name: ", company, sizeof company);
                if (suppliers.select(q1) == 0) { 
                    printf("No such supplier\n");
                    break;
                }
                shipment.supplier = suppliers.currentId();

                input("Detail name: ", name, sizeof name);
                if (details.select(q2) == 0) { 
                    printf("No such detail\n");
                    break;
                }
                shipment.detail = details.currentId();
                shipment.price = inputInt("Price: ");
                shipment.quantity = inputInt("Quantity: ");
                shipment.delivery = inputDate("Delivery: ");
                insert(shipment);
                printf("Shipment added\n");
                break;          
              case 4:
                input("Detail like: ", name, sizeof name);
                input("Company like: ", company, sizeof company);
                input("Address like: ", address, sizeof address);
                if (shipments.select(q3)) { 
                    printf("Detail Company Address Price Quantity Delivery\n");
                    do { 
                        printf("%s\t%s\t%s\t%d\t%d\t%s\n",
                               details.at(shipments->detail)->name, 
                               suppliers.at(shipments->supplier)->company, 
                               suppliers.at(shipments->supplier)->address, 
                               shipments->price, shipments->quantity, 
                               shipments->delivery.asString(buf, sizeof buf, "%A %x"));
                    } while(shipments.next());
                    printf("--------------------------------------------\n");
                } else {  
                    printf("No items selected\n");
                }
                break;
              case 5:
                input("Company like: ", company, sizeof company);
                input("Address like: ", address, sizeof address);
                input("Detail like: ", name, sizeof name);
                if (shipments.select(q4)) { 
                    printf("Company Address Detail Price Quantity Delivery\n");
                    do { 
                        printf("%s\t%s\t%s\t%d\t%d\t%s\n",
                               suppliers.at(shipments->supplier)->company, 
                               suppliers.at(shipments->supplier)->address, 
                               details.at(shipments->detail)->name, 
                               shipments->price, shipments->quantity, 
                               shipments->delivery.asString(buf, sizeof buf, "%A %x"));
                    } while(shipments.next());
                    printf("--------------------------------------------\n");
                } else {  
                    printf("No items selected\n");
                }
                break;
              case 6:
                printf("Close database session\n");
                db.close();
                return EXIT_SUCCESS;
            }
            printf("Press any key to continue...\n");
            getchar();
            db.commit();
        }
    } else { 
        printf("failed to open database\n");
        return EXIT_FAILURE;
    }
}

//-< TESTDB.CPP >----------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     10-Dec-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 12-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Sample of database application: supplier/contract/details database
//-------------------------------------------------------------------*--------*

#include "fastdb.h"
#include <stdio.h>

USE_FASTDB_NAMESPACE

class Contract;

class Detail { 
  public:
    char const* name;
    char const* material;
    char const* color;
    real4       weight;

    dbArray< dbReference<Contract> > contracts;

    TYPE_DESCRIPTOR((KEY(name, INDEXED|HASHED), 
                     KEY(material, HASHED), 
                     KEY(color, HASHED),
                     KEY(weight, INDEXED),
                     RELATION(contracts, detail)));
};

class Supplier { 
  public:
    char const* company;
    char const* location;
    bool        foreign;

    dbArray< dbReference<Contract> > contracts;

    TYPE_DESCRIPTOR((KEY(company, INDEXED|HASHED), 
                     KEY(location, HASHED), 
                     FIELD(foreign),
                     RELATION(contracts, supplier)));
};


class Contract { 
  public:
    dbDateTime            delivery;
    int4                  quantity;
    db_int8               price;
    dbReference<Detail>   detail;
    dbReference<Supplier> supplier;

    TYPE_DESCRIPTOR((KEY(delivery, HASHED|INDEXED), 
                     KEY(quantity, INDEXED), 
                     KEY(price, INDEXED),
                     RELATION(detail, contracts),
                     RELATION(supplier, contracts)));
};


REGISTER(Detail);
REGISTER(Supplier);
REGISTER(Contract);

void input(char const* prompt, char* buf, size_t buf_size)
{
    char* p;
    do { 
        printf(prompt);
        *buf = '\0';
        fgets(buf,(int) buf_size, stdin);
        p = buf + strlen(buf);
    } while (p <= buf+1); 
    
    if (*(p-1) == '\n') {
        *--p = '\0';
    }
}

int main() 
{
    const int maxStrLen = 256;

    dbDatabase db;

    char buf[maxStrLen];
    char name[maxStrLen];
    char company[maxStrLen];
    char material[maxStrLen];
    char address[maxStrLen];

    int d, m, y;
    int i, n;
    int choice;
    int quantity;
    db_int8 price;

    dbDateTime from, till;

    Contract contract;
    Supplier supplier;
    Detail detail;

    dbQuery q1, q2, q3, q4, q6, q9, q10;
    q1 = "exists i:(contracts[i].supplier.company=",company,")";
    q2 = "name like",name;
    q3 = "supplier.location =",address;
    q4 = between("delivery", from, till),"and price >",price,
        "order by",dbDateTime::ascent("delivery");
    q6 = "price >=",price,"or quantity >=",quantity;
    q9 = "company =",company;
    q10 = "supplier.company =",company,"and detail.name like",name; 

    dbCursor<Detail>   details;
    dbCursor<Contract> contracts;
    dbCursor<Supplier> suppliers;
    dbCursor<Contract> updateContracts(dbCursorForUpdate);
        
    if (db.open(_T("testdb"))) {
        while (true) { 
            printf(
"\n\n    MENU:\n\
1.  Details shipped by supplier\n\
2.  Suppliers of the detail\n\
3.  Contracts from specified city\n\
4.  Expensive details to be delivered in specified period\n\
5.  Foreign suppliers\n\
6.  Important contracts\n\
7.  New supplier\n\
8.  New detail\n\
9.  New contract\n\
10. Cancel contract\n\
11. Update contract\n\
12. Exit\n\n");
            input(">> ", buf, sizeof buf);
            if (sscanf(buf, "%d", &choice) != 1) { 
                continue;
            }
            switch (choice) { 
              case 1:
                printf("Details shipped by supplier\n");
                input("Supplier company: ", company, sizeof company);
                if (details.select(q1) > 0) { 
                    printf("Detail\tMaterial\tColor\tWeight\n");
                    do { 
                        printf("%s\t%s\t%s\t%f\n", 
                               details->name, details->material, 
                               details->color, details->weight);
                    } while (details.next());
                } else { 
                    printf("No details shipped by this supplier\n");
                }
                break;
              case 2:
                printf("Suppliers of the detail\n");
                input("Regular expression for detail name: ",name,sizeof name);
                if (details.select(q2) > 0) { 
                    printf("Detail\tCompany\tLocation\tPrice\n");
                    do { 
                        n = (int)details->contracts.length();
                        for (i = 0; i < n; i++) { 
                            contracts.at(details->contracts[i]);
                            suppliers.at(contracts->supplier);
                            printf("%s\t%s\t%s\t" INT8_FORMAT "\n", 
                                   details->name, 
                                   suppliers->company, suppliers->location,
                                   contracts->price);
                        }
                    } while (details.next());
                } else { 
                    printf("No such details\n");
                }
                break;
              case 3:
                printf("Contracts from specified city\n");
                input("City: ", address, sizeof address);
                if (contracts.select(q3) > 0) { 
                    printf("Detail\tCompany\tQuantity\n");
                    do { 
                        printf("%s\t%s\t%d\n", 
                               details.at(contracts->detail)->name, 
                               suppliers.at(contracts->supplier)->company, 
                               contracts->quantity);
                    } while (contracts.next());
                } else { 
                    printf("No contracts with companies in this city");
                }
                break;
              case 4:
                printf("Expensive details to be delivered in specified period\n");
                input("Delivered after (DD-MM-YYYY): ", buf, sizeof buf);
                if (sscanf(buf, "%d-%d-%d\n", &d, &m, &y) != 3) { 
                    printf("Wrong date\n");
                    continue;
                }
                from = dbDateTime(y, m, d);
                input("Delivered before (DD-MM-YYYY): ", buf, sizeof buf);
                if (sscanf(buf, "%d-%d-%d\n", &d, &m, &y) != 3) { 
                    printf("Wrong date\n");
                    continue;
                }
                till = dbDateTime(y, m, d);
                input("Minimal contract price: ", buf, sizeof buf);
                if (sscanf(buf, INT8_FORMAT, &price) != 1) { 
                    printf("Bad value\n");
                    continue;
                }
                if (contracts.select(q4) > 0) { 
                    printf("Detail\tDate\tPrice\n");
                    do { 
                        printf("%s\t%s\t" INT8_FORMAT "\n", 
                               details.at(contracts->detail)->name, 
                               contracts->delivery.asString(buf, sizeof buf),
                               contracts->price);
                    } while (contracts.next());
                } else { 
                    printf("No such contracts\n");
                }
                break;
              case 5:
                printf("Foreign suppliers\n");
                if (suppliers.select("foreign and length(contracts) > 0") > 0){
                    printf("Company\tLocation\n");
                    do { 
                        printf("%s\t%s\n", suppliers->company, 
                               suppliers->location);
                    } while (suppliers.next());
                } else { 
                    printf("No such suppliers\n");
                }
                break;
              case 6:
                printf("Important contracts\n");
                input("Minimal contract price: ", buf, sizeof buf);
                if (sscanf(buf, INT8_FORMAT, &price) != 1) { 
                    printf("Bad value\n");
                    continue;
                }
                input("Minimal contract quantity: ", buf, sizeof buf);
                if (sscanf(buf, "%d", &quantity) != 1) { 
                    printf("Bad value\n");
                    continue;
                }
                if (contracts.select(q6) > 0) { 
                    printf("Company\tPrice\tQuantity\tDelivery\n");
                    do { 
                        printf("%s\t" INT8_FORMAT "\t%d\t%s\n", 
                               suppliers.at(contracts->supplier)->company,
                               contracts->price, contracts->quantity,
                               contracts->delivery.asString(buf, sizeof buf, 
                                                            "%A %x"));
                    } while (contracts.next());
                } else { 
                    printf("No such contracts\n");
                }
                break;
              case 7:
                printf("New supplier\n");
                input("Company name: ", company, sizeof company);
                input("Company location: ", address, sizeof address);
                input("Foreign company (y/n): ", buf, sizeof buf);
                supplier.company = company;
                supplier.location = address;
                supplier.foreign = (*buf == 'y');
                insert(supplier);
                break;
              case 8:
                printf("New detail\n");
                input("Detail name: ", name, sizeof name);
                input("Detail material: ", material, sizeof material);
                input("Detail weight: ", buf, sizeof buf);
                sscanf(buf, "%f", &detail.weight);
                input("Detail color: ", buf, sizeof buf);
                detail.name = name;
                detail.material = material;
                detail.color = buf;
                insert(detail);
                break;
              case 9:
                printf("New contract\n");
                db.lock(); // prevent deadlock
                input("Supplier company: ", company, sizeof company);
                if (suppliers.select(q9) == 0) { 
                    printf("No such supplier\n");
                    continue;
                }
                input("Detail name: ", name, sizeof name);
                if (details.select(q2) == 0) { 
                    printf("No such detail\n");
                    continue;
                } else if (details.getNumberOfRecords() != 1) { 
                    printf("More than one record match this pattern");
                    continue;
                }
                input("Contract price: ", buf, sizeof buf);
                sscanf(buf, INT8_FORMAT, &contract.price);
                input("Contract quantity: ", buf, sizeof buf);
                sscanf(buf, "%d", &contract.quantity);
                input("Delivered after (days): ", buf, sizeof buf);
                sscanf(buf, "%d", &d);
                contract.delivery = 
                    dbDateTime::currentDate() + dbDateTime(24*d,0);
                contract.supplier = suppliers.currentId();
                contract.detail = details.currentId();
                insert(contract);
                break;
              case 10: 
                printf("Cancel contract\n");
                input("Supplier company: ", company, sizeof company);
                input("Detail name pattern: ", name, sizeof name);
                if (updateContracts.select(q10) == 0) { 
                    printf("No such contracts\n");
                } else { 
                    updateContracts.removeAllSelected();
                    // Just test rollback
                    input("Really cancel contract (y/n) ? ", buf, sizeof buf);
                    if (*buf != 'y') { 
                        printf("Not confirmed\n");
                        db.rollback();
                        continue;
                    }
                }
                break;
              case 11:
                printf("Update contract\n");
                input("Supplier company: ", company, sizeof company);
                input("Detail name pattern: ", name, sizeof name);
                if (updateContracts.select(q10) == 0) { 
                    printf("No such contracts\n");
                    break;
                }               
                do { 
                    printf("Contract with company %s for shipping %d details "
                           "%s for $" INT8_FORMAT " at %s\n",
                           suppliers.at(updateContracts->supplier)->company, 
                           updateContracts->quantity, 
                           details.at(updateContracts->detail)->name,
                           updateContracts->price, 
                           updateContracts->delivery.asString(buf,sizeof buf));
                    input("Change this contract (y/n) ? ", buf, sizeof buf);
                    if (*buf == 'y') { 
                        input("New contract price: ", buf, sizeof buf);
                        sscanf(buf, INT8_FORMAT, &updateContracts->price);
                        input("New number of details: ", buf, sizeof buf);
                        sscanf(buf, "%d", &updateContracts->quantity);
                        updateContracts.update();
                    }
                } while (updateContracts.next());
                break;
              case 12:
                input("Do you really want to exit (y/n) ? ", buf, sizeof buf);
                if (*buf == 'y') { 
                    printf("Close database session\n");
                    db.close();
                    return EXIT_SUCCESS;
                }
                break;
              default:
                printf("Please choose menu items 1..12\n");
                continue;
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















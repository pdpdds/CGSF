//-< CONTAINER.H >---------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     05-Nov-2002  K.A. Knizhnik  * / [] \ *
//                          Last update: 05-Nov-2002  K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// T-Tree object container
//-------------------------------------------------------------------*--------*

#ifndef __CONTAINER_H__
#define __CONTAINER_H__

BEGIN_FASTDB_NAMESPACE

/**
 * Spatial search operations
 */
enum SpatialSearchType { 
    SPATIAL_SEARCH_EQUAL,
    SPATIAL_SEARCH_OVERLAPS,
    SPATIAL_SEARCH_SUPERSET,
    SPATIAL_SEARCH_PROPER_SUPERSE,
    SPATIAL_SEARCH_SUBSET,
    SPATIAL_SEARCH_PROPER_SUBSET
};

/**
 * Base class for all containers.
 * Container are implemented using T-Tree
 */
class FASTDB_DLL_ENTRY dbAnyContainer : public dbAnyReference {
  protected:
    dbFieldDescriptor* fd;

    void create(dbDatabase* db);
    void purge(dbDatabase* db);
    void deallocate(dbDatabase* db);
    void add(dbDatabase* db, dbAnyReference const& ref);
    void remove(dbDatabase* db, dbAnyReference const& ref);
    int  search(dbAnyCursor& cursor, void const* from, void const* till);
    int  prefixSearch(dbAnyCursor& cursor, char const* key);
    int  prefixSearch(dbAnyCursor& cursor, wchar_t const* key);
    int  spatialSearch(dbAnyCursor& cursor, rectangle const& r, SpatialSearchType type);

    dbAnyContainer(char const* fieldName, dbTableDescriptor& desc);
};


/**
 * Template of container for particular table
 */
template<class T>
class dbContainer : public dbAnyContainer {
  public:
    /**
     * Search records matching search criteria (between, less or equal, great or equal)
     * @param cursor cursor to iterate through selected resords
     * @param from inclusive low bound for the search key, if <code>NULL</code> then there is no low bound
     * @param till inclusive high bound for the search key,  if <code>NULL</code> then there is no high bound
     * @return number of selected records
     */
    int search(dbCursor<T>& cursor, void const* from, void const* till) {
        return dbAnyContainer::search(cursor, from, till);
    }
    /**
     * Select records with specified value of the key
     * @param cursor cursor to iterate through selected resords
     * @param key searched value of the key
     * @return number of selected records
     */    
    int search(dbCursor<T>& cursor, void const* key) {
        return dbAnyContainer::search(cursor, key, key);
    }

    /**
     * Select records which indexed field is prefix of specfied key
     * @param cursor cursor to iterate through selected resords
     * @param key string key which prefixes are located in the index
     * @return number of selected records
     */    
    int prefixSearch(dbCursor<T>& cursor, char const* key) {
        return dbAnyContainer::prefixSearch(cursor, key);
    }

    /**
     * Select all records in the container
     * @param cursor cursor to iterate through selected resords
     * @return number of selected records
     */
    int search(dbCursor<T>& cursor) {
        return dbAnyContainer::search(cursor, NULL, NULL);
    }

    /**
     * Perform spatial search using R-Tree
     * @param cursor cursor to iterate through selected resords
     * @param r search rectangle
     * @param type search criteria: one of <code>SpatialSearchType</code>
     * @return number of selected records
     */    
    int spatialSearch(dbCursor<T>& cursor, rectangle const& r, SpatialSearchType type) { 
        return dbAnyContainer::spatialSearch(cursor, r, type);
    }

    /**
     * Create new container.
     */
    void create() {
        dbAnyContainer::create(T::dbDescriptor.getDatabase());
    }

    /**
     * Clear the container
     */
    void purge() {
        dbAnyContainer::purge(T::dbDescriptor.getDatabase());
    }

    /**
     * Deallocate container
     */
    void deallocate() {
        dbAnyContainer::deallocate(T::dbDescriptor.getDatabase());
    }

    /**
     * Add new record to container
     * @param ref reference to the record added to the container
     */
    void add(dbReference<T> const& ref) {
        dbAnyContainer::add(T::dbDescriptor.getDatabase(), ref);
    }

    /**
     * Remove record from the container
     * @param ref reference to the record deleted from the container
     */
    void remove(dbReference<T> const& ref) {
        dbAnyContainer::remove(T::dbDescriptor.getDatabase(), ref);
    }

    /**
     * Constructor of the contanier reference
     * @param fieldName name of the key field used by container
     */
    dbContainer(const char* fieldName) : dbAnyContainer(fieldName, T::dbDescriptor) {}
    
    /**
     * Create new container in specified database.
     * @param db database
     */
    void create(dbDatabase* db) {
        dbAnyContainer::create(db);
    }

    /**
     * Clear the container in specified database.
     * @param db database
     */
    void purge(dbDatabase* db) {
        dbAnyContainer::purge(db);
    }

    /**
     * Deallocate container in specified database.
     * @param db database
     */
    void deallocate(dbDatabase* db) {
        dbAnyContainer::deallocate(db);
    }

    /**
     * Add new record to container in specified database.
     * @param db database
     * @param ref reference to the record added to the container
     */
    void add(dbDatabase* db, dbReference<T> const& ref) {
        dbAnyContainer::add(db, ref);
    }

    /**
     * Remove record from the container in specified database.
     * @param db database
     * @param ref reference to the record deleted from the container
     */
    void remove(dbDatabase* db, dbReference<T> const& ref) {
        dbAnyContainer::remove(db, ref);
    }

    /**
     * Constructor of the contanier reference
     * @param db database
     * @param fieldName name of the key field used by container
     */
    dbContainer(dbDatabase* db, const char* fieldName) 
    : dbAnyContainer(fieldName, *db->lookupTable(&T::dbDescriptor)) {}
};

END_FASTDB_NAMESPACE

#endif





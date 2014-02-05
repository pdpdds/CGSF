//-< QUERY.H >-------------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 10-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Constructing and hashing database query statements
//-------------------------------------------------------------------*--------*

#ifndef __QUERY_H__
#define __QUERY_H__

BEGIN_FASTDB_NAMESPACE

/**
 * Element of the query
 */
class FASTDB_DLL_ENTRY dbQueryElement { 
    friend class dbQuery;
    friend class dbCompiler;
    friend class dbQueryExpression;
    friend class dbQueryElementAllocator;
    friend class dbCLI;
  public:
    enum ElementType { 
        qExpression, // part of SQL expression
        qVarBool,
        qVarInt1,
        qVarInt2,
        qVarInt4,
        qVarInt8,
        qVarReal4,
        qVarReal8,
        qVarString,
        qVarStringPtr,
        qVarWString,
        qVarWStringPtr,
        qVarReference,
        qVarRectangle, 
        qVarRectanglePtr, 
        qVarArrayOfRef, 
        qVarArrayOfInt4,
        qVarArrayOfInt8,
        qVarArrayOfRefPtr,
        qVarArrayOfInt4Ptr,
        qVarArrayOfInt8Ptr,
        qVarRawData,
        qVarRawDataPtr,
#ifdef USE_STD_STRING
        qVarStdString,
        qVarStdWString,
#endif
        qVarUnknown
    };
    
    ElementType getType() const { return type; }
    dbQueryElement* nextElement() const { return next; }

    void* operator new (size_t size EXTRA_DEBUG_NEW_PARAMS);
    void  operator delete(void* p EXTRA_DEBUG_NEW_PARAMS);

    char* dump(char* buf);
    char* dumpValues(char* buf);

    dbQueryElement(ElementType t, void const* p, 
                   dbTableDescriptor* table = NULL) 
    {
        type = t;
        ptr  = p;       
        ref  = table;
        next = NULL;
    } 
  private:
    dbQueryElement*    next;
    void const*        ptr;
    ElementType        type;
    dbTableDescriptor* ref;
};


/**
 * Allocator of query elements. This class links deallocated elements in the list of free elements,
 * and reused them in future. So number of system memory allocator invocations is dramatically reduced.
 * Cleanup of free elements lst is performed by <code>dbDatabase::cleanup()</code> method
 */
class FASTDB_DLL_ENTRY dbQueryElementAllocator { 
    friend class dbDatabase;

    dbMutex         mutex;
    dbQueryElement* freeChain;
    
  public:
    void deallocate(dbQueryElement* first, dbQueryElement** lastNext) { 
        dbCriticalSection cs(mutex);
        if (first != NULL) { 
            *lastNext = freeChain;
            freeChain = first;
        }
    }
        
    void* allocate(size_t size);

    dbQueryElementAllocator();
    ~dbQueryElementAllocator();

    void reset();

    static dbQueryElementAllocator instance;
};

/**
 * This class represents component of the structure.
 * It is mostly needed for implementation of application specific database types. For example, look at
 * <code>dbDate</code> class. It contains <code>int4 jday</code> component which stores time in seconds sinse 1970.
 * This class defines its own comparison methods:
 * <PRE>
 *     dbQueryExpression operator == (char const* field) { 
 *         dbQueryExpression expr;
 *         expr = dbComponent(field,"jday"),"=",jday;
 *         return expr;
 *     }
 * </PRE>
 * Making it possible to sepcify queries like this (assume that record has column "released" with type dbDate:
 * <PRE>
 *     dbQuery q;
 *     dbDate  date;
 *     q = date == "released";
 * </PRE>
 */
class FASTDB_DLL_ENTRY dbComponent { 
  public:
    char const* structure;
    char const* field; 

    dbComponent(char const* s, char const* f=NULL) : structure(s), field(f) {}
};


/**
 * Class representing SubSQL expression.  
 * It is mostly needed for implementation of application specific database types.
 * Look at the example in dbComponent class. 
 * The effect of addeing dbExpression to the query is the same as if this expresion is enclosed in parenthesis.
 */
class FASTDB_DLL_ENTRY dbQueryExpression { 
    friend class dbQuery;
    dbQueryElement*  first;
    dbQueryElement** last;
    bool             operand;

  public:
    dbQueryExpression& add(dbQueryElement::ElementType type, void const* ptr, dbTableDescriptor* table = NULL) {
        last = &(*last = new dbQueryElement(type, ptr, table))->next;
        operand = (type == dbQueryElement::qExpression);
        return *this;
    }
        
    dbQueryExpression& operator = (char const* ptr) { 
        first = NULL, last = &first;
        return add(dbQueryElement::qExpression, ptr);
    }
    dbQueryExpression& operator = (dbComponent const& comp);

    dbQueryExpression& operator = (dbQueryExpression const& expr);

    dbQueryExpression& operator,(int1 const& ptr) { 
        return add(dbQueryElement::qVarInt1, &ptr);
    }
    dbQueryExpression& operator,(int2 const& ptr) { 
        return add(dbQueryElement::qVarInt2, &ptr);
    }
    dbQueryExpression& operator,(int4 const& ptr) { 
        return add(dbQueryElement::qVarInt4, &ptr);
    }
    dbQueryExpression& operator,(db_int8 const& ptr) { 
        return add(dbQueryElement::qVarInt8, &ptr);
    }
    dbQueryExpression& operator,(nat1 const& ptr) {
        return add(dbQueryElement::qVarInt1, &ptr);
    }
    dbQueryExpression& operator,(nat2 const& ptr) {
        return add(dbQueryElement::qVarInt2, &ptr);
    }
    dbQueryExpression& operator,(nat4 const& ptr) {
        return add(dbQueryElement::qVarInt4, &ptr);
    }
    dbQueryExpression& operator,(db_nat8 const& ptr) {
        return add(dbQueryElement::qVarInt8, &ptr);
    }
#if SIZEOF_LONG != 8 
    dbQueryExpression& operator,(long const& ptr) {
        return add(sizeof(long) == 4 ? dbQueryElement::qVarInt4 : dbQueryElement::qVarInt8, &ptr);
    }
    dbQueryExpression& operator,(unsigned long const& ptr) {
        return add(sizeof(long) == 4 ? dbQueryElement::qVarInt4 : dbQueryElement::qVarInt8, &ptr);
    }
#endif
    dbQueryExpression& operator,(real4 const& ptr) { 
        return add(dbQueryElement::qVarReal4, &ptr);
    }
    dbQueryExpression& operator,(real8 const& ptr) { 
        return add(dbQueryElement::qVarReal8, &ptr);
    }
    dbQueryExpression& operator,(bool const& ptr) { 
        return add(dbQueryElement::qVarBool, &ptr);
    }
    dbQueryExpression& operator,(char const* ptr) { 
        return add(operand ? dbQueryElement::qVarString 
                   : dbQueryElement::qExpression, ptr);
    }
    dbQueryExpression& operator,(char const** ptr) { 
        return add(dbQueryElement::qVarStringPtr, ptr);
    }
    dbQueryExpression& operator,(char** ptr) { 
        return add(dbQueryElement::qVarStringPtr, ptr);
    }
    dbQueryExpression& operator,(wchar_t const* ptr) { 
        return add(dbQueryElement::qVarWString, ptr);
    }
    dbQueryExpression& operator,(wchar_t const** ptr) { 
        return add(dbQueryElement::qVarWStringPtr, ptr);
    }
    dbQueryExpression& operator,(wchar_t** ptr) { 
        return add(dbQueryElement::qVarWStringPtr, ptr);
    }
    dbQueryExpression& operator,(void const* ptr) { 
        return add(dbQueryElement::qVarRawData, ptr);
    }
    dbQueryExpression& operator,(void** ptr) { 
        return add(dbQueryElement::qVarRawDataPtr, ptr);
    }
    dbQueryExpression& operator,(rectangle const& rect) {
        return add(dbQueryElement::qVarRectangle, &rect);
    }
    dbQueryExpression& operator,(rectangle* rect) {
        return add(dbQueryElement::qVarRectanglePtr, &rect);
    }
#ifdef USE_STD_STRING
    dbQueryExpression& operator,(std::string const& str) {
        return add(dbQueryElement::qVarStdString, &str);
    }
    dbQueryExpression& operator,(std::wstring const& str) {
        return add(dbQueryElement::qVarStdWString, &str);
    }
#endif
    dbQueryExpression& operator,(dbQueryExpression const& expr) { 
        *last = new dbQueryElement(dbQueryElement::qExpression, "(");
        (*last)->next = expr.first;
        last = expr.last;
        *last = new dbQueryElement(dbQueryElement::qExpression, ")");
        last = &(*last)->next;
        operand = false;
        return *this;
    }
    dbQueryExpression& operator,(dbComponent const& comp) { 
        add(dbQueryElement::qExpression, comp.structure);
        if (comp.field != NULL) { 
            add(dbQueryElement::qExpression, ".");
            add(dbQueryElement::qExpression, comp.field);
        }
        operand = false;
        return *this;
    }
    dbQueryExpression& operator += (dbComponent const& comp) { 
        return *this,comp;
    }
    dbQueryExpression& operator += (char const* ptr) { 
        return add(dbQueryElement::qExpression, ptr);
    }
#ifndef NO_MEMBER_TEMPLATES
    template<class T>
    dbQueryExpression& operator,(dbReference<T> const& value) { 
        return add(dbQueryElement::qVarReference, &value, &T::dbDescriptor);
    }

    template<class T>
    inline dbQueryExpression& operator,(dbArray< dbReference<T> > const& value) { 
        return add(dbQueryElement::qVarArrayOfRef, &value, &T::dbDescriptor);
    }

    template<class T>
    inline dbQueryExpression& operator,(dbArray< dbReference<T> >const* const& value) { 
        return add(dbQueryElement::qVarArrayOfRefPtr, &value, &T::dbDescriptor);
    }
#endif
#if !defined(_MSC_VER) || _MSC_VER+0 >= 1300
    inline dbQueryExpression& operator,(dbArray<db_int4> const& value) { 
        return add(dbQueryElement::qVarArrayOfInt4, &value);
    }

    inline dbQueryExpression& operator,(dbArray<db_int4>const* const& value) { 
        return add(dbQueryElement::qVarArrayOfInt4Ptr, &value);
    }

    inline dbQueryExpression& operator,(dbArray<db_int8> const& value) { 
        return add(dbQueryElement::qVarArrayOfInt8, &value);
    }

    inline dbQueryExpression& operator,(dbArray<db_int8>const* const& value) { 
        return add(dbQueryElement::qVarArrayOfInt8Ptr, &value);
    }
#endif
};

class dbOrderByNode;
class dbFollowByNode;


/**
 * Class used for precompiled queries
 */
class FASTDB_DLL_ENTRY dbCompiledQuery { 
  public:
    dbExprNode*        tree;
    dbOrderByNode*     order;
    dbFollowByNode*    follow;
    dbTableDescriptor* table;
    int                schemeVersion;

    cardinality_t      stmtLimitStart;
    cardinality_t      stmtLimitLen;
    cardinality_t*     stmtLimitStartPtr;
    cardinality_t*     stmtLimitLenPtr;
    bool               limitSpecified;

    enum IteratorInit { 
        StartFromAny,
        StartFromFirst,
        StartFromLast,
        StartFromRef, 
        StartFromArray,
        StartFromArrayPtr
    };
    IteratorInit       startFrom;
    void const*        root;

    void destroy();

    bool compiled() { return tree != NULL; }

    bool compileError() { return !compiled(); }

    dbCompiledQuery() { 
        tree = NULL;
        order = NULL;
        follow = NULL;
        table = NULL;
        startFrom = StartFromAny;
        limitSpecified = false;
    }
};

/**
 * Query class. It is derived from <code>dbCompiledQuery</code> class because each query is compiled only once - when
 * it is executed first time. All subsequent executions of query used precompiled tree.
 */
class FASTDB_DLL_ENTRY dbQuery : public dbCompiledQuery { 
    friend class dbCompiler;
    friend class dbDatabase;
    friend class dbSubSql;
    friend class dbCLI;
  private:
    dbMutex            mutex;
    dbQueryElement*    elements;
    dbQueryElement**   nextElement;
    bool               operand;
    bool               mutexLocked;

    //
    // Prohibite query copying
    //
    dbQuery(dbQuery const&) : dbCompiledQuery() {} 
    dbQuery& operator =(dbQuery const&) { return *this; }

  public:
    int                pos; // position of condition in statement


    char* dump(char* buf) { 
        char* p = buf;
        for (dbQueryElement* elem = elements; elem != NULL; elem = elem->next) { 
            p = elem->dump(p);
        }
        return buf;
    }

    char* dumpValues(char* buf) { 
        char* p = buf;
        for (dbQueryElement* elem = elements; elem != NULL; elem = elem->next) { 
            p = elem->dumpValues(p);
        }
        return buf;
    }

    dbQuery& append(dbQueryElement::ElementType type, void const* ptr,
                    dbTableDescriptor* table = NULL) 
    { 
        nextElement = &(*nextElement=new dbQueryElement(type,ptr,table))->next;
        operand = (type == dbQueryElement::qExpression);
        return *this;
    }

    dbQuery& reset();

    //
    // Redefined operator = and , make it possible to specify query in the
    // following way:
    //         int x, y;
    //         dbDataTime dt;
    //         dbQuery q; 
    //         dbCursor<record> cursor;
    //         q = "x=",x,"and y=",y,"and",dt == "date";
    //         for (x = 0; x < max_x; x++) { 
    //             for (y = 0; y < max_y; y++) { 
    //                 cursor.select(q);
    //                 ...
    //             }
    //         }

    dbQuery& add(dbQueryExpression const& expr); 

    dbQuery& And(dbQueryExpression const& expr) { 
        if (elements != NULL) { 
            append(dbQueryElement::qExpression, "and");
        }
        return add(expr);
    }

    dbQuery& Or(dbQueryExpression const& expr) { 
        if (elements != NULL) { 
            append(dbQueryElement::qExpression, "or");
        }
        return add(expr);
    }

    dbQuery& And(char const* str) { 
        if (elements != NULL) { 
            append(dbQueryElement::qExpression, "and");
        }
        return append(dbQueryElement::qExpression, str);
    }

    dbQuery& Or(char const* str) { 
        if (elements != NULL) { 
            append(dbQueryElement::qExpression, "or");
        }
        return append(dbQueryElement::qExpression, str);
    }

    dbQuery& add(char const* str) { 
        return append(operand ? dbQueryElement::qVarString 
                      : dbQueryElement::qExpression, str);
    }
    dbQuery& add(char const** str) { 
        return append(dbQueryElement::qVarStringPtr, str);
    }
    dbQuery& add(wchar_t const* str) { 
        return append(dbQueryElement::qVarWString, str);
    }
    dbQuery& add(wchar_t const** str) { 
        return append(dbQueryElement::qVarWStringPtr, str);
    }
    dbQuery& add(rectangle const& rect) {
        return append(dbQueryElement::qVarRectangle, &rect);
    }
    dbQuery& add(rectangle* rect) {
        return append(dbQueryElement::qVarRectanglePtr, &rect);
    }
#ifdef USE_STD_STRING
    dbQuery& add(std::string const& str) { 
        return append(dbQueryElement::qVarStdString, &str);
    }
    dbQuery& operator,(std::string const& str) { return add(str); }
    dbQuery& add(std::wstring const& str) { 
        return append(dbQueryElement::qVarStdWString, &str);
    }
    dbQuery& operator,(std::wstring const& str) { return add(str); }
#endif
    dbQuery& add(char** str) { 
        return append(dbQueryElement::qVarStringPtr, str);
    }
    dbQuery& add(wchar_t** str) { 
        return append(dbQueryElement::qVarWStringPtr, str);
    }
    dbQuery& add(int1 const& value) { 
        return append(dbQueryElement::qVarInt1, &value);
    }
    dbQuery& add (int2 const& value) { 
        return append(dbQueryElement::qVarInt2, &value);
    }
    dbQuery& add (int4 const& value) { 
        return append(dbQueryElement::qVarInt4, &value);
    }
    dbQuery& add (db_int8 const& value) { 
        return append(dbQueryElement::qVarInt8, &value);
    }
    dbQuery& add(nat1 const& value) {
        return append(dbQueryElement::qVarInt1, &value);
    }
    dbQuery& add (nat2 const& value) {
        return append(dbQueryElement::qVarInt2, &value);
    }
    dbQuery& add (nat4 const& value) {
        return append(dbQueryElement::qVarInt4, &value);
    }
    dbQuery& add (db_nat8 const& value) {
        return append(dbQueryElement::qVarInt8, &value);
    }
#if SIZEOF_LONG != 8 
    dbQuery& add (long const& value) {
        return append(sizeof(long) == 4 ? dbQueryElement::qVarInt4 : dbQueryElement::qVarInt8, &value);
    }
    dbQuery& add (unsigned long const& value) {
        return append(sizeof(long) == 4 ? dbQueryElement::qVarInt4 : dbQueryElement::qVarInt8, &value);
    }
#endif
    dbQuery& add (real4 const& value) { 
        return append(dbQueryElement::qVarReal4, &value);
    }
    dbQuery& add(real8 const& value) { 
        return append(dbQueryElement::qVarReal8, &value);
    }
    dbQuery& add(bool const& value) { 
        return append(dbQueryElement::qVarBool, &value);
    }
    dbQuery& add(void const* value) { 
        return append(dbQueryElement::qVarRawData, value);
    }
    dbQuery& add(void** value) { 
        return append(dbQueryElement::qVarRawDataPtr, value);
    }

    dbQuery& operator,(char const*  value) { return add(value); }
    dbQuery& operator,(char const** value) { return add(value); }
    dbQuery& operator,(char** value) { return add(value); }
    dbQuery& operator,(wchar_t const*  value) { return add(value); }
    dbQuery& operator,(wchar_t const** value) { return add(value); }
    dbQuery& operator,(wchar_t** value) { return add(value); }
    dbQuery& operator,(int1 const&  value) { return add(value); }
    dbQuery& operator,(int2 const&  value) { return add(value); }
    dbQuery& operator,(int4 const&  value) { return add(value); }
    dbQuery& operator,(db_int8 const&  value) { return add(value); }
    dbQuery& operator,(nat1 const&  value) { return add(value); }
    dbQuery& operator,(nat2 const&  value) { return add(value); }
    dbQuery& operator,(nat4 const&  value) { return add(value); }
    dbQuery& operator,(db_nat8 const&  value) { return add(value); }
#if SIZEOF_LONG != 8
    dbQuery& operator,(long const&  value) { return add(value); }
    dbQuery& operator,(unsigned long const&  value) { return add(value); }
#endif
    dbQuery& operator,(real4 const& value) { return add(value); }
    dbQuery& operator,(real8 const& value) { return add(value); }
    dbQuery& operator,(bool const&  value) { return add(value); }
    dbQuery& operator,(void const*  value) { return add(value); }
    dbQuery& operator,(dbQueryExpression const& expr) { return add(expr); }
    dbQuery& operator,(rectangle const& rect) { return add(rect); }

    dbQuery& operator = (const char* str) { 
        return reset().append(dbQueryElement::qExpression, str);
    }

#if !defined(_MSC_VER) || _MSC_VER+0 >= 1300
    inline dbQuery& operator,(dbArray<db_int4> const& value) { 
        return append(dbQueryElement::qVarArrayOfInt4, &value);
    }

    inline dbQuery& operator,(dbArray<db_int4>const* const& value) { 
        return append(dbQueryElement::qVarArrayOfInt4Ptr, &value);
    }

    inline dbQuery& operator,(dbArray<db_int8> const& value) { 
        return append(dbQueryElement::qVarArrayOfInt8, &value);
    }

    inline dbQuery& operator,(dbArray<db_int8>const* const& value) { 
        return append(dbQueryElement::qVarArrayOfInt8Ptr, &value);
    }

    inline dbQuery& add(dbArray<db_int4> const& value) { 
        return append(dbQueryElement::qVarArrayOfInt4, &value);
    }

    inline dbQuery& add(dbArray<db_int4>const* const& value) { 
        return append(dbQueryElement::qVarArrayOfInt4Ptr, &value);
    }

    inline dbQuery& add(dbArray<db_int8> const& value) { 
        return append(dbQueryElement::qVarArrayOfInt8, &value);
    }

    inline dbQuery& add(dbArray<db_int8>const* const& value) { 
        return append(dbQueryElement::qVarArrayOfInt8Ptr, &value);
    }
#endif

#ifndef NO_MEMBER_TEMPLATES
    template<class T>
    dbQuery& operator,(dbReference<T> const& value) { 
        return append(dbQueryElement::qVarReference, &value, &T::dbDescriptor);
    }

    template<class T>
    inline dbQuery& operator,(dbArray< dbReference<T> > const& value) { 
        return append(dbQueryElement::qVarArrayOfRef, &value, &T::dbDescriptor);
    }

    template<class T>
    inline dbQuery& operator,(dbArray< dbReference<T> >const* const& value) { 
        return append(dbQueryElement::qVarArrayOfRefPtr, &value, &T::dbDescriptor);
    }

    template<class T>
    dbQuery& add(dbReference<T> const& value) { 
        return append(dbQueryElement::qVarReference, &value, &T::dbDescriptor);
    }

    template<class T>
    dbQuery& add(dbArray< dbReference<T> > const& value) { 
        return append(dbQueryElement::qVarArrayOfRef, &value, &T::dbDescriptor);
    }
    template<class T>
    dbQuery& add(dbArray< dbReference<T> >const* const& value) { 
        return append(dbQueryElement::qVarArrayOfRefPtr, &value, &T::dbDescriptor);
    }

    template<class T>
    dbQuery& operator = (T const& value) { 
        return reset().add(value);
    }   
#else
    dbQuery& operator = (dbQueryExpression const& expr) {
        return reset().add(expr);
    }    
    dbQuery& operator = (rectangle const& expr) {
        return reset().add(expr);
    }    
#endif


    dbQueryElement* getElements() const { 
        return elements;
    }

    bool isEmpty() { 
        return elements == NULL 
            || (elements->next == NULL && elements->type == dbQueryElement::qExpression && *(char*)elements->ptr == 0);
    }

    dbQuery() { 
        elements = NULL;
        nextElement = &elements;
        operand = false;
        pos = 0;
    } 
    dbQuery(char const* str) { 
        elements = new dbQueryElement(dbQueryElement::qExpression, str);
        nextElement = &elements->next;
        operand = true;
        pos = 0;
    } 
    ~dbQuery() { 
        reset();
    }
};

#ifdef NO_MEMBER_TEMPLATES
template<class T>
inline dbQueryExpression& operator,(dbQueryExpression& expr, dbReference<T> const& value) { 
    return expr.add(dbQueryElement::qVarReference, &value, &T::dbDescriptor);
}
template<class T>
inline dbQueryExpression& operator,(dbQueryExpression& expr, dbArray< dbReference<T> > const& value) { 
    return expr.add(dbQueryElement::qVarArrayOfRef, &value, 
                    &T::dbDescriptor);
}

template<class T>
inline dbQueryExpression& operator,(dbQueryExpression& expr, dbArray< dbReference<T> >const* const& value) { 
    return expr.add(dbQueryElement::qVarArrayOfRefPtr, &value, &T::dbDescriptor);
}

template<class T>
inline dbQuery& operator,(dbQuery& query, dbReference<T> const& value) { 
    return query.append(dbQueryElement::qVarReference, &value, 
                        &T::dbDescriptor);
}

template<class T>
inline dbQuery& operator,(dbQuery& query, 
                             dbArray< dbReference<T> > const& value) 
{ 
    return query.append(dbQueryElement::qVarArrayOfRef, &value, 
                        &T::dbDescriptor);
}

template<class T>
inline dbQuery& operator,(dbQuery& query, 
                             dbArray< dbReference<T> >const* const& value) 
{ 
    return query.append(dbQueryElement::qVarArrayOfRefPtr, &value, 
                        &T::dbDescriptor);
}

template<class T>
inline dbQuery& add(dbQuery& query, dbReference<T> const& value) { 
    return query.append(dbQueryElement::qVarReference, &value, &T::dbDescriptor);
}

template<class T>
inline dbQuery& add(dbQuery& query, dbArray< dbReference<T> > const& value) { 
    return query.append(dbQueryElement::qVarArrayOfRef, &value, 
                        &T::dbDescriptor);
}

template<class T>
inline dbQuery& add(dbQuery& query, dbArray< dbReference<T> >const* const& value) { 
    return query.append(dbQueryElement::qVarArrayOfRefPtr, &value, &T::dbDescriptor);
}
#endif

#define USER_FUNC(f) static dbUserFunction f##_descriptor(&f, (char*)#f)

struct dbInheritedAttribute;
union  dbSynthesizedAttribute;

/**
 * This class represent argument of user defined funtion. 
 * @see dbUserFunction class.
 */
class FASTDB_DLL_ENTRY  dbUserFunctionArgument { 
  public:
    enum dbArgumentType { 
        atInteger, 
        atBoolean, 
        atString, 
        atReal, 
        atReference, 
        atRawBinary,
        atWString
    };  
    dbArgumentType type; 
    union {
        real8          realValue;
        db_int8        intValue;
        bool           boolValue;
        char const*    strValue;
        wchar_t const* wstrValue;
        oid_t          oidValue;
        void*          rawValue;
    } u;

  private:
    friend class dbDatabase;
    dbUserFunctionArgument(dbExprNode*             expr, 
                           dbInheritedAttribute&   iattr, 
                           dbSynthesizedAttribute& sattr, 
                           int                     i);
};

/**
 * dbUserFunction class is used to bind user defined functions to the database.
 * This class contains list of all user functions declared by application and provides method
 * for locating function by name and adding new user defined functions.
 * User defined functions can have fro 0 till 3 parameters (of <code>dbUserFunctionArgument</code> type)
 * and should return integer, boolean, real or string value. Type of the function is detected by overloaded 
 * constructors. User defined function descriptor is cerated by <code>USER_FUNC</code> macro.
 */
class FASTDB_DLL_ENTRY dbUserFunction { 
    friend class dbDatabase;
    friend class dbCompiler;

    void* fptr;
    char* fname;
    
    dbUserFunction* next;
    static dbUserFunction* list;

    enum funcType {
        fInt2Bool,
        fReal2Bool,
        fStr2Bool,
        fWStr2Bool,
        fInt2Int,
        fReal2Int,
        fStr2Int,
        fWStr2Int,
        fInt2Real,
        fReal2Real,
        fStr2Real,
        fWStr2Real,
        fInt2Str,
        fReal2Str,
        fStr2Str, 
        fWStr2Str, 
        fInt2WStr,
        fReal2WStr,
        fStr2WStr, 
        fWStr2WStr, 
        fArg2Bool, 
        fArg2Int, 
        fArg2Real, 
        fArg2Str, 
        fArg2WStr, 
        fArgArg2Bool, 
        fArgArg2Int, 
        fArgArg2Real, 
        fArgArg2Str, 
        fArgArg2WStr, 
        fArgArgArg2Bool, 
        fArgArgArg2Int, 
        fArgArgArg2Real, 
        fArgArgArg2Str,
        fArgArgArg2WStr
    };
    int type;

    void bind(char* name, void* f, funcType ftype);

  public:

    static dbUserFunction* find(char const* name) { 
        for (dbUserFunction* func = list; func != NULL; func = func->next) { 
            if (name == func->fname) { 
                return func;
            }
        }
        return NULL;
    }
    
    int getParameterType();

    int getNumberOfParameters();

    dbUserFunction(bool (*f)(db_int8), char* name) { 
        bind(name, (void*)f, fInt2Bool); 
    }
    dbUserFunction(bool (*f)(real8), char* name) { 
        bind(name, (void*)f, fReal2Bool); 
    }
    dbUserFunction(bool (*f)(char const*), char* name) { 
        bind(name, (void*)f, fStr2Bool); 
    }
    dbUserFunction(bool (*f)(wchar_t const*), char* name) { 
        bind(name, (void*)f, fWStr2Bool); 
    }
    dbUserFunction(db_int8 (*f)(db_int8), char* name) { 
        bind(name, (void*)f, fInt2Int); 
    }
    dbUserFunction(db_int8 (*f)(real8), char* name) { 
        bind(name, (void*)f, fReal2Int); 
    }
    dbUserFunction(db_int8 (*f)(char const*), char* name) { 
        bind(name, (void*)f, fStr2Int); 
    }
    dbUserFunction(db_int8 (*f)(wchar_t const*), char* name) { 
        bind(name, (void*)f, fWStr2Int); 
    }
    dbUserFunction(real8 (*f)(db_int8), char* name) { 
        bind(name, (void*)f, fInt2Real); 
    }
    dbUserFunction(real8 (*f)(real8), char* name) { 
        bind(name, (void*)f, fReal2Real); 
    }
    dbUserFunction(real8 (*f)(char const*), char* name) { 
        bind(name, (void*)f, fStr2Real); 
    }
    dbUserFunction(real8 (*f)(wchar_t const*), char* name) { 
        bind(name, (void*)f, fWStr2Real); 
    }
    dbUserFunction(char* (*f)(db_int8), char* name) { 
        bind(name, (void*)f, fInt2Str); 
    }
    dbUserFunction(char* (*f)(real8), char* name) { 
        bind(name, (void*)f, fReal2Str); 
    }
    dbUserFunction(char* (*f)(char const*), char* name) { 
        bind(name, (void*)f, fStr2Str); 
    }
    dbUserFunction(char* (*f)(wchar_t const*), char* name) { 
        bind(name, (void*)f, fWStr2Str); 
    }
    dbUserFunction(wchar_t* (*f)(db_int8), char* name) { 
        bind(name, (void*)f, fInt2WStr); 
    }
    dbUserFunction(wchar_t* (*f)(real8), char* name) { 
        bind(name, (void*)f, fReal2WStr); 
    }
    dbUserFunction(wchar_t* (*f)(char const*), char* name) { 
        bind(name, (void*)f, fStr2WStr); 
    }
    dbUserFunction(wchar_t* (*f)(wchar_t const*), char* name) { 
        bind(name, (void*)f, fWStr2WStr); 
    }


    dbUserFunction(bool (*f)(dbUserFunctionArgument&), char* name) { 
        bind(name, (void*)f, fArg2Bool); 
    }
    dbUserFunction(char* (*f)(dbUserFunctionArgument&), char* name) { 
        bind(name, (void*)f, fArg2Str); 
    }
    dbUserFunction(wchar_t* (*f)(dbUserFunctionArgument&), char* name) { 
        bind(name, (void*)f, fArg2WStr); 
    }
    dbUserFunction(db_int8 (*f)(dbUserFunctionArgument&), char* name) { 
        bind(name, (void*)f, fArg2Int); 
    }
    dbUserFunction(real8 (*f)(dbUserFunctionArgument&), char* name) { 
        bind(name, (void*)f, fArg2Real); 
    }

    dbUserFunction(bool (*f)(dbUserFunctionArgument&, dbUserFunctionArgument&), char* name) { 
        bind(name, (void*)f, fArgArg2Bool); 
    }
    dbUserFunction(char* (*f)(dbUserFunctionArgument&, dbUserFunctionArgument&), char* name) { 
        bind(name, (void*)f, fArgArg2Str); 
    }
    dbUserFunction(wchar_t* (*f)(dbUserFunctionArgument&, dbUserFunctionArgument&), char* name) { 
        bind(name, (void*)f, fArgArg2WStr); 
    }
    dbUserFunction(db_int8 (*f)(dbUserFunctionArgument&, dbUserFunctionArgument&), char* name) { 
        bind(name, (void*)f, fArgArg2Int); 
    }
    dbUserFunction(real8 (*f)(dbUserFunctionArgument&, dbUserFunctionArgument&), char* name) { 
        bind(name, (void*)f, fArgArg2Real); 
    }


    dbUserFunction(bool (*f)(dbUserFunctionArgument&, dbUserFunctionArgument&, dbUserFunctionArgument&), char* name) { 
        bind(name, (void*)f, fArgArgArg2Bool); 
    }
    dbUserFunction(char* (*f)(dbUserFunctionArgument&, dbUserFunctionArgument&, dbUserFunctionArgument&), char* name) { 
        bind(name, (void*)f, fArgArgArg2Str); 
    }
    dbUserFunction(wchar_t* (*f)(dbUserFunctionArgument&, dbUserFunctionArgument&, dbUserFunctionArgument&), char* name) { 
        bind(name, (void*)f, fArgArgArg2WStr); 
    }
    dbUserFunction(db_int8 (*f)(dbUserFunctionArgument&, dbUserFunctionArgument&, dbUserFunctionArgument&), char* name) { 
        bind(name, (void*)f, fArgArgArg2Int); 
    }
    dbUserFunction(real8 (*f)(dbUserFunctionArgument&, dbUserFunctionArgument&, dbUserFunctionArgument&), char* name) { 
        bind(name, (void*)f, fArgArgArg2Real); 
    }

    ~dbUserFunction();
};

END_FASTDB_NAMESPACE

#endif




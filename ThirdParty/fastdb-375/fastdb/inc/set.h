//-< SET.H >---------------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     17-Jan-99    K.A. Knizhnik  * / [] \ *
//                          Last update: 25-Jan-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Small set implementation. Number of elements in the set should
// not be greater than 64. Template class is used to allow static
// type checking for different sets.
//-------------------------------------------------------------------*--------*

#ifndef __SET_H__
#define __SET_H__

BEGIN_FASTDB_NAMESPACE

/**
 * Small set implementation. Number of elements in the set should
 *  not be greater than 64. Template class is used to allow static
 *  type checking for different sets.
 */
template<class T>
class dbSet { 
  protected:
    typedef db_int8 set_t;
    set_t bits;
  public:
    dbSet(set_t elements = 0) : bits(elements) {}
    dbSet(T elem) : bits((set_t)1 << elem) {}
    dbSet(dbSet const& s) : bits(s.bits) {}

    dbSet(T from, T till)  {
        assert(from <= till);
        bits = ((set_t)1 << till << 1) - ((set_t)1 << till);
    }

    bool operator == (dbSet const& s) const {
        return bits == s.bits; 
    }
    bool operator != (dbSet const& s) const { 
        return bits != s.bits; 
    }
    bool operator == (T elem) const { 
        return bits == ((set_t)1 << elem); 
    }
    bool operator != (T elem) const { 
        return bits != ((set_t)1 << elem); 
    }
    bool operator <= (dbSet const& s) const { 
        return (bits & ~s.bits) == 0; 
    }
    bool operator >= (dbSet const& s) const { 
        return (s.bits & ~bits) == 0; 
    }
    bool operator < (dbSet const& s) const { 
        return (bits & ~s.bits) == 0 && bits != s.bits; 
    }
    bool operator > (dbSet const& s) const { 
        return (s.bits & ~bits) == 0 && bits != s.bits; 
    }
    dbSet operator + (dbSet const& s) const {
        dbSet result(bits | s.bits);
        return result;
    }
    dbSet operator + (T elem) const {
        dbSet result(bits | ((set_t)1 << elem));
        return result;
    }
    dbSet operator - (dbSet const& s) const {
        dbSet result(bits & ~s.bits);
        return result;
    }
    dbSet operator - (T elem) const {
        dbSet result(bits & ~((set_t)1 << elem));
        return result;
    }
    dbSet operator * (dbSet const& s) const {
        dbSet result(bits & s.bits);
        return result;
    }
    dbSet operator * (T elem) const {
        dbSet result(bits & ((set_t)1 << elem));
        return result;
    }                      
    bool has(T elem) const { 
        return (bits & ((set_t)1 << elem)) != 0;
    }
    bool empty() { 
        return bits == 0;
    }
    dbSet operator += (T elem) { 
        bits |= (set_t)1 << elem;
        return *this;
    }
    dbSet operator -= (T elem) { 
        bits &= ~((set_t)1 << elem);
        return *this;
    }
    dbSet operator = (dbSet const& s) { 
        bits = s.bits;
        return *this;
    }
    dbSet operator = (T elem) { 
        bits = (set_t)1 << elem;
        return *this;
    }

    dbSet operator, (dbSet const& s) {
        dbSet result(bits | s.bits);
        return result;
    }
    dbSet operator, (T elem) {
        dbSet result(bits | ((set_t)1 << elem));
        return result;
    }
        
    dbQueryExpression operator == (char const* field) { 
        dbQueryExpression expr;
        expr = dbComponent(field,"bits"),"=",bits;
        return expr;
    }
    dbQueryExpression operator != (char const* field) { 
        dbQueryExpression expr;
        expr = dbComponent(field,"bits"),"!=",bits;
        return expr;
    }
    dbQueryExpression operator <= (char const* field) { 
        dbQueryExpression expr;
        expr = "(not",dbComponent(field,"bits"),"and",bits,")=0";
        return expr;
    }
    dbQueryExpression operator >= (char const* field) { 
        dbQueryExpression expr;
        expr = "(not",bits,"and",dbComponent(field,"bits"),")=0";
        return expr;
    }
    dbQueryExpression operator < (char const* field) { 
        dbQueryExpression expr;
        expr = "(not",dbComponent(field,"bits"),"and",bits,")=0 and",
            dbComponent(field,"bits"),"!=",bits;
        return expr;
    }
    dbQueryExpression operator > (char const* field) { 
        dbQueryExpression expr;
        expr = "(not",bits,"and",dbComponent(field,"bits"),")=0 and",
            dbComponent(field,"bits"),"!=",bits;
        return expr;
    }
    dbQueryExpression has(char const* field) {
        dbQueryExpression expr;
        expr = "(",bits,"and 2^",dbComponent(field),") <> 0";
        return expr;
    }   

    CLASS_DESCRIPTOR(dbSet, (FIELD(bits), METHOD(empty)));
};

END_FASTDB_NAMESPACE

#endif

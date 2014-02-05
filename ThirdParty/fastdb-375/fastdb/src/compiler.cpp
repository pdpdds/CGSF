//-< COMPILE.CPP >---------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 15-Jan-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Conditional expresion compiler
//-------------------------------------------------------------------*--------*

#define INSIDE_FASTDB

#include "database.h"
#include "array.h"
#include "query.h"
#include "cursor.h"
#include "compiler.h"
#include "symtab.h"

#include <ctype.h>

BEGIN_FASTDB_NAMESPACE

nat1 const dbExprNode::nodeTypes[] = { 
#define DBVM(cop, type, n_opernads, commutative) type,
// in the RAID - we cannot use source file with a '.d' suffix - this clashes
// with the CFW build system (specific to StoreAge build system)).
#ifdef VXWORKS
#include "compiler.dd"
#else
#include "compiler.d"
#endif // VXWORKS
0};

nat1 const dbExprNode::nodeOperands[] = { 
#define DBVM(cop, type, n_operands, commutative) n_operands,
// in the RAID - we cannot use source file with a '.d' suffix - this clashes
// with the CFW build system (specific to StoreAge build system).
#ifdef VXWORKS
#include "compiler.dd"
#else
#include "compiler.d"
#endif // VXWORKS
0};

nat1 const dbExprNode::commutativeOperator[] = { 
#define DBVM(cop, type, n_operands, commutative) commutative,
// in the RAID - we cannot use source file with a '.d' suffix - this clashes
// with the CFW build system (specific to StoreAge build system).
#ifdef VXWORKS
#include "compiler.dd"
#else
#include "compiler.d"
#endif // VXWORKS
0};


bool        dbCompiler::initialized;

dbExprNodeAllocator dbExprNodeAllocator::instance; 

dbExprNodeAllocator::~dbExprNodeAllocator() {
    dbExprNodeSegment* segm = segmentList; 
    while (segm != NULL) { 
        dbExprNodeSegment* next = segm->next;
        delete segm;
        segm = next;
    }
}

inline dbExprNode* dbExprNodeAllocator::allocate()
{
    dbCriticalSection cs(mutex);
    dbExprNode* node = freeNodeList;
    if (node == NULL) {
        dbExprNodeSegment* seg = new dbExprNodeSegment;
        seg->next = segmentList;
        segmentList = seg;
        node = (dbExprNode*)seg->buf;
        dbExprNode* free = NULL;
        for (int i = dbExprNodeSegment::allocationQuantum; --i != 0;) {
            node->next = free;
            free = node++;
        }
        freeNodeList = free;
    } else {
        freeNodeList = node->next;
    }
    return node;
}

void dbExprNodeAllocator::deallocate(dbExprNode* node)
{
    if (node != NULL) { 
        node->next = freeNodeList;
        freeNodeList = node;
    }
}

void dbExprNodeAllocator::reset()
{
    dbExprNode* free = NULL;
    for (dbExprNodeSegment* seg = segmentList; seg != NULL; seg = seg->next)
    {
        dbExprNode* node = (dbExprNode*)seg->buf;
        for (int i = dbExprNodeSegment::allocationQuantum; --i >= 0;) {
            node->next = free;
            free = node++;
         }
    }
    freeNodeList = free;
}

dbExprNode::dbExprNode(dbExprNode* node)
{
    memcpy(this, node, sizeof(*this));
    
    for (int i = nodeOperands[cop]; --i >= 0;) { 
        operand[i] = new dbExprNode(operand[i]);
    }
    if (cop == dbvmLoadStringConstant) { 
        char* s = new char[strlen(svalue.str)+1];
        strcpy(s, svalue.str);
        svalue.str = s;
    }
}

dbExprNode::~dbExprNode()
{
    if (cop == dbvmLoadStringConstant) { 
        delete[] svalue.str;
    } else if (cop == dbvmLoadWStringConstant) { 
        delete[] wsvalue.str;
#ifdef USE_REGEX
    } else if (cop == dbvmMatchString) { 
        delete regex.opd;
        regfree(&regex.re);
#endif
    } else { 
        for (int i = nodeOperands[cop]; --i >= 0; ) { 
            delete operand[i];
        }
    }
}

int dbCompiler::scan() 
{
    char ch;
    char *p, *q;
    int  i, n, value;
    char buf[maxSymbolLen+1];

    if (hasToken) { 
        hasToken = false;
        return lex;
    }
   nextElement:
    if (queryElement == NULL) { 
        return tkn_eof;
    }
    if (queryElement->type != dbQueryElement::qExpression) {
        varType = queryElement->type;
        varPtr = queryElement->ptr;
        varRefTable = queryElement->ref;
        if (varRefTable != NULL) { 
            varRefTable = table->db->lookupTable(varRefTable);
        }
        queryElement = queryElement->next;
        return tkn_var;
    } 
    p = q = (char*)queryElement->ptr + currPos;
    
    do { 
        ch = *p++;
        if (ch == '\n') { 
            offsetWithinStatement = (int)((char*)queryElement->ptr - p);
            firstPos = 0;
        }
    } while (isspace(byte(ch)));
    currPos += (int)(p - q);

    switch (ch) { 
      case '\0':
        if ((queryElement = queryElement->next) == NULL) { 
            return tkn_eof;
        }
        currPos = 0;
        goto nextElement;
      case '+':
        return tkn_add;
      case '-':
        if (*p == '-') { 
            // ANSI comments
            q = p;
            do { 
                p += 1;
            } while (*p != '\n' && *p != '\0');
            currPos +=(int)(p - q);
            goto nextElement;
        }
        return tkn_sub;
      case '*':
        return tkn_mul;
      case '/':
        return tkn_div;
      case '.':
        return tkn_dot;
      case ',':
        return tkn_comma;
      case '(':
        return tkn_lpar;
      case ')':
        return tkn_rpar;
      case '[':
        return tkn_lbr;
      case ']':
        return tkn_rbr;
      case ':':
        return tkn_col;
      case '^':
        return tkn_power;
      case '<':
        if (*p == '=') { 
            currPos += 1;
            return tkn_le;
        } else if (*p == '>') { 
            currPos += 1;
            return tkn_ne;
        }
        return tkn_lt;
      case '>':
        if (*p == '=') { 
            currPos += 1;
            return tkn_ge;
        }
        return tkn_gt;
      case '=':
        return tkn_eq;
      case '!':
        if (*p != '=') { 
            error("Invalid token '!'");
        } 
        currPos += 1;
        return tkn_ne;
      case '|':
        if (*p != '|') { 
            error("Invalid token '|'");
        } 
        currPos += 1;
        return tkn_add;
      case '\'':
      {
          char* strbuf = buf;
          int bufSize = maxSymbolLen;
          q = p;
          i = 0; 
          while (true) { 
              if (*p == '\'') { 
                  if (*++p != '\'') { 
                      break;
                  }
              } else if (*p == '\0') { 
                  error("Unterminated character constant");
              }
              if (i+1 == bufSize) { 
                  char* newbuf = new char[(bufSize *= 2) + 1];
                  memcpy(newbuf, strbuf, i);
                  if (strbuf != buf) { 
                      delete[] strbuf;
                  }
                  strbuf = newbuf;
              }
              strbuf[i++] = *p++;
          }
          strbuf[i++] = '\0';
          currPos += (int)(p - q);
          svalue.str = new char[i];
          memcpy(svalue.str, strbuf, i);
          svalue.len = i;
          if (strbuf != buf) { 
              delete[] strbuf;
          }
          return tkn_sconst;
      }
      case '#':
        ivalue = 0;
        q = p;
        while (true) { 
            ch = *p++;
            if (ch >= '0' && ch <= '9') { 
                ivalue = (ivalue << 4) + ch-'0';
            } else if (ch >= 'a' && ch <= 'f') {
                ivalue = (ivalue << 4) + ch-'a'+10;
            } else if (ch >= 'A' && ch <= 'F') {
                ivalue = (ivalue << 4) + ch-'A'+10;
            } else { 
                currPos += (int)(p - q - 1); 
                return tkn_iconst;
            }
        }
      case '0': case '1': case '2': case '3': case '4': 
      case '5': case '6': case '7': case '8': case '9':
        value = ch - '0';
        for (q = p; isdigit(*(byte*)p); p++) { 
            value = value*10 + *p - '0';
        }
        if (*p == '.' || *p == 'e' || *p == 'E') { 
            if (sscanf(q-1, "%lf%n", &fvalue, &n) != 1) { 
                error("Bad floating point constant");
            }
            currPos += n - 1;
            return tkn_fconst;
        } else if (p - q >= 9) { 
            if (sscanf(q-1, INT8_FORMAT "%n", &ivalue, &n) != 1) { 
                error("Bad integer constant");
            }
            currPos += n - 1;
            return tkn_iconst;
        } else { 
            currPos += (int)(p - q); 
            ivalue = value;
            return tkn_iconst;
        }
      default:
        if (isalpha(ch) || ch == '$' || ch == '_') { 
            i = 0;
            do { 
                if (i == maxSymbolLen) { 
                    error("Name too long");
                }
                buf[i++] = ch;
                ch = *p++;
            } while (isalnum(ch) || ch == '$' || ch == '_');
            buf[i] = '\0';
            name = buf;
            currPos += i - 1;
        } else { 
            error("Invalid symbol");
        } 
        return dbSymbolTable::add(name, tkn_ident);
   }
}

dbExprNode* dbCompiler::disjunction()
{
    dbExprNode* left = conjunction();
    if (lex == tkn_or) { 
        int pos = currPos;
        dbExprNode* right = disjunction();
        if (left->type == tpInteger && right->type == tpInteger) { 
            left = new dbExprNode(dbvmOrInt, left, right);
        } else if (left->type == tpBoolean && right->type == tpBoolean) { 
            left = new dbExprNode(dbvmOrBool, left, right);
        } else { 
            error("Bad operands for OR operator", pos);
        }
    }
    return left;
}

dbExprNode* dbCompiler::conjunction()
{
    dbExprNode* left = comparison();
    if (lex == tkn_and) { 
        int pos = currPos;
        dbExprNode* right = conjunction();
        if (left->type == tpInteger && right->type == tpInteger) { 
            left = new dbExprNode(dbvmAndInt, left, right);
        } else if (left->type == tpBoolean && right->type == tpBoolean) { 
            left = new dbExprNode(dbvmAndBool, left, right);
        } else { 
            error("Bad operands for AND operator", pos);
        }
    }
    return left;
}

inline dbExprNode* int2real(dbExprNode* expr) 
{
    if (expr->cop == dbvmLoadIntConstant) { 
        expr->fvalue = (real8)expr->ivalue;
        expr->cop = dbvmLoadRealConstant;
        expr->type = tpReal;
    } else { 
        expr = new dbExprNode(dbvmIntToReal, expr);
    }
    return expr;
}

inline dbExprNode* mbs2wcs(dbExprNode* expr) 
{
    if (expr->cop == dbvmLoadStringConstant) { 
        size_t len = mbstowcs(NULL, expr->svalue.str, 0);
        wchar_t* wstr = new wchar_t[len+1];
        mbstowcs(wstr, expr->svalue.str, len);
        delete[] expr->svalue.str;
        expr->wsvalue.str = wstr;
        expr->wsvalue.len = (int)len+1;
        wstr[len] = 0;
        expr->cop = dbvmLoadWStringConstant;
        expr->type = tpWString;
    } else if (expr->type == tpString) { 
        expr = new dbExprNode(dbvmMbsToWcs, expr);
    } else if (expr->type != tpWString) { 
        return NULL;
    }
    return expr;
}

void dbCompiler::compare(dbExprNode* expr, dbExprNode* list)
{
    dbExprNode* selector = expr;
    for (int n = 1; list != NULL; n++) { 
        dbExprNode* elem = list->operand[0];
        dbExprNode* tail = list->operand[1];
        int cop = dbvmVoid;
        int rawBinarySize = 0;
        void* rawBinaryComparator = NULL;
        if (expr->type == tpInteger) { 
            if (elem->type == tpReal) { 
                expr = int2real(expr);
                cop = dbvmEqReal;
            } else if (elem->type == tpReference) {
                expr = new dbExprNode(dbvmIntToReference, expr);
                cop = dbvmEqReference;
            } else if (elem->type == tpInteger) { 
                cop = dbvmEqInt;
            }
        } else if (expr->type == tpReal) {
            if (elem->type == tpReal) { 
                cop = dbvmEqReal;
            } else if (elem->type == tpInteger) { 
                cop = dbvmEqReal;
                elem = int2real(elem);
            }
        } else if (expr->type == tpWString) { 
            if (elem->type == tpWString) {
                cop = dbvmEqWString;
            } else if (elem->type == tpString) {
                cop = dbvmEqWString;
                elem = mbs2wcs(elem);
            }
        } else if (elem->type == tpWString) { 
            if (expr->type == tpString) {
                cop = dbvmEqWString;
                expr = mbs2wcs(expr);
            }
        } else if (expr->type == tpString && elem->type == tpString) {
            cop = dbvmEqString;
        } else if (expr->type == tpReference && (elem->type == tpReference || elem->type == tpInteger)) {
            if (elem->type == tpInteger) {
                elem = new dbExprNode(dbvmIntToReference, elem);
            }
            cop = dbvmEqReference;
        } else if (expr->type == tpBoolean && elem->type == tpBoolean) {
            cop = dbvmEqBool;
        } else if (expr->type == tpRawBinary && elem->type == tpRawBinary) {
            cop = dbvmEqBinary;
            if ((expr->cop == dbvmLoadSelfRawBinary || expr->cop == dbvmLoadRawBinary) 
                && expr->ref.field != NULL)
            {
                rawBinarySize = (int)expr->ref.field->dbsSize;
                rawBinaryComparator = (void*)expr->ref.field->_comparator;
            } else if ((elem->cop == dbvmLoadSelfRawBinary || elem->cop == dbvmLoadRawBinary) 
                       && elem->ref.field != NULL)
            {
                rawBinarySize = (int)elem->ref.field->dbsSize;
                rawBinaryComparator = (void*)elem->ref.field->_comparator;
            } else { 
                error("Operations with raw binary types should include at least one record field");
            }
        }
        if (cop == dbvmVoid) { 
            char buf[256];
            sprintf(buf, "Expression %d in right part of IN operator have "
                    "incompatible type", n);
            error(buf);
        } else { 
            list->type = tpBoolean;
            if (tail != NULL) { 
                expr = new dbExprNode(cop, expr, elem, rawBinarySize);
                expr->func.fptr = rawBinaryComparator;
                list->operand[0] = expr;
                list->cop = dbvmOrBool;
                expr = new dbExprNode(selector);
            } else { 
                list->operand[1] = elem;
                list->operand[0] = expr;
                list->cop = cop;
                list->offs = rawBinarySize;
                list->func.fptr = rawBinaryComparator;
            }
        }
        list = tail;
    }
}

dbExprNode* dbCompiler::rectangleConstant(dbExprNode* head)
{
    rectangle r;
    for (int i = 0; i < RECTANGLE_DIMENSION*2; i++) { 
        if (head == NULL || head->operand[0] == NULL) { 
            error("Bad rectangle constant");
        }
        dbExprNode* elem = head->operand[0];
        dbExprNode* tail = head->operand[1];
        if (elem->type == tpReal) {                         
            r.boundary[i] = (coord_t)elem->fvalue;
        } else if (elem->type == tpInteger) {  
            r.boundary[i] = (coord_t)elem->ivalue;
        } else { 
            error("Bad rectangle constant");
        }
        head = tail;
    }
    if (head != NULL) { 
        error("Bad rectangle constant");
    }
    return new dbExprNode(dbvmLoadRectangleConstant, r);
}


dbExprNode* dbCompiler::comparison()
{
    int leftPos = currPos;
    dbExprNode *left, *right;
    left = addition();
    int cop = lex;
    if (cop == tkn_eq || cop == tkn_ne || cop == tkn_gt || cop == tkn_ge
        || cop == tkn_le || cop == tkn_lt || cop == tkn_between || cop == tkn_overlaps
        || cop == tkn_like || cop == tkn_match || cop == tkn_not || cop == tkn_is || cop == tkn_in)
    {
        int rightPos = currPos;
        bool notOp = false;
        if (cop == tkn_not) { 
            notOp = true;
            cop = scan();
            if (cop != tkn_like && cop != tkn_match && cop != tkn_between && cop != tkn_in) { 
                error("LIKE, MATCH, BETWEEN or IN expected", rightPos);
            } 
            rightPos = currPos;
        } else if (cop == tkn_is) {
            if (left->type != tpReference) { 
                error("IS [NOT] NULL predicate can be applied only to "
                      "references", rightPos);
            } 
            rightPos = currPos;
            if ((cop = scan()) == tkn_null) { 
                left = new dbExprNode(dbvmIsNull, left);
            } else if (cop == tkn_not) { 
                rightPos = currPos;
                if (scan() == tkn_null) { 
                    left = new dbExprNode(dbvmNotBool, 
                                          new dbExprNode(dbvmIsNull, left));
                } else { 
                    error("NULL expected", rightPos);
                }
            } else { 
                error("[NOT] NULL expected", rightPos);
            } 
            lex = scan();
            return left;
        }       
        right = addition();
        if (cop == tkn_in) { 
            int type;
            switch (right->type) {
              case tpArray:
                switch (right->cop) { 
                  case dbvmLoadVarArrayOfOid:
                  case dbvmLoadVarArrayOfOidPtr:
                    type = dbField::tpReference;
                    break;
                  case dbvmLoadVarArrayOfInt4:
                  case dbvmLoadVarArrayOfInt4Ptr:
                    type = dbField::tpInt4;
                    break;
                  case dbvmLoadVarArrayOfInt8:
                  case dbvmLoadVarArrayOfInt8Ptr:
                    type = dbField::tpInt8;
                    break;
                  default:
                    type = right->ref.field->components->type;
                }
                if ((left->type == tpBoolean && type == dbField::tpBool) 
                    || (left->type == tpInteger 
                        && (type == dbField::tpInt1 
                            || type == dbField::tpInt2
                            || type == dbField::tpInt4 
                            || type == dbField::tpInt8))
                    || (left->type == tpReal 
                        && (type == dbField::tpReal4
                            || type == dbField::tpReal8))
                    || ((left->type == tpString || left->type == tpWString) 
                        && (type == dbField::tpString || type == dbField::tpWString))
                    || ((left->type == tpRectangle || left->type == tpList) && type == dbField::tpRectangle)
                    || ((left->type == tpReference || left->type == tpInteger)
                        && type == dbField::tpReference))
                {
                    if (left->type == tpInteger && type == dbField::tpReference) { 
                        left = new dbExprNode(dbvmIntToReference, left);
                    }
                    if (type == dbField::tpRectangle) { 
                        if (left->type == tpList) { 
                            left = rectangleConstant(left);
                        }
                        left = new dbExprNode(dbvmInArrayRectangle, left, right);
                    } else if (left->type == tpWString || type == dbField::tpWString) { 
                        left = new dbExprNode(dbvmInArrayWString, mbs2wcs(left), mbs2wcs(right));
                    } else { 
                        left = new dbExprNode(dbvmInArrayBool + type, left, right);
                    }
                } else {
                    error("Type of selective expression of IN operator doesn't"
                          " match type of the array");
                }
                break;
              case tpWString:
                if (left->type == tpWString) { 
                    left = new dbExprNode(dbvmInWString, left, right);
                } else if (left->type == tpString) { 
                    left = new dbExprNode(dbvmInWString, mbs2wcs(left), right);
                } else { 
                    error("Left operand of IN expression hasn't string type");
                }
                break;
              case tpString:
                if (left->type == tpString) { 
                    left = new dbExprNode(dbvmInString, left, right);
                } else if (left->type == tpWString) { 
                    left = new dbExprNode(dbvmInWString, left, mbs2wcs(right));
                } else { 
                    error("Left operand of IN expression hasn't string type");
                }
                break;
              case tpRectangle:
                if (left->type == tpRectangle) {
                    if (IS_CONSTANT(left->cop)) {
                        left = new dbExprNode(dbvmGeRectangle, right, left);
                    } else { 
                        left = new dbExprNode(dbvmLeRectangle, left, right);
                    }
                } else if (left->type == tpList) {
                    left = new dbExprNode(dbvmGeRectangle, right, rectangleConstant(left));
                } else {
                    error("Left operand of IN expression hasn't rectangle type");
                }
                break;          
              case tpList:
                if (left->type == tpRectangle) {
                    left = new dbExprNode(dbvmLeRectangle, left, rectangleConstant(right));
                } else {
                    if (right->operand[0] == NULL) { // emtry list
                        left = new dbExprNode(dbvmLoadFalse);
                    } else { 
                        compare(left, right);
                        left = right;
                    }
                }
                break;
              default:
                error("List of expressions or array expected", rightPos);
            }
        } else if (cop == tkn_between) { 
            int andPos = currPos;
            if (lex != tkn_and) { 
                error("AND expected");
            }
            dbExprNode* right2 = addition();
            if (left->type == tpReal 
                || right->type == tpReal || right2->type == tpReal)
            {
                if (left->type == tpInteger) { 
                    left = int2real(left);
                } else if (left->type != tpReal) { 
                    error("operand of BETWEEN operator should be of "
                          "integer, real or string type", leftPos);
                }
                if (right->type == tpInteger) {
                    right = int2real(right);
                } else if (right->type != tpReal) { 
                    error("operand of BETWEEN operator should be of "
                          "integer, real or string type", rightPos);
                }
                if (right2->type == tpInteger) {
                    right2 = int2real(right2);
                } else if (right2->type != tpReal) { 
                    error("operand of BETWEEN operator should be of "
                          "integer, real or string type", andPos);
                }
                left = new dbExprNode(dbvmBetweenReal, left, right, right2);
            } 
            else if (left->type == tpInteger && right->type == tpInteger 
                     && right2->type == tpInteger)
            {               
                left = new dbExprNode(dbvmBetweenInt, left, right, right2);
            }
            else if (left->type == tpArray && right->type == tpArray 
                     && right2->type == tpArray)
            {               
                void* arrayComparator = NULL;
                if ((left->cop == dbvmLoadSelfArray || left->cop == dbvmLoadArray) 
                    && left->ref.field != NULL)
                {
                    arrayComparator = (void*)left->ref.field->_comparator;
                } else { 
                    error("Operations with array should include at least one record field");
                }
                left = new dbExprNode(dbvmBetweenArray, left, right, right2);
                left->func.fptr = arrayComparator;
            }
            else if (left->type == tpWString || right->type == tpWString 
                     || right2->type == tpWString)
            {
                left = mbs2wcs(left);
                right = mbs2wcs(right);
                right2 = mbs2wcs(right2);
                if (left == NULL || right == NULL || right2 == NULL) { 
                   error("operand of BETWEEN operator should be of integer, real or string type", andPos);
                }
                left = new dbExprNode(dbvmBetweenWString, left, right, right2);
            }
            else if (left->type == tpString && right->type == tpString 
                     && right2->type == tpString)
            {
                left = new dbExprNode(dbvmBetweenString, left, right, right2);
            }
            else if (left->type == tpRawBinary && right->type == tpRawBinary
                     && right2->type == tpRawBinary)
            {
                int rawBinarySize = 0;
                void* rawBinaryComparator = NULL;
                if ((left->cop == dbvmLoadSelfRawBinary || left->cop == dbvmLoadRawBinary) 
                    && left->ref.field != NULL)
                {
                    rawBinarySize = (int)left->ref.field->dbsSize;
                    rawBinaryComparator = (void*)left->ref.field->_comparator;
                } else { 
                    error("Operations with raw binary types should include at least one record field");
                }
                left = new dbExprNode(dbvmBetweenBinary, left, right, right2);
                left->offs = rawBinarySize;
                left->func.fptr = rawBinaryComparator;
            }
            else { 
                error("operands of BETWEEN operator should be of "
                      "integer, real or string type", rightPos);
            }
#ifdef USE_REGEX
        } else if (cop == tkn_match) {
            if (left->type != tpString) {
                error("left operands of MATCH operator should be of string type", leftPos);
            }
            if (right->cop != dbvmLoadStringConstant) { 
                error("right operand of MATCH operator should be string constant", rightPos);
            }
            dbExprNode* expr = new dbExprNode(dbvmMatchString);
            expr->regex.opd = left;
            left = expr;
            if (regcomp(&left->regex.re, (char*)right->svalue.str, REG_EXTENDED|REG_ICASE|REG_NOSUB) != 0) { 
                error("invalid regular expression", rightPos);
            }
#endif
        } else if (cop == tkn_like) {  
            if ((left->type != tpString && left->type != tpWString) || (right->type != tpString && right->type != tpWString)) { 
                error("operands of LIKE operator should be of "
                      "string type", rightPos);
            }
            if (lex == tkn_escape) { 
                rightPos = currPos;
                if (scan() != tkn_sconst) { 
                    error("String literal espected after ESCAPE", rightPos);
                }
                lex = scan();
                if (left->type == tpWString || right->type == tpWString) { 
                    left = new dbExprNode(dbvmLikeEscapeWString, mbs2wcs(left), mbs2wcs(right),
                                          mbs2wcs(new dbExprNode(dbvmLoadStringConstant, svalue)));
                } else { 
                    left = new dbExprNode(dbvmLikeEscapeString, left, right,
                                          new dbExprNode(dbvmLoadStringConstant, svalue));
                }
            } else { 
                if (right->cop == dbvmLoadStringConstant) {
                    char* str = right->svalue.str;
                    char* w = findWildcard(str);
                    if (w == NULL) { 
                        if (left->type == tpWString) { 
                            left = new dbExprNode(dbvmEqWString, left, mbs2wcs(right));
                        } else { 
                            left = new dbExprNode(dbvmEqString, left, right);
                        }
                    } else if (w == str && *w == '%' 
                               && (w = findWildcard(str+1)) != NULL && *w == '%' && *(w+1) == '\0') 
                    {
                        memcpy(str, str+1, w-str-1);
                        str[w-str-1] = '\0';
                        right->svalue.len -= 2;
                        if (left->type == tpWString) { 
                            left = new dbExprNode(dbvmInWString, mbs2wcs(right), left);
                        } else { 
                            left = new dbExprNode(dbvmInString, right, left);
                        }
                    } else {
                        if (left->type == tpWString) { 
                            left = new dbExprNode(dbvmLikeWString, left, mbs2wcs(right));
                        } else { 
                            left = new dbExprNode(dbvmLikeString, left, right);
                        }
                    }
                } else if (right->cop == dbvmLoadWStringConstant) {
                    wchar_t* str = right->wsvalue.str;
                    wchar_t* w = findWildcard(str);
                    if (w == NULL) { 
                        left = new dbExprNode(dbvmEqWString, mbs2wcs(left), right);
                    } else if (w == str && *w == '%' 
                               && (w = findWildcard(str+1)) != NULL && *w == '%' && *(w+1) == '\0') 
                    {
                        memcpy(str, str+1, (w-str-1)*sizeof(wchar_t));
                        str[w-str-1] = '\0';
                        right->wsvalue.len -= 2;
                        left = new dbExprNode(dbvmInWString, right, mbs2wcs(left));
                    } else {
                        left = new dbExprNode(dbvmLikeWString, mbs2wcs(left), right);
                    }
                } else if (left->type == tpWString || right->type == tpWString) { 
                    left = new dbExprNode(dbvmLikeWString, mbs2wcs(left), mbs2wcs(right));
                } else {
                    left = new dbExprNode(dbvmLikeString, left, right);
                }
            }
        } else if (cop == tkn_overlaps) {
            if (left->type == tpRectangle && right->type == tpRectangle) {
                if (IS_CONSTANT(left->cop)) {
                    left = new dbExprNode(dbvmOverlapsRectangle, right, left);
                } else { 
                    left = new dbExprNode(dbvmOverlapsRectangle, left, right);
                }
            } else if (left->type == tpList && right->type == tpRectangle) { 
                left = new dbExprNode(dbvmOverlapsRectangle, right, rectangleConstant(left));
            } else if (left->type == tpRectangle && right->type == tpList) { 
                left = new dbExprNode(dbvmOverlapsRectangle, left, rectangleConstant(right));
            } else { 
                error("operands of OVERLAPS operator should be of rectangle type", leftPos);
            }       
        } else { 
            if (left->type == tpReal || right->type == tpReal) { 
                if (left->type == tpInteger) { 
                    left = int2real(left);
                } else if (left->type != tpReal) { 
                    error("operands of relation operator should be of "
                          "intger, real or string type", leftPos);
                }
                if (right->type == tpInteger) { 
                    right = int2real(right);
                } else if (right->type != tpReal) { 
                    error("operands of relation operator should be of "
                          "intger, real or string type", rightPos);
                }
                left = new dbExprNode(dbvmEqReal + cop - tkn_eq, left, right);
            } else if (left->type == tpInteger && right->type == tpInteger) { 
                left = new dbExprNode(dbvmEqInt + cop - tkn_eq, left, right);
            } else if (left->type == tpWString || right->type == tpWString) { 
                left = mbs2wcs(left);
                right = mbs2wcs(right);
                if (left == NULL || right == NULL) { 
                    error("operands of relation operator should be of intger, real or string type", rightPos);
                }
                left = new dbExprNode(dbvmEqWString + cop-tkn_eq, left, right);
            } else if (left->type == tpString && right->type == tpString) { 
                left = new dbExprNode(dbvmEqString + cop-tkn_eq, left, right);
            } else if (left->type == tpRectangle && right->type == tpRectangle) {
                left = new dbExprNode(dbvmEqRectangle + cop-tkn_eq, left, right);
            } else if (left->type == tpList && right->type == tpRectangle) { 
                left = new dbExprNode(dbvmEqRectangle + cop-tkn_eq, rectangleConstant(left), right);
            } else if (left->type == tpRectangle && right->type == tpList) { 
                left = new dbExprNode(dbvmEqRectangle + cop-tkn_eq, left, rectangleConstant(right));
            } else if (left->type == tpArray && right->type == tpArray) { 
                void* arrayComparator = NULL;
                if ((left->cop == dbvmLoadSelfArray || left->cop == dbvmLoadArray) 
                    && left->ref.field != NULL)
                {
                    arrayComparator = (void*)left->ref.field->_comparator;
                } else if ((right->cop == dbvmLoadSelfArray || right->cop == dbvmLoadArray) 
                    && right->ref.field != NULL)
                {
                    arrayComparator = (void*)right->ref.field->_comparator;
                } else { 
                    error("Operations with array should include at least one record field");
                }
                left = new dbExprNode(dbvmEqArray + cop-tkn_eq, left, right);
                left->func.fptr = arrayComparator;
            } else if ((left->type == tpReference || left->type == tpInteger) 
                       && (right->type == tpReference || right->type == tpInteger)) 
            {
                if (cop != tkn_eq && cop != tkn_ne) {
                    error("References can be checked only for equality",
                          rightPos);
                }
                if (left->type == tpInteger) { 
                    left = new dbExprNode(dbvmIntToReference, left);
                } else if (right->type == tpInteger) { 
                    right = new dbExprNode(dbvmIntToReference, right);
                }
                left = new dbExprNode(dbvmEqReference+cop-tkn_eq, left, right);
            } else if (left->type == tpBoolean && right->type == tpBoolean) { 
                if (cop != tkn_eq && cop != tkn_ne) { 
                    error("Boolean variables can be checked only for equality",
                          rightPos);
                }
                left = new dbExprNode(dbvmEqBool + cop - tkn_eq, left, right);
            } else if (left->type == tpRawBinary && right->type == tpRawBinary) {
                int rawBinarySize = 0;
                void* rawBinaryComparator = NULL;
                if ((left->cop == dbvmLoadSelfRawBinary || left->cop == dbvmLoadRawBinary) 
                    && left->ref.field != NULL)
                {
                    rawBinarySize = (int)left->ref.field->dbsSize;
                    rawBinaryComparator = (void*)left->ref.field->_comparator;
                } else if ((right->cop == dbvmLoadSelfRawBinary || right->cop == dbvmLoadRawBinary) 
                           && right->ref.field != NULL)
                {
                    rawBinarySize = (int)right->ref.field->dbsSize;
                    rawBinaryComparator = (void*)right->ref.field->_comparator;
                } else { 
                    error("Operations with raw binary types should include at least one record field");
                }
                left = new dbExprNode(dbvmEqBinary + cop - tkn_eq, left, right, rawBinarySize);
                left->func.fptr = rawBinaryComparator;
            } else { 
                error("operands of relation operator should be of "
                      "integer, real or string type", rightPos);
            }
            //
            // Optimization for applying indices: if operation is 
            // commuatative and left operand is constant then swap operands
            //
            if (IS_CONSTANT(left->operand[0]->cop)) { 
                right = left->operand[1];
                left->operand[1] = left->operand[0];
                left->operand[0] = right;
                left->cop = dbExprNode::commutativeOperator[left->cop];
            }   
        }
        if (notOp) { 
            left = new dbExprNode(dbvmNotBool, left);
        }
    }
    return left;
}

dbExprNode* dbCompiler::addition() 
{ 
    int leftPos = currPos;
    dbExprNode* left = multiplication();
    while (lex == tkn_add || lex == tkn_sub) { 
        int cop = lex;
        int rightPos = currPos;
        dbExprNode* right = multiplication();
        if (left->type == tpReal || right->type == tpReal) { 
            if (left->type == tpInteger) { 
                left = int2real(left);
            } else if (left->type != tpReal) { 
                error("operands of arithmetic operators should be of "
                      "integer or real type", leftPos);
            }
            if (right->type == tpInteger) { 
                right = int2real(right);
            } else if (right->type != tpReal) { 
                error("operands of arithmetic operator should be of "
                      "integer or real type", rightPos);
            }
            left = new dbExprNode(cop==tkn_add ? dbvmAddReal : dbvmSubReal,
                                  left, right);
        } else if (left->type == tpInteger && right->type == tpInteger) { 
            left = new dbExprNode(cop==tkn_add ? dbvmAddInt : dbvmSubInt,
                                  left, right);
        } else if (left->type == tpRectangle || right->type == tpRectangle) {
            if (cop == tkn_add) { 
                if (left->type == tpRectangle || right->type == tpRectangle) {
                    left = new dbExprNode(dbvmAddRectangle, left, right);
                } else if (left->type == tpRectangle || right->type == tpList) {
                    left = new dbExprNode(dbvmAddRectangle, left, rectangleConstant(right));
                } else if (right->type == tpRectangle || left->type == tpList) {
                    left = new dbExprNode(dbvmAddRectangle, rectangleConstant(left), right);
                } else {
                    error("Rectangle can be added only with rectangle", rightPos);
                } 
            } else {
                error("Operation - is not defined for rectangles", rightPos);
            }    
        } else if (left->type == tpWString || right->type == tpWString) { 
            if (cop == tkn_add) { 
                left = mbs2wcs(left);
                right = mbs2wcs(right);
                if (left == NULL || right == NULL) { 
                     error("Operands if concatenation operator should have string type", rightPos);
                }
                left = new dbExprNode(dbvmWStringConcat, left, right);
            } else { 
                error("Operation - is not defined for strings", rightPos);
            }
        } else if (left->type == tpString && right->type == tpString) { 
            if (cop == tkn_add) { 
                left = new dbExprNode(dbvmStringConcat, left, right);
            } else { 
                error("Operation - is not defined for strings", rightPos);
            }
        } else { 
            error("operands of arithmentic operator should be of "
                  "integer or real type", rightPos);
        }
        leftPos = rightPos;
    }
    return left;
}


dbExprNode* dbCompiler::multiplication() 
{ 
    int leftPos = currPos;
    dbExprNode* left = power();
    while (lex == tkn_mul || lex == tkn_div) { 
        int cop = lex;
        int rightPos = currPos;
        dbExprNode* right = power();
        if (left->type == tpReal || right->type == tpReal) { 
            if (left->type == tpInteger) { 
                left = int2real(left);
            } else if (left->type != tpReal) { 
                error("operands of arithmetic operators should be of "
                      "integer or real type", leftPos);
            }
            if (right->type == tpInteger) { 
                right = int2real(right);
            } else if (right->type != tpReal) { 
                    error("operands of arithmetic operator should be of "
                          "integer or real type", rightPos);
            }
            left = new dbExprNode(cop==tkn_mul ? dbvmMulReal : dbvmDivReal,
                                  left, right);
        } else if (left->type == tpInteger && right->type == tpInteger) { 
            left = new dbExprNode(cop==tkn_mul ? dbvmMulInt : dbvmDivInt,
                                  left, right);
        } else { 
            error("operands of arithmentic operator should be of "
                  "integer or real type", rightPos);
        }
        leftPos = rightPos;
    }
    return left;
}

dbExprNode* dbCompiler::power() 
{ 
    int leftPos = currPos;
    dbExprNode* left = userDefinedOperator();
    if (lex == tkn_power) { 
        int rightPos = currPos;
        dbExprNode* right = power();
        if (left->type == tpReal || right->type == tpReal) { 
            int cop = dbvmPowerReal;
            if (left->type == tpInteger) { 
                left = int2real(left);
            } else if (left->type != tpReal) { 
                error("operands of arithmetic operators should be of "
                      "integer or real type", leftPos);
            }
            if (right->type == tpInteger) { 
                cop = dbvmPowerRealInt;
            } else if (right->type != tpReal) { 
                    error("operands of arithmetic operator should be of "
                          "integer or real type", rightPos);
            }
            left = new dbExprNode(cop, left, right);
        } else if (left->type == tpInteger && right->type == tpInteger) { 
            left = new dbExprNode(dbvmPowerInt, left, right);
        } else { 
            error("operands of arithmentic operator should be of "
                  "integer or real type", rightPos);
        }
    }    
    return left;
}


dbExprNode* dbCompiler::userDefinedOperator() 
{ 
    dbExprNode* left = term();
    while (lex == tkn_ident) { 
        dbUserFunction* func = dbUserFunction::find(name);
        if (func != NULL) { 
            int nParams =  func->getNumberOfParameters();
            if (nParams != 2) { 
                error("Only function with two arguments can be used as operator", currPos);
            }
            int rightPos = currPos;
            dbExprNode* right = term();
            if ((left->type != tpInteger && left->type != tpReal && left->type != tpString && left->type != tpWString
                 && left->type != tpReference && left->type != tpRawBinary && left->type != tpBoolean)
                || (right->type != tpInteger && right->type != tpReal && right->type != tpString && right->type != tpWString
                    && right->type != tpReference && right->type != tpRawBinary && right->type != tpBoolean))
            {
                
                error("User function should receive parameter of boolean, integer, real, string, reference or user defined type", rightPos);
            } 
            left = new dbExprNode(dbvmFuncInt2Bool + func->type, func->fptr, left, right);
        } else { 
            break;
        }
    }
    return left;
}

dbExprNode* dbCompiler::field(dbExprNode* expr, dbTableDescriptor* refTable,
                              dbFieldDescriptor* fd) 
{
    int pos;

    while (true) { 
        switch (lex) {
          case tkn_dot: 
            pos = currPos;
            if (scan() != tkn_ident) { 
                error("identifier expected", pos);
            }
            if (fd != NULL && fd->type == dbField::tpStructure) { 
                if ((fd = fd->findSymbol(name)) == NULL) { 
                    error("Field not found");
                }
            } else { 
                assert(expr != NULL);
                if (expr->type != tpReference) { 
                    error("Left operand of '.' should be "
                          "structure or reference", pos);
                }
                if (refTable == NULL && fd != NULL) { 
                    refTable = fd->refTable;
                }
                if (refTable == NULL || (fd = refTable->findSymbol(name)) == NULL) { 
                    error("Field not found");
                }
                refTable = NULL;
                expr = new dbExprNode(dbvmDeref, expr);
            }
            break;
          case tkn_lbr:
            if (expr == NULL || 
                (expr->type != tpArray && expr->type != tpString && expr->type != tpWString && expr->type != tpRectangle))
            { 
                error("Index can be applied only to arrays");
            } else { 
                dbExprNode* index = disjunction();
                if (lex != tkn_rbr) { 
                    error("']' expected");
                }
                if (index->type != tpInteger && index->type != tpFreeVar) {
                    error("Index should have integer type");
                }
                
                if (expr->type == tpString) { 
                    lex = scan();
                    return new dbExprNode(dbvmCharAt, expr, index);
                } else if (expr->type == tpWString) { 
                    lex = scan();
                    return new dbExprNode(dbvmWCharAt, expr, index);
                } if (expr->type == tpRectangle) {
                    lex = scan();
                    return new dbExprNode(dbvmRectangleCoord, expr, index);
                }
                if (fd == NULL) { 
                    // variable of array of reference type
                    expr = new dbExprNode(dbvmGetAt,expr,index,sizeof(oid_t));
                } else { 
                    if (refTable == NULL) { 
                        refTable = fd->refTable;
                    }
                    fd = fd->components;
                    expr = new dbExprNode(dbvmGetAt, expr, index, (int)fd->dbsSize);
                }
            }
            break;
          default:
            if (expr == NULL) { 
                error("'.' expected");
            }
            return expr;
        }       
        if (fd == NULL) { 
            expr = new dbExprNode(dbvmLoadReference, expr, 0); 
        } else if (fd->type == dbField::tpRawBinary) {
            expr = new dbExprNode(expr != NULL ? dbvmLoadRawBinary : dbvmLoadSelfRawBinary, 
                                  fd, expr);
        } else if (fd->type == dbField::tpRectangle) {
            expr = new dbExprNode(expr != NULL ? dbvmLoadRectangle: dbvmLoadSelfRectangle, 
                                  fd, expr);
        } else if (fd->type == dbField::tpWString) {
            expr = new dbExprNode(expr != NULL ? dbvmLoadWString : dbvmLoadSelfWString, 
                                  fd, expr);
        } else if (fd->type == dbField::tpMethodWString) {
            expr = new dbExprNode(expr != NULL ? dbvmInvokeMethodWString : dbvmInvokeSelfMethodWString,  fd, expr);
        } else if ((unsigned)fd->type - dbField::tpMethodBool <= dbField::tpMethodReference - dbField::tpMethodBool) {
            expr = new dbExprNode((expr != NULL ? dbvmInvokeMethodBool : dbvmInvokeSelfMethodBool) + fd->type - dbField::tpMethodBool, 
                                  fd, expr);
        } else if (fd->type != dbField::tpStructure) { 
            expr = new dbExprNode((expr != NULL 
                                   ? dbvmLoadBool : dbvmLoadSelfBool) 
                                  + fd->type, fd, expr);
        }
        lex = scan();
    }
}



dbExprNode* dbCompiler::buildList() { 
    dbExprNode* expr = disjunction(); 
    return new dbExprNode(dbvmList, expr, lex == tkn_comma ? buildList() : NULL);
}

dbExprNode* dbCompiler::term() 
{
    dbFieldDescriptor* fd;
    dbTableDescriptor* refTable;
    int cop;
    int pos = currPos;
    int prevLex = lex;
    dbBinding* bp;
    dbExprNode* expr = NULL;
    dbUserFunction* func;

    if ((cop = scan()) == tkn_where) { 
        if (firstPos == 0) { 
            firstPos = currPos;
        }
        cop = scan(); // just ignore 'where' keyword
    }
    lex = cop;

    switch (cop) { 
      case tkn_eof:
      case tkn_order:
      case tkn_start:
      case tkn_limit:
        return new dbExprNode(dbvmVoid);
      case tkn_current:
        lex = scan();
        return field(new dbExprNode(dbvmCurrent), table, NULL);
      case tkn_ident:
        for (bp = bindings; bp != NULL; bp = bp->next) { 
            if (name == bp->name) { 
                bp->used = true;
                lex = scan();
                return new dbExprNode(dbvmVariable, (dbExprNode*)0, bp->index);
            }
        }
        if ((fd = table->findSymbol(name)) == NULL) { 
            if ((func = dbUserFunction::find(name)) != NULL) { 
                pos = currPos;
                expr = term();
                int argType = func->getParameterType();
                int nParams =  func->getNumberOfParameters();
                if (nParams > 1) { 
                    if (expr->type != tpList || expr->operand[0] == NULL) {
                        error("Too few paramters for user defined function");
                    }
                    dbExprNode* params[3];
                    int n = 0;
                    do { 
                        if (n == nParams) { 
                            error("Too many parameters for user defined function");
                        }       
                        dbExprNode* param = expr->operand[0];
                        if (param->type != tpInteger && param->type != tpReal && param->type != tpString && param->type != tpWString
                            && param->type != tpReference && param->type != tpRawBinary && param->type != tpBoolean)
                        {
                            error("User function should receive parameters of boolean, integer, real, string, reference or user defined type", pos);
                        } 
                        params[n++] = param;
                        expr = expr->operand[1];
                    } while (expr != NULL);

                    if (n != nParams) { 
                        error("Too few parameters for user defined function");
                    }   
                    expr = new dbExprNode(dbvmFuncInt2Bool + func->type, func->fptr, params[0], params[1], params[2]);
                } else { 
                    if (argType == tpReal && expr->type == tpInteger) { 
                        expr = int2real(expr);
                    } else if (argType == tpList) {
                        if (expr->type != tpInteger && expr->type != tpReal && expr->type != tpString && expr->type != tpWString
                            && expr->type != tpReference && expr->type != tpRawBinary && expr->type != tpBoolean)
                        {
                            error("User function should receive parameter of boolean, integer, real, string, reference or user defined type", pos);
                        } 
                    } else if (argType != expr->type) { 
                        error("Incompatible function argument", pos);
                    }
                    expr = new dbExprNode(dbvmFuncInt2Bool + func->type, func->fptr, expr);
                }
                return field(expr, NULL, NULL);
            }
            error("Field not found", pos);
        }
        if (fd->type == dbField::tpRawBinary) { 
            expr = new dbExprNode(dbvmLoadSelfRawBinary, fd);
        } else if (fd->type == dbField::tpRectangle) { 
            expr = new dbExprNode(dbvmLoadSelfRectangle, fd);
        } else if (fd->type == dbField::tpWString) { 
            expr = new dbExprNode(dbvmLoadSelfWString, fd);
        } else if (fd->type != dbField::tpStructure) {
            expr = new dbExprNode(dbvmLoadSelfBool + fd->type, fd);
        } 
        lex = scan();
        return field(expr, NULL, fd);
      case tkn_exists:
        if (scan() == tkn_ident) { 
            dbBinding var;
            var.next = bindings;
            var.name = name;
            var.used = false;
            var.index = nFreeVars;
            bindings = &var;
            if (nFreeVars >= maxFreeVars) { 
                error("Too many nested EXISTS clauses\n");
            }
            nFreeVars += 1;
            pos = currPos;
            if (scan() != tkn_col) { 
                error("':' expected", pos);
            }
            expr = term();
            if (expr->type != tpBoolean) { 
                error("Expresion in EXISTS clause should be of boolean type");
            }
            if (var.used) { 
                expr = new dbExprNode(dbvmExists, expr, nFreeVars-1);
            }
            nFreeVars -= 1;         
            assert(bindings == &var);
            bindings = var.next;
            return expr;
        } else { 
            error("Free variable name expected");
        }       
        break;
      case tkn_first:
        lex = scan();
        return field(new dbExprNode(dbvmFirst), table, NULL);
      case tkn_last:
        lex = scan();
        return field(new dbExprNode(dbvmLast), table, NULL);
      case tkn_false:
        expr = new dbExprNode(dbvmLoadFalse);
        break;
      case tkn_true:
        expr = new dbExprNode(dbvmLoadTrue);
        break;
      case tkn_null:
        expr = new dbExprNode(dbvmLoadNull);
        break;
      case tkn_iconst:
        expr = new dbExprNode(dbvmLoadIntConstant, ivalue);
        break;
      case tkn_fconst:
        expr = new dbExprNode(dbvmLoadRealConstant, fvalue);
        break;
      case tkn_sconst:
        expr = new dbExprNode(dbvmLoadStringConstant, svalue); 
        lex = scan();
        return field(expr, NULL, NULL);
      case tkn_var:
        expr = new dbExprNode(dbvmLoadVarBool + varType - 
                              dbQueryElement::qVarBool, varPtr);
        refTable = varRefTable;
        lex = scan();
        return field(expr, refTable, NULL);
      case tkn_abs:
      case tkn_area:
      case tkn_length:
      case tkn_lower:
      case tkn_upper:
      case tkn_integer:
      case tkn_real:
      case tkn_string:
      case tkn_wstring:
        pos = currPos;
        expr = term();
        switch (cop) {
          case tkn_abs:
            if (expr->type == tpInteger) { 
                cop = dbvmAbsInt;
            } else if (expr->type == tpReal) { 
                cop = dbvmAbsReal;
            } else { 
                error("ABS function can be applied only "
                      "to integer or real expression", pos);
            }
            break;
          case tkn_area:
            if (expr->type == tpRectangle) {
                cop = dbvmRectangleArea;
            } else if (expr->type == tpList) { 
                expr = rectangleConstant(expr);
                cop = dbvmRectangleArea;                
            } else {
                error("AREA function can be applied only to rectangles", pos);
            }
            break;
          case tkn_length:
            if (expr->type == tpArray) { 
                cop = dbvmLength;
            } else if (expr->type == tpString) { 
                cop = dbvmStringLength;
            } else if (expr->type == tpWString) { 
                cop = dbvmWStringLength;
            } else { 
                error("LENGTH function is defined only for arrays and strings",
                      pos);
            } 
            break;
          case tkn_integer:
            if (expr->type == tpReal) { 
                cop = dbvmRealToInt;
            } else { 
                error("INTEGER function can be applied only to "
                      "expression of real type", pos);
            }
            break;
          case tkn_real:
            if (expr->type == tpInteger) { 
                cop = dbvmIntToReal;
            } else { 
                error("REAL function can be applied only to "
                      "expression of integer type", pos);
            }
            break;
          case tkn_wstring:
            if (expr->type == tpString) { 
                cop = dbvmMbsToWcs;
            } else { 
                error("WSTRING function can be applied only "
                      "to string expression", pos);
            }
           case tkn_string:
            if (expr->type == tpInteger) { 
                cop = dbvmIntToString;
            } else if (expr->type == tpReal) { 
                cop = dbvmRealToString;
            } else if (expr->type == tpWString) { 
                cop = dbvmWcsToMbs;
            } else { 
                error("STRING function can be applied only "
                      "to integer or real expression", pos);
            }
            break;
          case tkn_lower:
            if (expr->type == tpString) { 
                cop = dbvmLowerString;
            } else if (expr->type == tpWString) { 
                cop = dbvmLowerWString;
            } else { 
                error("LOWER function can be applied only to string argument", pos);
            }
            break;
          case tkn_upper:
            if (expr->type == tpString) { 
                cop = dbvmUpperString;
            } else if (expr->type == tpWString) { 
                cop = dbvmUpperWString;
            } else { 
                error("UPPER function can be applied only to string argument", pos);
            }
            break;
          default:
            assert(false);
        }
        return field(new dbExprNode(cop, expr), NULL, NULL);
      case tkn_lpar:
        {
            int tkn = scan();
            if (tkn == tkn_rpar) {
                expr = new dbExprNode(dbvmList, (dbExprNode*)NULL);
            } else {
                ungetToken(tkn);
                expr = disjunction();            
                if (lex == tkn_comma) { 
                    expr = new dbExprNode(dbvmList, expr, buildList());
                } else if (prevLex == tkn_in) { 
                    expr = new dbExprNode(dbvmList, expr);
                }
                if (lex != tkn_rpar) { 
                    error("')' expected");
                }
            }
        }
        break;
      case tkn_not:
        pos = currPos;
        expr = comparison();
        if (expr->type == tpInteger) { 
            if (expr->cop == dbvmLoadIntConstant) { 
                expr->ivalue = ~expr->ivalue;
            } else {
                expr = new dbExprNode(dbvmNotInt, expr);
            } 
            return expr;
        } else if (expr->type == tpBoolean) { 
            return new dbExprNode(dbvmNotBool, expr);
        } else { 
            error("NOT operator can be applied only to "
                  "integer or boolean expressions", pos);
        }
        break;
      case tkn_add:
        error("Using of unary plus operator has no sence");
        break;
      case tkn_sub:
        pos = currPos;
        expr = term();
        if (expr->type == tpInteger) { 
            if (expr->cop == dbvmLoadIntConstant) { 
                expr->ivalue = -expr->ivalue;
            } else { 
                expr = new dbExprNode(dbvmNegInt, expr);
            }
            return expr;
        } else if (expr->type == tpReal) { 
            if (expr->cop == dbvmLoadRealConstant) { 
                expr->fvalue = -expr->fvalue;
            } else { 
                expr = new dbExprNode(dbvmNegReal, expr);
            }
            return expr;
        } else { 
            error("Unary minus can be applied only to "
                  "integer or real expressions", pos);
        }
      default:
        error("operand expected");
    }
    lex = scan();
    return expr;
}

void dbCompiler::error(const char* msg, int pos) 
{ 
    if (pos < 0) { 
        if ((pos = currPos-1) < 0) { 
            pos = 0;
        }
    } else if (pos < firstPos) { 
        pos = firstPos;
    }
    if (pos + offsetWithinStatement >= 0) { 
        pos += offsetWithinStatement;
    }
    table->db->handleError(dbDatabase::QueryError, msg, pos);
    longjmp(abortCompilation, dbDatabase::QueryError);
}

void dbCompiler::compileStartFollowPart(dbQuery& query)
{
    if (lex != tkn_start) {     
        return;
    }
    int pos = currPos;
    if (scan() != tkn_from) { 
        error("FROM expected after START", pos);
    }
    pos = currPos;
    switch (scan()) { 
      case tkn_first:
        query.startFrom = dbCompiledQuery::StartFromFirst; 
        break;
      case tkn_last:
        query.startFrom = dbCompiledQuery::StartFromLast; 
        break;
      case tkn_var:
        if (varType == dbQueryElement::qVarReference) { 
            if (varRefTable != table) { 
                error("Incompatiable type of reference variable");
            }
            query.startFrom = dbCompiledQuery::StartFromRef; 
        } else if (varType == dbQueryElement::qVarArrayOfRef) {
            if (varRefTable != table) { 
                error("Incompatiable type of array of reference variable");
            }
            query.startFrom = dbCompiledQuery::StartFromArray; 
        } else if (varType == dbQueryElement::qVarArrayOfRefPtr) {
            if (varRefTable != table) { 
                error("Incompatiable type of array of reference variable");
            }
            query.startFrom = dbCompiledQuery::StartFromArrayPtr; 
        } else { 
            error("Reference or array of reference variable expected");
        }
        query.root = varPtr;
        break;
      default:
        error("FIRST, LAST or reference varaible expected", pos); 
    }
    if ((lex = scan()) == tkn_follow) { 
        pos = currPos;
        if (scan() != tkn_by) { 
            error("BY expected after FOLLOW", pos);
        }
        do { 
            pos = currPos;
            if (scan() != tkn_ident) { 
                error("Field name expected", pos);
            }
            dbFieldDescriptor* fd;
            if ((fd = table->findSymbol(name)) == NULL) { 
                error("Field not found");
            }
            while (fd->type == dbField::tpStructure) { 
                pos = currPos;
                if (scan() != tkn_dot) { 
                    error("'.' expected", pos);
                }
                pos = currPos;
                if (scan() != tkn_ident) { 
                    error("Field name expected", pos);
                }
                if ((fd = fd->findSymbol(name)) == NULL) { 
                    error("Field not found");
                }
            }
            if (!(fd->type == dbField::tpReference 
                  && fd->refTable == table) &&
                !(fd->type == dbField::tpArray 
                  && fd->components->type == dbField::tpReference
                  && fd->components->refTable == table)) 
            { 
                error("Follow field should be of compatibale reference "
                      "or array of reference type");
            }
            dbFollowByNode* node = new dbFollowByNode;
            node->field = fd;
            node->next = query.follow; // list should be inverted
            query.follow = node;
        } while ((lex = scan()) == tkn_comma); 
    }   
}

void dbCompiler::deleteNode(dbExprNode* node) {
    dbMutex& mutex = dbExprNodeAllocator::instance.getMutex();
    dbCriticalSection cs(mutex);
    delete node;
}

void dbCompiler::compileLimitPart(dbQuery& query)
{ 
    if (lex == tkn_limit) {
        int pos = currPos;
        cardinality_t l1 = 0;
        cardinality_t* lp1 = NULL;
        switch (scan()) {
          case tkn_iconst:
            l1 = (cardinality_t)ivalue;
            break;
          case tkn_var:
            if (sizeof(cardinality_t) == 4) { 
                if (varType != dbQueryElement::qVarInt4) {
                    error("LIMIT parameter should have int4 type", pos);
                }
            } else { 
                if (varType != dbQueryElement::qVarInt8) {
                    error("LIMIT parameter should have int8 type", pos);
                }
            }
            lp1 = (cardinality_t*)varPtr;
            break;
          default:
            error("Parameter or integer constant expected", pos);
        }
        if ((lex = scan()) == tkn_comma) { 
            pos = currPos;
            cardinality_t l2 = 0;
            cardinality_t* lp2 = NULL;
            switch (scan()) {
              case tkn_iconst:
                l2 = (cardinality_t)ivalue;
                break;
              case tkn_var:
                if (sizeof(cardinality_t) == 4) { 
                    if (varType != dbQueryElement::qVarInt4) {
                        error("LIMIT parameter should have int4 type", pos);
                    }
                } else { 
                    if (varType != dbQueryElement::qVarInt8) {
                        error("LIMIT parameter should have int8 type", pos);
                    }
                }
                lp2 = (cardinality_t*)varPtr;
                break;
              default:
                error("Parameter or integer constant expected", pos);
            }
            query.stmtLimitStart = l1;
            query.stmtLimitStartPtr = lp1;
            query.stmtLimitLen = l2;
            query.stmtLimitLenPtr = lp2;
            lex = scan();
        } else { 
            query.stmtLimitStart = 0;
            query.stmtLimitStartPtr = NULL;
            query.stmtLimitLen = l1;
            query.stmtLimitLenPtr = lp1;
        }
        query.limitSpecified = true;
    }
}
            
void dbCompiler::compileOrderByPart(dbQuery& query)
{
    if (lex == tkn_order) {     
        dbOrderByNode** opp = &query.order;
        int pos = currPos;
        if (scan() != tkn_by) {
            error("BY expected after ORDER", pos);
        }
        int parentheses = 0;
        do {
            pos = currPos;
            int tkn = scan();
            if (tkn == tkn_lpar) {
                parentheses += 1;
            } else {
                ungetToken(tkn);
            }
            dbExprNode* expr = disjunction();
            dbOrderByNode* node = new dbOrderByNode;
            switch (expr->cop) { 
              case dbvmLoadSelfBool:
              case dbvmLoadSelfInt1:
              case dbvmLoadSelfInt2:
              case dbvmLoadSelfInt4:
              case dbvmLoadSelfInt8:
              case dbvmLoadSelfReal4:
              case dbvmLoadSelfReal8:
              case dbvmLoadSelfString:
              case dbvmLoadSelfWString:
              case dbvmLoadSelfArray:
              case dbvmLoadSelfReference:
              case dbvmLoadSelfRawBinary:
                assert(expr->ref.field != NULL);
                node->field = expr->ref.field;
                node->expr = NULL;
                deleteNode(expr);
                break;
              case dbvmLength:
                if (expr->operand[0]->cop == dbvmLoadSelfArray) { 
                    node->field = expr->operand[0]->ref.field;
                    node->expr = NULL;
                    deleteNode(expr);
                    break;                    
                }
                // no break
              default:
                if (expr->type > tpReference) { 
                    error("Expressions in ORDER BY part should have scalar type", pos);
                } 
                node->field = NULL;
                node->expr = expr;
            }
            node->table = table;
            node->ascent = true;
            *opp = node;
            opp = &node->next;
            *opp = NULL;
            if (lex == tkn_desc) {
                node->ascent = false;
                lex = scan();
            } else if (lex == tkn_asc) {
                lex = scan();
            } 
            if (lex == tkn_rpar) { 
                if (--parentheses < 0) { 
                    error("Unbalanced parentheses ");
                }
                lex = scan();
            }
        } while (lex == tkn_comma);
    }
}


dbExprNode* dbCompiler::compileExpression(dbTableDescriptor* table, char const* expr, int startPos)
{
    TRACE_MSG(("Compile expression for table %s\n", table->name));    
    if (setjmp(abortCompilation) == 0) { 
        this->table = table;
        bindings = NULL;
        nFreeVars = 0;
        dbQueryElement elem(dbQueryElement::qExpression, expr, NULL); 
        queryElement = &elem;
        hasToken  = false;
        currPos = firstPos = 0;
        offsetWithinStatement = startPos;
        return disjunction(); 
    } else {
        return NULL;
    }
}

bool dbCompiler::compile(dbTableDescriptor* table, dbQuery& query)
{
    TRACE_MSG(("Compile query for table %s\n", table->name));
    query.destroy(); 
    if (setjmp(abortCompilation) == 0) { 
        this->table = table;
        bindings = NULL;
        nFreeVars = 0;
        queryElement = query.elements;
        currPos = firstPos = 0;
        hasToken  = false;
        offsetWithinStatement = query.pos;
        lex = tkn_eof;
        dbExprNode* expr = disjunction(); 
        if (expr->type != tpBoolean && expr->type != tpVoid) { 
            error("Conditional expression should have boolean type\n");
        }
        compileStartFollowPart(query); 
        compileOrderByPart(query); 
        compileLimitPart(query);
        if (lex != tkn_eof) { 
            error("START, ORDER BY or LIMIT expected");
        }
        query.tree = expr;
        query.table = table;
        return true;
    } else { 
        dbMutex& mutex = dbExprNodeAllocator::instance.getMutex();
        dbCriticalSection cs(mutex);
        for (dbOrderByNode *op = query.order, *nop; op != NULL; op = nop) {
            nop = op->next;
            delete op;
        }
        for (dbFollowByNode *fp = query.follow, *nfp; fp != NULL; fp = nfp) {
            nfp = fp->next;
            delete fp;
        }
        return false;
    }
}

dbCompiler::dbCompiler() {
    static struct { 
        char* name;
        int   tag;
    } keywords[] = { 
        {"all",     tkn_all},
        {"abs",     tkn_abs},
        {"and",     tkn_and},
#ifndef AREA_IS_IDENTIFIER
        {"area",    tkn_area},
#endif
        {"asc",     tkn_asc},
        {"between", tkn_between},
        {"by",      tkn_by},
        {"current", tkn_current},
        {"desc",    tkn_desc},
        {"escape",  tkn_escape},
        {"exists",  tkn_exists},
        {"first",   tkn_first},
        {"false",   tkn_false},
        {"follow",  tkn_follow},
        {"from",    tkn_from},
        {"in",      tkn_in},
        {"is",      tkn_is},
        {"integer", tkn_integer},
        {"insert",  tkn_insert},
        {"into",    tkn_into},
        {"last",    tkn_last},
        {"length",  tkn_length},
        {"like",    tkn_like},
        {"limit",   tkn_limit},
        {"lower",   tkn_lower},
#ifdef USE_REGEX
        {"match",   tkn_match},
#endif
        {"not",     tkn_not},
        {"null",    tkn_null},
        {"or",      tkn_or},
        {"order",   tkn_order},
        {"overlaps",tkn_overlaps},
        {"real",    tkn_real},
        {"select",  tkn_select},
        {"start",   tkn_start},
        {"string",  tkn_string},
        {"table",   tkn_table},
        {"true",    tkn_true},
        {"upper",   tkn_upper},
        {"where",   tkn_where},
        {"wstring", tkn_wstring}
    };
    if (!initialized) { 
        for (unsigned i = 0; i < itemsof(keywords); i++) { 
            dbSymbolTable::add(keywords[i].name, keywords[i].tag, FASTDB_CLONE_ANY_IDENTIFIER);    
        }
        initialized = true;
    }
}

void dbInheritedAttribute::removeTemporaries()
{
    for (dbStringValue *next, *s = tempStrings; s != NULL; s = next) { 
        next = s->next;
        delete s;
    }
}

END_FASTDB_NAMESPACE

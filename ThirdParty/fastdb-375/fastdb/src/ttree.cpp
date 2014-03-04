//-< TTREE.CPP >-----------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update:  6-Jan-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// T-Tree implementation
//-------------------------------------------------------------------*--------*

#define INSIDE_FASTDB

#include "fastdb.h"
#include "ttree.h"

BEGIN_FASTDB_NAMESPACE

void dbTtree::find(dbDatabase* db, oid_t treeId, dbSearchContext& sc)
{
    oid_t rootId = ((dbTtree*)db->get(treeId))->root;
    if (rootId != 0) { 
        ((dbTtreeNode*)db->get(rootId))->find(db, sc);
    }
}

void dbTtree::prefixSearch(dbDatabase* db, oid_t treeId, dbSearchContext& sc)
{
    oid_t rootId = ((dbTtree*)db->get(treeId))->root;
    if (rootId != 0) { 
        ((dbTtreeNode*)db->get(rootId))->prefixSearch(db, sc);
    }
}

oid_t dbTtree::allocate(dbDatabase* db)
{
    oid_t oid = db->allocateObject(dbTtreeMarker);
    dbTtree* tree = (dbTtree*)db->get(oid);
    tree->root = 0;
    return oid;
}

void dbTtree::insert(dbDatabase* db, oid_t treeId, oid_t recordId, 
                     int type, int sizeofType, dbUDTComparator comparator, int offs)
{
    oid_t rootId = ((dbTtree*)db->get(treeId))->root;
    if (rootId == 0) {
        oid_t nodeId = dbTtreeNode::allocate(db, recordId);
        ((dbTtree*)db->put(treeId))->root = nodeId;
    } else { 
        byte* rec = (byte*)db->getRow(recordId);
        byte* key = rec + offs;
        dbArray<byte> arr;
        if (type == dbField::tpString || type == dbField::tpWString) { 
            key = rec + ((dbVarying*)key)->offs;
        } else if (type == dbField::tpArray) { 
            arr.assign(rec + ((dbVarying*)key)->offs, ((dbVarying*)key)->size, false);
            key = (byte*)&arr;
        }
        oid_t nodeId = rootId;
        dbTtreeNode::insert(db, nodeId, recordId, key, type, sizeofType, comparator, offs);
        if (rootId != nodeId) { 
            ((dbTtree*)db->put(treeId))->root = nodeId;
        }
    }
}


void dbTtree::remove(dbDatabase* db, oid_t treeId, oid_t recordId, 
                     int type, int sizeofType, dbUDTComparator comparator, int offs)
{
    oid_t rootId = ((dbTtree*)db->get(treeId))->root;
    byte* rec = (byte*)db->getRow(recordId);
    byte* key = rec + offs;
    dbArray<byte> arr;
    if (type == dbField::tpString || type == dbField::tpWString) { 
        key = rec + ((dbVarying*)key)->offs;
    } else if (type == dbField::tpArray) { 
        arr.assign(rec + ((dbVarying*)key)->offs, ((dbVarying*)key)->size, false);
        key = (byte*)&arr;
    }
    oid_t nodeId = rootId;
    int h = dbTtreeNode::remove(db, nodeId, recordId, key, type, sizeofType, comparator, offs);
    assert(h >= 0);
    if (nodeId != rootId) { 
        ((dbTtree*)db->put(treeId))->root = nodeId;
    }
}   


void dbTtree::purge(dbDatabase* db, oid_t treeId) 
{
    oid_t rootId = ((dbTtree*)db->get(treeId))->root;
    dbTtreeNode::purge(db, rootId);
    ((dbTtree*)db->put(treeId))->root = 0;
}
    
void dbTtree::drop(dbDatabase* db, oid_t treeId) 
{
    purge(db, treeId);
    db->freeObject(treeId);
}
    

void dbTtree::traverseForward(dbDatabase* db, oid_t treeId,
                              dbAnyCursor* cursor)
{
    oid_t rootId = ((dbTtree*)db->get(treeId))->root;
    if (rootId != 0) { 
        ((dbTtreeNode*)db->get(rootId))->traverseForward(db, cursor);
    }
}

void dbTtree::traverseBackward(dbDatabase* db, oid_t treeId,
                              dbAnyCursor* cursor)
{
    oid_t rootId = ((dbTtree*)db->get(treeId))->root;
    if (rootId != 0) { 
        ((dbTtreeNode*)db->get(rootId))->traverseBackward(db, cursor);
    }
}


void dbTtree::traverseForward(dbDatabase* db, oid_t treeId,
                             dbAnyCursor* cursor, dbExprNode* condition)
{
    oid_t rootId = ((dbTtree*)db->get(treeId))->root;
    if (rootId != 0) { 
        ((dbTtreeNode*)db->get(rootId))->traverseForward(db, cursor,condition);
    }
}


void dbTtree::traverseBackward(dbDatabase* db, oid_t treeId,
                              dbAnyCursor* cursor, dbExprNode* condition)
{
    oid_t rootId = ((dbTtree*)db->get(treeId))->root;
    if (rootId != 0) { 
        ((dbTtreeNode*)db->get(rootId))->traverseBackward(db,cursor,condition);
    }
}

static int prefmbscmp(char const* key, char const* prefix) { 
    return strncmp(key, prefix, strlen(prefix));
}

static int prefwcscmp(wchar_t const* key, wchar_t const* prefix) { 
    return wcsncmp(key, prefix, wcslen(prefix));
}

typedef int(*prefcmp_t)(const char* key, const char* prefix);

bool dbTtreeNode::prefixSearch(dbDatabase* db, dbSearchContext& sc)
{
    char* rec;
    int   l, r, m, n = nItems;

    sc.probes += 1;
    dbTable* table = (dbTable*)db->getRow(sc.cursor->table->tableId);
    dbUDTComparator comparator = sc.field->_comparator;
    assert (sc.type == dbField::tpString || sc.type == dbField::tpWString);
    rec = (char*)db->getRow(item[0]);
    char* key = sc.firstKey;
    if (comparator(key, rec+((dbVarying*)(rec+sc.offs))->offs, ((dbVarying*)(rec+sc.offs))->size) > 0) {
        rec = (char*)db->getRow(item[n-1]);
        if (comparator(key, rec+((dbVarying*)(rec+sc.offs))->offs, ((dbVarying*)(rec+sc.offs))->size) > 0) { 
            if (right != 0) { 
                return ((dbTtreeNode*)db->get(right))->find(db, sc); 
            } 
            return true;
        }
        for (l = 0, r = n; l < r;) { 
            m = (l + r) >> 1;
            rec = (char*)db->getRow(item[m]);
            if (comparator(sc.firstKey, rec + ((dbVarying*)(rec+sc.offs))->offs, ((dbVarying*)(rec+sc.offs))->size) > 0) {
                l = m+1;
            } else { 
                r = m;
            }
        }
        while (r < n) { 
            rec = (char*)db->getRow(item[r]);
            if (comparator(key, rec + ((dbVarying*)(rec+sc.offs))->offs, ((dbVarying*)(rec+sc.offs))->size) < 0) { 
                return false;
            }
            if (!sc.condition 
                || db->evaluate(sc.condition, item[r], table, sc.cursor)) 
            {
                if (!sc.cursor->add(item[r])) { 
                    return false;
                }
            }
            r += 1;
        }
        if (right != 0) { 
            return ((dbTtreeNode*)db->get(right))->find(db, sc); 
        } 
        return true;    
    }
    if (left != 0) { 
        if (!((dbTtreeNode*)db->get(left))->find(db, sc)) { 
            return false;
        }
    }
    for (l = 0; l < n; l++) { 
        rec = (char*)db->getRow(item[l]);
        if (comparator(key, rec + ((dbVarying*)(rec+sc.offs))->offs, ((dbVarying*)(rec+sc.offs))->size) < 0) { 
            return false;
        }
        if (!sc.condition || db->evaluate(sc.condition, item[l], table, sc.cursor)) {
            if (!sc.cursor->add(item[l])) { 
                return false;
            }
        }
    }
    if (right != 0) { 
        return ((dbTtreeNode*)db->get(right))->find(db, sc);
    } 
    return false;
}


bool dbTtreeNode::find(dbDatabase* db, dbSearchContext& sc)
{
    char* rec;
    int   diff;
    int   l, r, m, n = nItems;

    sc.probes += 1;
    dbTable* table = (dbTable*)db->getRow(sc.cursor->table->tableId);
    dbUDTComparator comparator = sc.field->_comparator;

    if (sc.type == dbField::tpString || sc.type == dbField::tpWString) { 
        if (sc.firstKey != NULL) { 
            rec = (char*)db->getRow(item[0]);
            diff = comparator(sc.firstKey, rec+((dbVarying*)(rec+sc.offs))->offs, MAX_STRING_LENGTH);
            if (diff >= sc.firstKeyInclusion) {     
                rec = (char*)db->getRow(item[n-1]);
                diff = comparator(sc.firstKey, rec+((dbVarying*)(rec+sc.offs))->offs, MAX_STRING_LENGTH);
                if (diff >= sc.firstKeyInclusion) { 
                    if (right != 0) { 
                        return ((dbTtreeNode*)db->get(right))->find(db, sc); 
                    } 
                    return true;
                }
                for (l = 0, r = n; l < r;) { 
                    m = (l + r) >> 1;
                    rec = (char*)db->getRow(item[m]);
                    diff = comparator(sc.firstKey, rec + ((dbVarying*)(rec+sc.offs))->offs, MAX_STRING_LENGTH);
                    if (diff >= sc.firstKeyInclusion) {
                        l = m+1;
                    } else { 
                        r = m;
                    }
                }
                while (r < n) { 
                    rec = (char*)db->getRow(item[r]);
                    if ((sc.lastKey != NULL 
                         && comparator(rec + ((dbVarying*)(rec+sc.offs))->offs,
                                          sc.lastKey, MAX_STRING_LENGTH) >= sc.lastKeyInclusion) || 
                        (sc.prefixLength != 0 
                         && comparator(rec + ((dbVarying*)(rec+sc.offs))->offs,
                                          sc.firstKey, sc.prefixLength) != 0))
                    { 
                        return false;
                    }                        
                    if (!sc.condition 
                        || db->evaluate(sc.condition, item[r], table, sc.cursor)) 
                    {
                        if (!sc.cursor->add(item[r])) { 
                            return false;
                        }
                    }
                    r += 1;
                }
                if (right != 0) { 
                    return ((dbTtreeNode*)db->get(right))->find(db, sc); 
                } 
                return true;    
            }
        }       
        if (left != 0) { 
            if (!((dbTtreeNode*)db->get(left))->find(db, sc)) { 
                return false;
            }
        }
        for (l = 0; l < n; l++) { 
            rec = (char*)db->getRow(item[l]);
            if ((sc.lastKey != NULL 
                 && comparator(rec + ((dbVarying*)(rec+sc.offs))->offs, 
                                  sc.lastKey, MAX_STRING_LENGTH) >= sc.lastKeyInclusion) ||
                (sc.prefixLength != 0
                 && comparator(rec + ((dbVarying*)(rec+sc.offs))->offs,
                                  sc.firstKey, sc.prefixLength) != 0))
            {
                return false;
            }
            if (!sc.condition || db->evaluate(sc.condition, item[l], table, sc.cursor)) {
                if (!sc.cursor->add(item[l])) { 
                    return false;
                }
            }
        }
        if (right != 0) { 
            return ((dbTtreeNode*)db->get(right))->find(db, sc);
        } 
    } else if (sc.type == dbField::tpArray) { 
        if (sc.firstKey != NULL) { 
            rec = (char*)db->getRow(item[0]);
            {
                dbArray<char> a(rec+((dbVarying*)(rec+sc.offs))->offs, ((dbVarying*)(rec+sc.offs))->size);
                diff = comparator(sc.firstKey, &a, 0);
            }
            if (diff >= sc.firstKeyInclusion) {     
                rec = (char*)db->getRow(item[n-1]);
                {
                    dbArray<char> a(rec+((dbVarying*)(rec+sc.offs))->offs, ((dbVarying*)(rec+sc.offs))->size);                    
                    diff = comparator(sc.firstKey, &a, 0);
                }
                if (diff >= sc.firstKeyInclusion) { 
                    if (right != 0) { 
                        return ((dbTtreeNode*)db->get(right))->find(db, sc); 
                    } 
                    return true;
                }
                for (l = 0, r = n; l < r;) { 
                    m = (l + r) >> 1;
                    rec = (char*)db->getRow(item[m]);
                    {
                        dbArray<char> a(rec+((dbVarying*)(rec+sc.offs))->offs, ((dbVarying*)(rec+sc.offs))->size);
                        diff = comparator(sc.firstKey, &a, 0);
                    }
                    if (diff >= sc.firstKeyInclusion) {
                        l = m+1;
                    } else { 
                        r = m;
                    }
                }
                while (r < n) { 
                    rec = (char*)db->getRow(item[r]);
                    if (sc.lastKey != NULL) {
                        dbArray<char> a(rec+((dbVarying*)(rec+sc.offs))->offs, ((dbVarying*)(rec+sc.offs))->size);
                        if (comparator(&a, sc.lastKey, 0) >= sc.lastKeyInclusion) { 
                            return false;
                        }
                    }                        
                    if (!sc.condition 
                        || db->evaluate(sc.condition, item[r], table, sc.cursor)) 
                    {
                        if (!sc.cursor->add(item[r])) { 
                            return false;
                        }
                    }
                    r += 1;
                }
                if (right != 0) { 
                    return ((dbTtreeNode*)db->get(right))->find(db, sc); 
                } 
                return true;    
            }
        }       
        if (left != 0) { 
            if (!((dbTtreeNode*)db->get(left))->find(db, sc)) { 
                return false;
            }
        }
        for (l = 0; l < n; l++) { 
            rec = (char*)db->getRow(item[l]);
            if (sc.lastKey != NULL) { 
                dbArray<char> a(rec+((dbVarying*)(rec+sc.offs))->offs, ((dbVarying*)(rec+sc.offs))->size);
                if (comparator(&a, sc.lastKey, 0) >= sc.lastKeyInclusion) {
                    return false;
                }
            }
            if (!sc.condition || db->evaluate(sc.condition, item[l], table, sc.cursor)) {
                if (!sc.cursor->add(item[l])) { 
                    return false;
                }
            }
        }
        if (right != 0) { 
            return ((dbTtreeNode*)db->get(right))->find(db, sc);
        } 
    } else { 
        size_t sizeofType = sc.field->dbsSize;
        if (sc.firstKey != NULL) { 
            rec = (char*)db->getRow(item[0]);
            diff = comparator(sc.firstKey, rec+sc.offs, sizeofType);
            if (diff >= sc.firstKeyInclusion) {     
                rec = (char*)db->getRow(item[n-1]);
                diff = comparator(sc.firstKey, rec+sc.offs, sizeofType);
                if (diff >= sc.firstKeyInclusion) {
                    if (right != 0) { 
                        return ((dbTtreeNode*)db->get(right))->find(db, sc); 
                    } 
                    return true;
                }
                for (l = 0, r = n; l < r;) { 
                    m = (l + r) >> 1;
                    rec = (char*)db->getRow(item[m]);
                    diff = comparator(sc.firstKey, rec+sc.offs, sizeofType);
                    if (diff >= sc.firstKeyInclusion) {
                        l = m+1;
                    } else { 
                        r = m;
                    }
                }
                while (r < n) { 
                    rec = (char*)db->getRow(item[r]);
                    if (sc.lastKey != NULL 
                        && comparator(rec+sc.offs, sc.lastKey, sizeofType) 
                           >= sc.lastKeyInclusion) 
                    { 
                        return false;
                    }
                    if (!sc.condition 
                        || db->evaluate(sc.condition, item[r], table, sc.cursor)) 
                    {
                        if (!sc.cursor->add(item[r])) { 
                            return false;
                        }
                    }
                    r += 1;
                }
                if (right != 0) { 
                    return ((dbTtreeNode*)db->get(right))->find(db, sc); 
                } 
                return true;    
            }
        }       
        if (left != 0) { 
            if (!((dbTtreeNode*)db->get(left))->find(db, sc)) { 
                return false;
            }
        }
        for (l = 0; l < n; l++) { 
            rec = (char*)db->getRow(item[l]);
            if (sc.lastKey != NULL && comparator(rec+sc.offs, sc.lastKey, sizeofType) 
                >= sc.lastKeyInclusion) 
            {
                return false;
            }
            if (!sc.condition || db->evaluate(sc.condition, item[l], table, sc.cursor)) { 
                if (!sc.cursor->add(item[l])) { 
                    return false;
                }
            }
        }
        if (right != 0) { 
            return ((dbTtreeNode*)db->get(right))->find(db, sc);
        } 
    }
    return true;
}


oid_t dbTtreeNode::allocate(dbDatabase* db, oid_t recordId)
{
    oid_t nodeId = db->allocateObject(dbTtreeNodeMarker);
    dbTtreeNode* node = (dbTtreeNode*)db->get(nodeId);
    node->nItems = 1;
    node->item[0] = recordId;
    node->left = node->right = 0;
    node->balance = 0;
    return nodeId;
}

bool dbTtreeNode::insert(dbDatabase* db, oid_t& nodeId, oid_t recordId, 
                         void* key, int type, int sizeofType, dbUDTComparator comparator, int offs)
{
    dbTtreeNode* node = (dbTtreeNode*)db->get(nodeId);
    char* rec = (char*)db->getRow(node->item[0]);
    int n = node->nItems;
    int diff;
    if (type == dbField::tpArray) { 
        dbArray<char> a(rec + ((dbVarying*)(rec+offs))->offs, ((dbVarying*)(rec+offs))->size);
        diff = comparator(key, &a, 0);
    } else {
        diff = (type == dbField::tpString || type == dbField::tpWString)
            ? comparator(key, rec + ((dbVarying*)(rec+offs))->offs, MAX_STRING_LENGTH)
            : comparator(key, rec+offs, sizeofType);
    }
    if (diff <= 0) { 
        oid_t leftId = node->left;
        if ((leftId == 0 || diff == 0) && node->nItems != pageSize) { 
            node = (dbTtreeNode*)db->put(nodeId);
            for (int i = n; i > 0; i--) node->item[i] = node->item[i-1];
            node->item[0] = recordId;
            node->nItems += 1;
            return false;
        } 
        if (leftId == 0) { 
            leftId = allocate(db, recordId);
            node = (dbTtreeNode*)db->put(nodeId);
            node->left = leftId;
        } else {
            oid_t childId = leftId;
            bool grow = insert(db, childId, recordId, key, type, sizeofType, comparator, offs);
            if (childId != leftId) { 
                ((dbTtreeNode*)db->put(nodeId))->left = leftId = childId;
            }
            if (!grow) return false;
        }
        node = (dbTtreeNode*)db->put(nodeId);
        if (node->balance > 0) { 
            node->balance = 0;
            return false;
        } else if (node->balance == 0) { 
            node->balance = -1;
            return true;
        } else { 
            dbTtreeNode* left = (dbTtreeNode*)db->put(leftId);
            node = (dbTtreeNode*)db->get(nodeId);
            if (left->balance < 0) { // single LL turn
                node->left = left->right;
                left->right = nodeId;
                node->balance = 0;
                left->balance = 0;
                nodeId = leftId;
            } else { // double LR turn
                oid_t rightId = left->right;
                dbTtreeNode* right = (dbTtreeNode*)db->put(rightId);
                left = (dbTtreeNode*)db->get(leftId);
                node = (dbTtreeNode*)db->get(nodeId);
                left->right = right->left;
                right->left = leftId;
                node->left = right->right;
                right->right = nodeId;
                node->balance = (right->balance < 0) ? 1 : 0;
                left->balance = (right->balance > 0) ? -1 : 0;
                right->balance = 0;
                nodeId = rightId;
            }
            return false;
        }
    } 
    rec = (char*)db->getRow(node->item[n-1]);
    if (type == dbField::tpArray) {
        dbArray<char> a(rec + ((dbVarying*)(rec+offs))->offs, ((dbVarying*)(rec+offs))->size);
        diff = comparator(key, &a, 0);
    } else { 
        diff = (type == dbField::tpString || type == dbField::tpWString)
            ? comparator(key, rec + ((dbVarying*)(rec+offs))->offs, MAX_STRING_LENGTH)
            : comparator(key, rec+offs, sizeofType);
    }
    if (diff >= 0) { 
        oid_t rightId = node->right;
        if ((rightId == 0 || diff == 0) && node->nItems != pageSize) { 
            node = (dbTtreeNode*)db->put(nodeId);
            node->item[n] = recordId;
            node->nItems += 1;
            return false;
        }
        if (rightId == 0) { 
            rightId = allocate(db, recordId);
            node = (dbTtreeNode*)db->put(nodeId);
            node->right = rightId;
        } else { 
            oid_t childId = rightId;
            bool grow = insert(db, childId, recordId, key, type, sizeofType, comparator, offs);
            if (childId != rightId) { 
                ((dbTtreeNode*)db->put(nodeId))->right = rightId = childId;
            }
            if (!grow) return false;
        }
        node = (dbTtreeNode*)db->put(nodeId);
        if (node->balance < 0) { 
            node->balance = 0;
            return false;
        } else if (node->balance == 0) { 
            node->balance = 1;
            return true;
        } else { 
            dbTtreeNode* right = (dbTtreeNode*)db->put(rightId);
            node = (dbTtreeNode*)db->get(nodeId);
            if (right->balance > 0) { // single RR turn
                node->right = right->left;
                right->left = nodeId;
                node->balance = 0;
                right->balance = 0;
                nodeId = rightId;
            } else { // double RL turn
                oid_t leftId = right->left;
                dbTtreeNode* left = (dbTtreeNode*)db->put(leftId);
                right = (dbTtreeNode*)db->get(rightId);
                node = (dbTtreeNode*)db->get(nodeId);
                right->left = left->right;
                left->right = rightId;
                node->right = left->left;
                left->left = nodeId;
                node->balance = (left->balance > 0) ? -1 : 0;
                right->balance = (left->balance < 0) ? 1 : 0;
                left->balance = 0;
                nodeId = leftId;
            }
            return false;
        }
    }
    int l = 1, r = n-1;
    if (type == dbField::tpString || type == dbField::tpWString) { 
        while (l < r)  {
            int i = (l+r) >> 1;
            rec = (char*)db->getRow(node->item[i]);
            diff = comparator(key, rec + ((dbVarying*)(rec+offs))->offs, MAX_STRING_LENGTH);
            if (diff > 0) { 
                l = i + 1;
            } else { 
                r = i;
                if (diff == 0) { 
                    break;
                }
            }
        }
    } else if (type == dbField::tpArray) { 
        while (l < r)  {
            int i = (l+r) >> 1;
            rec = (char*)db->getRow(node->item[i]);
            dbArray<char> a(rec + ((dbVarying*)(rec+offs))->offs, ((dbVarying*)(rec+offs))->size);
            diff = comparator(key, &a, 0);
            if (diff > 0) { 
                l = i + 1;
            } else { 
                r = i;
                if (diff == 0) { 
                    break;
                }
            }
        }
    } else { 
        while (l < r)  {
            int i = (l+r) >> 1;
            rec = (char*)db->getRow(node->item[i]);
            diff = comparator(key, rec+offs, sizeofType);
            if (diff > 0) { 
                l = i + 1;
            } else { 
                r = i;
                if (diff == 0) { 
                    break;
                }
            }
        }
    }
    // Insert before item[r]
    node = (dbTtreeNode*)db->put(nodeId);
    if (n != pageSize) {
        for (int i = n; i > r; i--) node->item[i] = node->item[i-1]; 
        node->item[r] = recordId;
        node->nItems += 1;
        return false;
    } else { 
        oid_t reinsertId;
        dbArray<char> arr;
        if (node->balance >= 0) { 
            reinsertId = node->item[0];
            for (int i = 1; i < r; i++) node->item[i-1] = node->item[i]; 
            node->item[r-1] = recordId;
        } else { 
            reinsertId = node->item[n-1];
            for (int i = n-1; i > r; i--) node->item[i] = node->item[i-1]; 
            node->item[r] = recordId;
        }
        rec = (char*)db->getRow(reinsertId);
        key = rec + offs;
        if (type == dbField::tpString || type == dbField::tpWString) { 
            key = rec + ((dbVarying*)key)->offs;
        } else if (type == dbField::tpArray) { 
            arr.assign(rec + ((dbVarying*)key)->offs, ((dbVarying*)key)->size, false);
            key = &arr;
        }
        return insert(db, nodeId, reinsertId, key, type, sizeofType, comparator, offs);
    }
}

inline int dbTtreeNode::balanceLeftBranch(dbDatabase* db, oid_t& nodeId)
{
    dbTtreeNode* node = (dbTtreeNode*)db->put(nodeId);
    if (node->balance < 0) { 
        node->balance = 0;
        return 1;
    } else if (node->balance == 0) { 
        node->balance = 1;
        return 0;
    } else { 
        oid_t rightId = node->right;
        dbTtreeNode* right = (dbTtreeNode*)db->put(rightId);
        node = (dbTtreeNode*)db->get(nodeId);
        if (right->balance >= 0) { // single RR turn
            node->right = right->left;
            right->left = nodeId;
            if (right->balance == 0) { 
                node->balance = 1;
                right->balance = -1;
                nodeId = rightId;
                return 0;
            } else { 
                node->balance = 0;
                right->balance = 0;
                nodeId = rightId;
                return 1;
            }
        } else { // double RL turn
            oid_t leftId = right->left;
            dbTtreeNode* left = (dbTtreeNode*)db->put(leftId);
            node = (dbTtreeNode*)db->get(nodeId);
            right = (dbTtreeNode*)db->get(rightId);
            right->left = left->right;
            left->right = rightId;
            node->right = left->left;
            left->left = nodeId;
            node->balance = left->balance > 0 ? -1 : 0;
            right->balance = left->balance < 0 ? 1 : 0;
            left->balance = 0;
            nodeId = leftId;
            return 1;
        }
    }
}


inline int dbTtreeNode::balanceRightBranch(dbDatabase* db, oid_t& nodeId)
{
    dbTtreeNode* node = (dbTtreeNode*)db->put(nodeId);
    if (node->balance > 0) { 
        node->balance = 0;
        return 1;
    } else if (node->balance == 0) { 
        node->balance = -1;
        return 0;
    } else { 
        oid_t leftId = node->left;
        dbTtreeNode* left = (dbTtreeNode*)db->put(leftId);
        node = (dbTtreeNode*)db->get(nodeId);
        if (left->balance <= 0) { // single LL turn
            node->left = left->right;
            left->right = nodeId;
            if (left->balance == 0) { 
                node->balance = -1;
                left->balance = 1;
                nodeId = leftId;
                return 0;
            } else { 
                node->balance = 0;
                left->balance = 0;
                nodeId = leftId;
                return 1;
            }
        } else { // double LR turn
            oid_t rightId = left->right;
            dbTtreeNode* right = (dbTtreeNode*)db->put(rightId);
            node = (dbTtreeNode*)db->get(nodeId);
            left = (dbTtreeNode*)db->get(leftId);
            left->right = right->left;
            right->left = leftId;
            node->left = right->right;
            right->right = nodeId;
            node->balance = right->balance < 0 ? 1 : 0;
            left->balance = right->balance > 0 ? -1 : 0;
            right->balance = 0;
            nodeId = rightId;
            return 1;
        }
    }
}

int dbTtreeNode::remove(dbDatabase* db, oid_t& nodeId, oid_t recordId, 
                        void* key, int type, int sizeofType, dbUDTComparator comparator, int offs)
{
    dbTtreeNode* node = (dbTtreeNode*)db->get(nodeId);
    char* rec = (char*)db->getRow(node->item[0]);
    int n = node->nItems;
    dbArray<char> arr;
    int diff;
    if (type == dbField::tpArray) { 
        dbArray<char> a(rec + ((dbVarying*)(rec+offs))->offs, ((dbVarying*)(rec+offs))->size);
        diff = comparator(key, &a, 0);
    } else {
        diff = (type == dbField::tpString || type == dbField::tpWString)
            ? comparator(key, rec + ((dbVarying*)(rec+offs))->offs, MAX_STRING_LENGTH)
            : comparator(key, rec+offs, sizeofType);
    }
    if (diff <= 0) { 
        oid_t leftId = node->left;
        if (leftId != 0) { 
            oid_t childId = leftId;
            int h = remove(db, childId, recordId, key, type, sizeofType, comparator, offs);
            if (childId != leftId) { 
                ((dbTtreeNode*)db->put(nodeId))->left = childId;
            }
            if (h > 0) { 
                return balanceLeftBranch(db, nodeId);
            } else if (h == 0) { 
                return 0;
            }
        }
        assert (diff == 0);
    }
    rec = (char*)db->getRow(node->item[n-1]);
    if (type == dbField::tpArray) {
        dbArray<char> a(rec + ((dbVarying*)(rec+offs))->offs, ((dbVarying*)(rec+offs))->size);
        diff = comparator(key, &a, 0);
    } else {
        diff = (type == dbField::tpString || type == dbField::tpWString)
            ? comparator(key, rec + ((dbVarying*)(rec+offs))->offs, MAX_STRING_LENGTH)
            : comparator(key, rec+offs, sizeofType);
    }
    if (diff <= 0) {        
        for (int i = 0; i < n; i++) { 
            if (node->item[i] == recordId) { 
                if (n == 1) { 
                    oid_t delId = nodeId;                    
                    if (node->right == 0) { 
                        nodeId = node->left;
                        db->freeObject(delId);
                        return 1;
                    } else if (node->left == 0) { 
                        nodeId = node->right;
                        db->freeObject(delId);
                        return 1;
                    } 
                } 
                node = (dbTtreeNode*)db->put(nodeId);
                oid_t leftId = node->left, rightId = node->right;
                if (n <= minItems) { 
                    if (leftId != 0 && node->balance <= 0) {  
                        dbTtreeNode* left = (dbTtreeNode*)db->get(leftId);
                        while (left->right != 0) { 
                            left = (dbTtreeNode*)db->get(left->right);
                        }
                        while (--i >= 0) { 
                            node->item[i+1] = node->item[i];
                        }
                        node->item[0] = left->item[left->nItems-1];
                        rec = (char*)db->getRow(node->item[0]);
                        key = rec + offs;
                        if (type == dbField::tpString || type == dbField::tpWString) { 
                            key = rec + ((dbVarying*)key)->offs;
                        } else if (type == dbField::tpArray) { 
                            arr.assign(rec + ((dbVarying*)key)->offs, ((dbVarying*)key)->size, false);
                            key = &arr;
                        }
                        oid_t childId = leftId;
                        int h = remove(db, childId, node->item[0], 
                                       key, type, sizeofType, comparator, offs);
                        if (childId != leftId) { 
                            ((dbTtreeNode*)db->get(nodeId))->left = childId;
                        }
                        if (h > 0) {
                            h = balanceLeftBranch(db, nodeId);
                        }
                        return h;
                    } else if (node->right != 0) { 
                        dbTtreeNode* right = (dbTtreeNode*)db->get(rightId);
                        while (right->left != 0) { 
                            right = (dbTtreeNode*)db->get(right->left);
                        }
                        while (++i < n) { 
                            node->item[i-1] = node->item[i];
                        }
                        node->item[n-1] = right->item[0];
                        rec = (char*)db->getRow(node->item[n-1]);
                        key = rec + offs;
                        if (type == dbField::tpString || type == dbField::tpWString) { 
                            key = rec + ((dbVarying*)key)->offs;
                        } else if (type == dbField::tpArray) { 
                            arr.assign(rec + ((dbVarying*)key)->offs, ((dbVarying*)key)->size, false);
                            key = &arr;
                        }
                        oid_t childId = rightId;
                        int h = remove(db, childId, node->item[n-1], 
                                       key, type, sizeofType, comparator, offs);
                        if (childId != rightId) { 
                            ((dbTtreeNode*)db->get(nodeId))->right = childId;
                        }
                        if (h > 0) {
                            h = balanceRightBranch(db, nodeId);
                        }
                        return h;
                    }
                }
                while (++i < n) { 
                    node->item[i-1] = node->item[i];
                }
                node->nItems -= 1;
                return 0;
            }
        }
    }
    oid_t rightId = node->right;
    if (rightId != 0) { 
        oid_t childId = rightId;
        int h = remove(db, childId, recordId, key, type, sizeofType, comparator, offs);
        if (childId != rightId) { 
            ((dbTtreeNode*)db->put(nodeId))->right = childId;
        }
        if (h > 0) { 
            return balanceRightBranch(db, nodeId);
        } else { 
            return h;
        }
    }
    return -1;
}

void dbTtreeNode::purge(dbDatabase* db, oid_t nodeId)
{
    if (nodeId != 0) { 
        dbTtreeNode* node = (dbTtreeNode*)db->get(nodeId);
        oid_t leftId = node->left;
        oid_t rightId = node->right;
        db->freeObject(nodeId);
        purge(db, leftId);
        purge(db, rightId);
    }    
}

bool dbTtreeNode::traverseForward(dbDatabase* db, dbAnyCursor* cursor)
{
    if (left != 0) { 
        if (!((dbTtreeNode*)db->get(left))->traverseForward(db, cursor)) {
            return false;
        }
    }
    for (int i = 0, n = nItems; i < n; i++) { 
        if (!cursor->add(item[i])) { 
            return false;
        }
    }
    if (right != 0) { 
        return ((dbTtreeNode*)db->get(right))->traverseForward(db, cursor);
    }
    return true;
}

bool dbTtreeNode::traverseBackward(dbDatabase* db, dbAnyCursor* cursor)
{
    if (right != 0) { 
        if (!((dbTtreeNode*)db->get(right))->traverseBackward(db, cursor)) {
            return false;
        }
    }
    for (int i = nItems; --i >= 0;) { 
        if (!cursor->add(item[i])) { 
            return false;
        }
    }
    if (left != 0) { 
        return ((dbTtreeNode*)db->get(left))->traverseBackward(db, cursor);
    }
    return true;
}

bool dbTtreeNode::traverseForward(dbDatabase* db, dbAnyCursor* cursor,
                                  dbExprNode* condition)
{
    if (left != 0) { 
        if (!((dbTtreeNode*)db->get(left))->traverseForward(db, cursor, 
                                                            condition)) 
        {
            return false;
        }
    }
    dbTable* table = (dbTable*)db->getRow(cursor->table->tableId);
    for (int i = 0, n = nItems; i < n; i++) { 
        if (db->evaluate(condition, item[i], table, cursor)) { 
            if (!cursor->add(item[i])) { 
                return false;
            }
        }
    }
    if (right != 0) { 
        return ((dbTtreeNode*)db->get(right))->traverseForward(db, cursor, 
                                                               condition);
    }
    return true;
}

bool dbTtreeNode::traverseBackward(dbDatabase* db, dbAnyCursor* cursor,
                                   dbExprNode* condition)
{
    if (right != 0) { 
        if (!((dbTtreeNode*)db->get(right))->traverseBackward(db, cursor,
                                                             condition)) 
        {
            return false;
        }
    }
    dbTable* table = (dbTable*)db->getRow(cursor->table->tableId);
    for (int i = nItems; --i >= 0;) { 
        if (db->evaluate(condition, item[i], table, cursor)) { 
            if (!cursor->add(item[i])) { 
                return false;
            }
        }
    }
    if (left != 0) { 
        return ((dbTtreeNode*)db->get(left))->traverseBackward(db, cursor,
                                                              condition);
    }
    return true;
}

END_FASTDB_NAMESPACE

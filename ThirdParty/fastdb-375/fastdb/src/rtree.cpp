//-< RTREE.CPP >-----------------------------------------------------*--------*
// Fastdb                  Version 1.0         (c) 1999  GARRET    *     ?  *
// (Post Relational Database Management System)                      *   /\|  *
//                                                                   *  /  \  *
//                          Created:     22-Nov-2001  K.A. Knizhnik  * / [] \ *
//                          Last update: 22-Nov-2001  K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// R-tree class implementation
//-------------------------------------------------------------------*--------*

#define INSIDE_FASTDB

#include "fastdb.h"
#include "rtree.h"

BEGIN_FASTDB_NAMESPACE

void dbRtree::insert(dbDatabase* db, oid_t treeId, oid_t recordId, int offs)
{
    dbRtree* tree = (dbRtree*)db->get(treeId);
    byte* record = (byte*)db->get(recordId);
    rectangle r = *(rectangle*)(record + offs);
    oid_t root = tree->root;
    if (root == 0) { 
        oid_t newRoot = dbRtreePage::allocate(db, recordId, r);
        dbRtree* t = (dbRtree*)db->put(treeId);
        t->root = newRoot;
        t->height = 1;  
    } else { 
        oid_t p = dbRtreePage::insert(db, r, root, recordId, tree->height);
        if (p != 0) { 
            oid_t newRoot = dbRtreePage::allocate(db, root, p);
            dbRtree* t = (dbRtree*)db->put(treeId);
            // root splitted
            t->root = newRoot;
            t->height += 1;
        }
    }
}


void dbRtree::insert(dbDatabase* db, oid_t treeId, oid_t recordId, rectangle const& r)
{
    dbRtree* tree = (dbRtree*)db->get(treeId);
    oid_t root = tree->root;
    if (root == 0) { 
        oid_t newRoot = dbRtreePage::allocate(db, recordId, r);
        dbRtree* t = (dbRtree*)db->put(treeId);
        t->root = newRoot;
        t->height = 1;  
    } else { 
        oid_t p = dbRtreePage::insert(db, r, root, recordId, tree->height);
        if (p != 0) { 
            oid_t newRoot = dbRtreePage::allocate(db, root, p);
            dbRtree* t = (dbRtree*)db->put(treeId);
            // root splitted
            t->root = newRoot;
            t->height += 1;
        }
    }
}


void dbRtree::remove(dbDatabase* db, oid_t treeId, oid_t recordId, int offs)
{
    dbRtree* tree = (dbRtree*)db->get(treeId);
    assert(tree->height != 0);

    byte* record = (byte*)db->get(recordId);
    rectangle r = *(rectangle*)(record + offs);

    dbRtreePage::reinsert_list rlist;
    bool found = dbRtreePage::remove(db, r, tree->root, recordId, tree->height, rlist);
    assert(found);

    oid_t p = rlist.chain;
    int level = rlist.level;
    
    while (p != 0) {
        dbRtreePage* pg = (dbRtreePage*)db->get(p);
        for (int i = 0, n = pg->n; i < n; i++) { 
            oid_t q = dbRtreePage::insert(db, pg->b[i].rect, tree->root, 
                                          pg->b[i].p, tree->height-level);
            tree = (dbRtree*)db->get(treeId); 
            if (q != 0) { 
                // root splitted
                oid_t oldRoot = tree->root;
                oid_t newRoot = dbRtreePage::allocate(db, oldRoot, q);
                tree = (dbRtree*)db->put(treeId);
                tree->root = newRoot;
                tree->height += 1;
            }
            pg = (dbRtreePage*)db->get(p);
        }
        level -= 1;
        oid_t next = pg->next_reinsert_page();
        db->freeObject(p);
        p = next;
    }
    tree = (dbRtree*)db->get(treeId);
    dbRtreePage* pg = (dbRtreePage*)db->get(tree->root);
    if (pg->n == 1 && tree->height > 1) { 
        oid_t newRoot = (tree->height > 1) ? pg->b[0].p : 0;
        db->freeObject(tree->root);
        tree = (dbRtree*)db->put(treeId);
        tree->root = newRoot;
        tree->height -= 1;
    }
}

bool dbRtree::find(dbDatabase* db, oid_t treeId, dbSearchContext& sc)
{
    dbRtree* tree = (dbRtree*)db->get(treeId);
    if (tree->height > 0) {
        return dbRtreePage::find(db, tree->root, sc, tree->height);
    }
    return true;
}

void dbRtree::cover(dbDatabase* db, oid_t treeId, rectangle& r)
{
    dbRtree* tree = (dbRtree*)db->get(treeId);
    if (tree->height > 0) {
        dbRtreePage::cover(db, tree->root, r);
    } else { 
        for (int i = 0; i < RECTANGLE_DIMENSION; i++) { 
            r.boundary[i] = 1;
            r.boundary[i+RECTANGLE_DIMENSION] = 0;
        }
    }
}

void dbRtree::purge(dbDatabase* db, oid_t treeId)
{
    dbRtree* tree = (dbRtree*)db->put(treeId);
    if (tree->height > 0) {
        dbRtreePage::purge(db, tree->root, tree->height);
    } 
    tree = (dbRtree*)db->get(treeId);
    tree->root = 0;
    tree->height = 0;
}

void dbRtree::drop(dbDatabase* db, oid_t treeId)
{
    purge(db, treeId);
    db->freeObject(treeId);
}

oid_t dbRtree::allocate(dbDatabase* db)
{
    oid_t oid = db->allocateObject(dbRtreeMarker);
    dbRtree* tree = (dbRtree*)db->get(oid);
    tree->root = 0;
    tree->height = 0;
    return oid;
}

//-------------------------------------------------------------------------
// R-tree page methods
//-------------------------------------------------------------------------

//
// Search for objects overlapped with specified rectangle and call
// callback method for all such objects.
//

bool dbRtreePage::find(dbDatabase* db, oid_t pageId, dbSearchContext& sc, int level) 
{
    dbRtreePage* pg = (dbRtreePage*)db->get(pageId);
    bool rc = pg->find(db, sc, level);
    return rc;
}

static rectangle::comparator comparators[] = 
{
    &rectangle::operator ==, 
    &rectangle::operator &,
    &rectangle::operator >, 
    &rectangle::operator >=, 
    &rectangle::operator <, 
    &rectangle::operator <= 
};

bool dbRtreePage::find(dbDatabase* db, dbSearchContext& sc, int level) const
{
    assert(level >= 0);
    rectangle& r = *(rectangle*)sc.firstKey;
    sc.probes += 1;
    if (--level != 0) { /* this is an internal node in the tree */
        for (int i = 0; i < n; i++) { 
            if (b[i].rect & r) {
                if (!find(db, b[i].p, sc, level)) { 
                    return false;
                }
            }
        }
    } else { /* this is a leaf node */
        rectangle::comparator cmp = comparators[sc.firstKeyInclusion]; 
        dbTable* table = (dbTable*)db->get(sc.cursor->table->tableId);
        for (int i = 0; i < n; i++) { 
            if ((b[i].rect.*cmp)(r)) {              
                if (sc.condition == NULL 
                    || db->evaluate(sc.condition, b[i].p, table, sc.cursor)) 
                {
                    if (!sc.cursor->add(b[i].p)) { 
                        return false;
                    }
                }
            }
        }
    }
//    printf("Level %d, nodes %d, intersects %d\n", level, n, nIntersects);
    return true;
}

//
// Create root page
//
oid_t dbRtreePage::allocate(dbDatabase* db, oid_t recordId, rectangle const& r)
{
    oid_t pageId = db->allocateObject(dbRtreePageMarker);
    dbRtreePage* pg = (dbRtreePage*)db->put(pageId);
    pg->n = 1;
    pg->b[0].rect = r;
    pg->b[0].p = recordId;
    return pageId;
}

//
// Create new root page (root splitting)
//
oid_t dbRtreePage::allocate(dbDatabase* db, oid_t oldRootId, oid_t newPageId)
{
    oid_t pageId = db->allocateObject(dbRtreePageMarker);
    dbRtreePage* pg = (dbRtreePage*)db->put(pageId);
    pg->n = 2;
    cover(db, oldRootId, pg->b[0].rect);
    pg->b[0].p = oldRootId;
    cover(db, newPageId, pg->b[1].rect);
    pg->b[1].p = newPageId;
    return pageId;
}

//
// Calculate cover of all rectangles at page
//
void dbRtreePage::cover(dbDatabase* db, oid_t pageId, rectangle& r) 
{
    dbRtreePage* pg = (dbRtreePage*)db->get(pageId);
    pg->cover(r);
}

void dbRtreePage::cover(rectangle& r) const 
{
    r = b[0].rect;
    for (int i = 1; i < n; i++) { 
        r += b[i].rect;
    }
}

#define INFINITY (area_t)1000000000*1000000000

oid_t dbRtreePage::add_branch(dbDatabase* db, oid_t pageId, branch const& br)  
{
    dbRtreePage* pg = (dbRtreePage*)db->get(pageId);
    if (pg->n < card) { 
        pg->b[pg->n++] = br;
        return 0;
    }
    int i, j, seed[2] = {0,0};
    area_t rect_area[card+1], waste, worst_waste = -INFINITY;

    //
    // As the seeds for the two groups, find two rectangles which waste 
    // the most area if covered by a single rectangle.
    //
    rect_area[0] = area(br.rect);
    for (i = 0; i < card; i++) { 
        rect_area[i+1] = area(pg->b[i].rect);
    }
    branch const* bp = &br;
    for (i = 0; i < card; i++) { 
        for (j = i+1; j <= card; j++) { 
            waste = area(bp->rect + pg->b[j-1].rect) - rect_area[i] - rect_area[j];
            if (waste > worst_waste) {
                worst_waste = waste;
                seed[0] = i;
                seed[1] = j;
            }
        }
        bp = &pg->b[i];
    }       
    char taken[card];
    rectangle group[2];
    area_t group_area[2];
    int    group_card[2];
    oid_t  pid;
    
    memset(taken, 0, sizeof taken);
    taken[seed[1]-1] = 2;
    group[1] = pg->b[seed[1]-1].rect;
    
    if (seed[0] == 0) { 
        group[0] = br.rect;
        pid = allocate(db, br.p, br.rect);
    } else { 
        group[0] = pg->b[seed[0]-1].rect;
        oid_t pp = pg->b[seed[0]-1].p;
        pg->b[seed[0]-1] = br;
        pid = allocate(db, pp, group[0]);
    }
    dbRtreePage* p = (dbRtreePage*)db->put(pid);
    pg = (dbRtreePage*)db->get(pageId);

    group_card[0] = group_card[1] = 1;
    group_area[0] = rect_area[seed[0]];
    group_area[1] = rect_area[seed[1]];
    //
    // Split remaining rectangles between two groups.
    // The one chosen is the one with the greatest difference in area 
    // expansion depending on which group - the rect most strongly 
    // attracted to one group and repelled from the other.
    //
    while (group_card[0] + group_card[1] < card + 1 
           && group_card[0] < card + 1 - min_fill
           && group_card[1] < card + 1 - min_fill)
    {
        int better_group = -1, chosen = -1;
        area_t biggest_diff = -1;
        for (i = 0; i < card; i++) { 
            if (!taken[i]) { 
                area_t diff = (area(group[0] + pg->b[i].rect) - group_area[0])
                             - (area(group[1] + pg->b[i].rect) - group_area[1]);
                if (diff > biggest_diff || -diff > biggest_diff) { 
                    chosen = i;
                    if (diff < 0) { 
                        better_group = 0;
                        biggest_diff = -diff;
                    } else { 
                        better_group = 1;
                        biggest_diff = diff;
                    }
                }
            }
        }
        assert(chosen >= 0);
        group_card[better_group] += 1;
        group[better_group] += pg->b[chosen].rect;
        group_area[better_group] = area(group[better_group]);
        taken[chosen] = better_group+1;
        if (better_group == 0) { 
            p->b[group_card[0]-1] = pg->b[chosen];
        }
    }
    //
    // If one group gets too full, then remaining rectangle are
    // split between two groups in such way to balance cards of two groups.
    //
    if (group_card[0] + group_card[1] < card + 1) { 
        for (i = 0; i < card; i++) { 
            if (!taken[i]) { 
                if (group_card[0] >= group_card[1]) { 
                    taken[i] = 2;
                    group_card[1] += 1;
                } else { 
                    taken[i] = 1;
                    p->b[group_card[0]++] = pg->b[i];               
                }
            }
        }
    }
    p->n = group_card[0];
    pg->n = group_card[1];
    int n = pg->n;
    for (i = 0, j = 0; i < n; j++) { 
        if (taken[j] == 2) {
            pg->b[i++] = pg->b[j];
        }
    }
    return pid;
}

void dbRtreePage::remove_branch(int i)
{
    n -= 1;
    memmove(&b[i], &b[i+1], (n-i)*sizeof(branch));
}
    
oid_t dbRtreePage::insert(dbDatabase* db, rectangle const& r, oid_t pageId, oid_t recordId, int level)
{
    dbRtreePage* pg = (dbRtreePage*)db->put(pageId);
    int n = pg->n;
    branch br;
    if (--level != 0) { 
        // not leaf page
        int i, mini = 0;
        area_t min_incr = INFINITY;
        area_t best_area = INFINITY;
        for (i = 0; i < n; i++) { 
            area_t r_area = area(pg->b[i].rect);
            area_t incr = area(pg->b[i].rect + r) - r_area;
            if (incr < min_incr) { 
                best_area = r_area;
                min_incr = incr;
                mini = i;
            } else if (incr == min_incr && r_area < best_area) { 
                best_area = r_area;
                mini = i;
            }   
        }
        oid_t q = insert(db, r, pg->b[mini].p, recordId, level);
        pg = (dbRtreePage*)db->get(pageId);
        if (q == 0) { 
            // child was not split
            pg->b[mini].rect += r;
            return 0;
        } else { 
            // child was split
            cover(db, pg->b[mini].p, pg->b[mini].rect);
            br.p = q;
            cover(db, q, br.rect);
            return add_branch(db, pageId, br);
        }
    } else { 
        br.p = recordId;
        br.rect = r;
        return add_branch(db, pageId, br);
    }
}

bool dbRtreePage::remove(dbDatabase* db, rectangle const& r, oid_t pageId,  oid_t recordId, int level,
                         reinsert_list& rlist)
{
    dbRtreePage* pg = (dbRtreePage*)db->put(pageId);
    int n = pg->n;
    if (--level != 0) { 
        for (int i = 0; i < n; i++) { 
            if (pg->b[i].rect & r) { 
                if (remove(db, r, pg->b[i].p, recordId, level, rlist)) { 
                    dbRtreePage* p = (dbRtreePage*)db->get(pg->b[i].p);
                    pg = (dbRtreePage*)db->get(pageId);
                    if (p->n >= min_fill) { 
                        p->cover(pg->b[i].rect);
                    } else { 
                        // not enough entries in child
                        p = (dbRtreePage*)db->put(pg->b[i].p);
                        pg = (dbRtreePage*)db->get(pageId);
                        p->b[card-1].p = rlist.chain;
                        rlist.chain = pg->b[i].p;
                        rlist.level = level - 1; 
                        pg->remove_branch(i);
                    }
                    return true;
                }
            }
        }
    } else {
        for (int i = 0; i < n; i++) { 
            if (pg->b[i].p == recordId) { 
                pg->remove_branch(i);
                return true;
            }
        }
    }
    return false;
}

void dbRtreePage::purge(dbDatabase* db, oid_t pageId, int level)
{
    if (--level != 0) { /* this is an internal node in the tree */
        dbRtreePage* pg = (dbRtreePage*)db->get(pageId);
        for (int i = 0; i < pg->n; i++) { 
            purge(db, pg->b[i].p, level);
        }
    }
    db->freeObject(pageId);
}



END_FASTDB_NAMESPACE

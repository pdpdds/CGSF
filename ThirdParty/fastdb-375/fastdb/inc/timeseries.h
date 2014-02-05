//-< TIMESERIES.H >--------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Post Relational Database Management System)                      *   /\|  *
//                                                                   *  /  \  *
//                          Created:     22-Nov-2002  K.A. Knizhnik  * / [] \ *
//                          Last update: 22-Nov-2002  K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Container for time serires data
//-------------------------------------------------------------------*--------*

#ifndef __TIMESERIES_H__
#define __TIMESERIES_H__

#include "fastdb.h"

BEGIN_FASTDB_NAMESPACE

#define INFINITE_TIME 0x7fffffff

/**
 * Time series block contaning array of elements. Grouping several elements in one block (record)
 * reduce space overhead and increase processing speed.<BR>
 * <B>Attention!</B> This class is not serialized, so it is can be accessed only by one thread<P>
 * <I>You are defining your own time series class, for example:</I>
 * <PRE>
 * class Stock {
 *   public:
 *     char const* name;
 *     TYPE_DESCRIPTOR((KEY(name, INDEXED)));
 * };
 * 
 * 
 * class Quote {
 *   public:
 *     int4        tickerDate;
 *     real4       bid;
 *     int4        bidSize;
 *     real4       ask;
 *     int4        askSize;
 * 
 *     time_t time() const { return tickerDate; } // this method should be defined 
 * 
 *     TYPE_DESCRIPTOR((FIELD(tickerDate), FIELD(bid), FIELD(bidSize), FIELD(ask), FIELD(askSize)));
 * };
 * typedef dbTimeSeriesBlock<Quote>  DailyBlock;
 * REGISTER_TEMPLATE(DailyBlock);
 * REGISTER(Stock);
 * </PRE>    
 * <I>Now you can work with time series objects in the followin way:</I>
 * <PRE>
 * dbDatabase db;
 * if (db.open("mydatabase.dbs")) {
 *     dbTimeSeriesProcessor&lt;Quote&gt; proc(db, MIN_ELEMENTS_IN_BLOCK,MAX_ELEMENTS_IN_BLOCK);
 *     Quote quote;
 *     // initialize quote
 *     Stock stock;
 *     stock.name = "AAD";
 *     oid_t stockId = insert(stock).getOid();
 *     proc.add(stockId, quote); // add new element in time series
 * 
 *     Quote quoteBuf[MAX_QUOTES];
 *     // select quotes for the specified interval
 *     int n = proc.getInterval(stockId, fromDate, tillDate, quoteBuf, MAX_QUOTES);
 *     for (int i = 0; i < n; i++) {
 *         printf("bid=d ask=%d\n", quoteBuf[i].bid, quoteBuf[i].ask);
 *     }
 * }  
 * </PRE>
 */
template<class T>
class dbTimeSeriesBlock { 
  public:
    db_int8 blockId;
    db_int4 used;
    dbArray<T> elements;
  
    TYPE_DESCRIPTOR((KEY(blockId, INDEXED), FIELD(used), FIELD(elements)));
};


/**
 * Time series processor.<BR>
 * Element of time series can be arbitrary type with declared TYPE_DESCRIPTOR and defined
 * <code>time_t time()</code> method
 */
template<class T>
class dbTimeSeriesProcessor { 
    struct Interval { 
        db_int8 from;
        db_int8 till;
    };

  public:
    /**
     * Virtual method for processing elements, Should be redefinedin derived class.
     * @param data reference to the processed data element
     */
    virtual void process(T const& data) {}
    
    /**
     * Add new element
     * @param oid time series identifer (OID of the object associated with this time series)
     * @param data reference to the inserted element
     */
    void add(oid_t oid, T const& data) 
    { 
        Interval interval;
        interval.from = generateBlockId(oid, data.time() - maxBlockTimeInterval);
        interval.till = generateBlockId(oid, data.time());
        dbCursor< dbTimeSeriesBlock<T> > blocks;
        blocks.select(selectBlock, dbCursorForUpdate, &interval);
        if (blocks.last()) { 
            insertInBlock(oid, blocks, data);
        } else { 
            addNewBlock(oid, data);
        }
    }

    /**
     * Process elements in the block belonging to the specified range
     * @param oid time series identifer (OID of the object associated with this time series)
     * @param from inclusive low bound for element timestamp (set 0 to disable this criteria)
     * @param till inclusive high bound for element timestamp (set INFINITE_TIME to disable this criteria)
     */
    void select(oid_t oid, time_t from, time_t till) 
    { 
        Interval interval;
        interval.from = generateBlockId(oid, from - maxBlockTimeInterval);
        interval.till = generateBlockId(oid, till);
        dbCursor< dbTimeSeriesBlock<T> > blocks;
        if (blocks.select(selectBlock, dbCursorViewOnly, &interval)) { 
            do { 
                int n = blocks->used;
                T const* e =  blocks->elements.get();
                int l = 0, r = n;
                while (l < r)  {
                    int i = (l+r) >> 1;
                    if (from > e[i].time()) { 
                        l = i+1;
                    } else { 
                        r = i;
                    }
                }
                assert(l == r && (l == n || e[l].time() >= from)); 
                while (l < n && e[l].time() <= till) {
                    process(e[l++]);
                }
            } while (blocks.next());
        }
    }
    
    /**
     * Get the time of the first element in time series
     * @param oid time series identifer (OID of the object associated with this time series)
     * @return earliest time in times series or -1 if there are no elements in time series
     */
    time_t getFirstTime(oid_t oid) 
    {
        Interval interval;
        interval.from = generateBlockId(oid, 0);
        interval.till = generateBlockId(oid, INFINITE_TIME);
        dbCursor< dbTimeSeriesBlock<T> > blocks;
        blocks.setSelectionLimit(1);
        if (blocks.select(selectBlock, dbCursorViewOnly, &interval)) { 
            return blocks->elements[0].time();
        }
        return (time_t)-1;
    }
    
    /**
     * Get the time of the last element in time series
     * @param oid time series identifer (OID of the object associated with this time series)
     * @return latest time in times series or -1 if there are no elements in time series
     */
    time_t getLastTime(oid_t oid) 
    {
        Interval interval;
        interval.from = generateBlockId(oid, 0);
        interval.till = generateBlockId(oid, INFINITE_TIME);
        dbCursor< dbTimeSeriesBlock<T> > blocks;
        blocks.setSelectionLimit(1);
        if (blocks.select(selectBlockReverse, dbCursorViewOnly, &interval)) { 
            return blocks->elements[blocks->used-1].time();
        }
        return (time_t)-1;
    }
    
    /**
     * Get number of elements in time series.
     * @param oid time series identifer (OID of the object associated with this time series)
     * @return number of elements in time series.
     */
    size_t getNumberOfElements(oid_t oid) 
    {
        Interval interval;
        interval.from = generateBlockId(oid, 0);
        interval.till = generateBlockId(oid, INFINITE_TIME);
        dbCursor< dbTimeSeriesBlock<T> > blocks;
        int n = 0;
        if (blocks.select(selectBlock, dbCursorViewOnly, &interval)) {
            do { 
                n += blocks->used;
            } while (blocks.next());
        }
        return n;
    }
        
    /**
     * Select elements belonging to the specified interval
     * @param oid time series identifer (OID of the object associated with this time series)
     * @param from inclusive low bound for element timestamp (set 0 to disable this criteria)
     * @param till inclusive high bound for element timestamp (set INFINITE_TIME to disable this criteria)
     * @param buf destination buffer for selected elements
     * @param bufSize size of buffer: up to bufSize elements will be placed in buffer
     * @return number of elements belonging to the specified interval (can be greater than bufSize)
     */
    size_t getInterval(oid_t oid, time_t from, time_t till, T* buf, size_t bufSize) 
    { 
        Interval interval;
        interval.from = generateBlockId(oid, from == 0 ? 0 : from - maxBlockTimeInterval);
        interval.till = generateBlockId(oid, till);
        dbCursor< dbTimeSeriesBlock<T> > blocks;
        size_t nSelected = 0;
        if (blocks.select(selectBlock, dbCursorViewOnly, &interval)) { 
            do { 
                int n = blocks->used;
                T const* e =  blocks->elements.get();
                int l = 0, r = n;
                while (l < r)  {
                    int i = (l+r) >> 1;
                    if (from > e[i].time()) { 
                        l = i+1;
                    } else { 
                        r = i;
                    }
                }
                assert(l == r && (l == n || e[l].time() >= from)); 
                while (l < n && e[l].time() <= till) {
                    if (nSelected < bufSize) { 
                        buf[nSelected] = e[l];
                    }
                    l += 1;
                    nSelected += 1;
                }
            } while (blocks.next());
        }
        return nSelected;
    }        
        
    /**
     * Get time series element with specified time
     * @param oid time series identifer (OID of the object associated with this time series)
     * @param elem reference to the extracted element 
     * @param t timestamp of extracted element
     * @return <code>true</code> if element with specifed times exists in time series
     */
    bool getElement(oid_t oid, T& elem, time_t t) 
    { 
        return getInterval(oid, t, t, &elem, 1) == 1;
    }        
        
    /**
     * Select first N elements of times series with timestamp less than or equal to specified
     * @param oid time series identifer (OID of the object associated with this time series)
     * @param till inclusive high bound for element timestamp (set INFINITE_TIME to disable this criteria) 
     * @param buf destination buffer for selected elements
     * @param bufSize size of buffer: up to bufSize elements will be placed in buffer
     * @return number of selected elements (can be less than bufSize if there are less elements in time series
     * with timestamp less or equal than specified, but can not be greater than bufSize)
     */
    size_t getFirstInterval(oid_t oid, time_t till, T* buf, size_t bufSize) 
    {
        if (bufSize == 0) { 
            return 0;
        }
        Interval interval;
        interval.from = generateBlockId(oid, 0);
        interval.till = generateBlockId(oid, till);
        dbCursor< dbTimeSeriesBlock<T> > blocks;
        size_t nSelected = 0;
        if (blocks.select(selectBlock, dbCursorViewOnly, &interval)) { 
            do { 
                int n = blocks->used;
                T const* e =  blocks->elements.get();
                for (int i = 0; i < n && e[i].time() <= till; i++) { 
                    buf[nSelected++] = e[i];
                    if (nSelected == bufSize) { 
                        return nSelected;
                    }
                }
            } while (blocks.next());
        }
        return nSelected;
    }        


    /**
     * Select last N elements of times series with timestamp greater than or equal to specified
     * @param oid time series identifer (OID of the object associated with this time series)
     * @param from inclusive low bound for element timestamp (set 0 to disable this criteria)
     * @param buf destination buffer for selected elements
     * @param bufSize size of buffer: up to bufSize elements will be placed in buffer
     * @return number of selected elements (can be less than bufSize if there are less elements in time series
     * with timestamp greater or equal than specified, but can not be greater than bufSize)
     */
    size_t getLastInterval(oid_t oid, time_t from, T* buf, size_t bufSize) 
    {
        if (bufSize == 0) { 
            return 0;
        }
        Interval interval;
        interval.from = generateBlockId(oid, from == 0 ? 0 : from - maxBlockTimeInterval);
        interval.till = generateBlockId(oid, INFINITE_TIME);
        dbCursor< dbTimeSeriesBlock<T> > blocks;

        size_t nSelected = 0;
        blocks.select(selectBlock, dbCursorViewOnly, &interval);
        if (blocks.last()) { 
            do { 
                int n = blocks->used;
                T const* e =  blocks->elements.get();
                for (int i = n; --i >= 0 && e[i].time() >= from;) { 
                    buf[nSelected++] = e[i];
                    if (nSelected == bufSize) { 
                        return nSelected;
                    }
                }
            } while (blocks.prev());
        }
        return nSelected;
    }        



    /**
     * Check if there is element for specified data in time series
     * @param oid time series identifer (OID of the object associated with this time series)
     * @param t timestamp of checked element
     * @return <code>true</code> if element with specifed times exists in time series
     */
    bool hasElement(oid_t oid, time_t t) 
    { 
        T dummy;
        return getElement(oid, dummy, t);
    }        

    /**
     * TimeSeries processor constructor
     * @param database reference to the database
     * @param minElementsInBlock preallocated number of the elements in the block: 
     * array with specified number of elements will be allocated for new block
     * @param maxElementsInBlock maximal number of the elements in the block: block will be splitten if it has maxElementsInBlock
     * elements and new is added to the block
     * @param maxBlockTimeInterval maximal interval between first and last element in the block, new block will be created if 
     * adding new element to the block cause violation of this assumption. If maxBlockTimeInterval is 0, then it is assigned
     * to doubled number of seconds in day multipied on maxElementsInBlock
     */
    dbTimeSeriesProcessor(dbDatabase& database, int minElementsInBlock=100, int maxElementsInBlock=100, time_t maxBlockTimeInterval=0) :
        db(database) 
    {
        assert(minElementsInBlock > 0 && maxElementsInBlock >= minElementsInBlock);
        if (maxBlockTimeInterval == 0) { 
            maxBlockTimeInterval = 2*(maxElementsInBlock*24*60*60); // doubled interval in seconds, one element per day
        }        
        this->maxElementsInBlock = maxElementsInBlock;
        this->minElementsInBlock = minElementsInBlock;
        this->maxBlockTimeInterval = maxBlockTimeInterval;

        // correct instance of interval will be specified in select
        Interval* dummy = NULL;
        selectBlock = "blockId between",dummy->from,"and",dummy->till;
        selectBlockReverse = "blockId between",dummy->from,"and",dummy->till,"order by blockId desc";
    }

    /**
     * Remove elements for the sepcified period
     * @param oid time series identifer (OID of the object associated with this time series)
     * @param from inclusive low bound for element timestamp (set 0 to disable this criteria)
     * @param till inclusive high bound for element timestamp (set INFINITE_TIME to disable this criteria)
     * @return number of removed elements
     */
    int remove(oid_t oid, time_t from, time_t till)
    {
        Interval interval;
        interval.from = generateBlockId(oid, from == 0 ? 0 : from - maxBlockTimeInterval);
        interval.till = generateBlockId(oid, till);
        dbCursor< dbTimeSeriesBlock<T> > blocks;
        size_t nRemoved = 0;
        if (blocks.select(selectBlock, dbCursorForUpdate, &interval)) { 
            do { 
                int n = blocks->used;
                T const* e =  blocks->elements.get();
                int l = 0, r = n;
                while (l < r)  {
                    int i = (l+r) >> 1;
                    if (from > e[i].time()) { 
                        l = i+1;
                    } else { 
                        r = i;
                    }
                }
                assert(l == r && (l == n || e[l].time() >= from)); 
                while (r < n && e[r].time() <= till) {
                    r += 1;
                    nRemoved += 1;
                }
                if (l == 0 && r == n) { 
                    blocks.remove();
                } else if (l < n && l != r) { 
                    if (l == 0) { 
                        blocks->blockId = generateBlockId(oid, e[r].time());
                    }
                    T* ue = blocks->elements.update();
                    while (r < n) { 
                        ue[l++] = ue[r++];
                    }
                    blocks->used = l;
                    blocks.update();
                }
            } while (blocks.next());
        }
        return nRemoved;
    }        
    
    virtual~dbTimeSeriesProcessor() {}

    /**
     * This method should be actually private but since there is no portable way of declaration 
     * of friend templates classes recognized by all C++ compiler, it is made public.
     * Do not use this method yourself.
     */
    int _openIteratorCursor(dbCursor< dbTimeSeriesBlock<T> >& cursor, oid_t oid, time_t from, time_t till) 
    { 
        Interval interval;
        interval.from = generateBlockId(oid, from == 0 ? 0 : from - maxBlockTimeInterval);
        interval.till = generateBlockId(oid, till);
        return cursor.select(selectBlock, dbCursorViewOnly, &interval);
    }

   private:
     db_int8 generateBlockId(oid_t oid, time_t date) 
     {
        return cons_int8(oid, date);
     }
     
     
     void addNewBlock(oid_t oid, T const& data)
     {
         dbTimeSeriesBlock<T> block;
         block.blockId = generateBlockId(oid, data.time());
         block.elements.resize(minElementsInBlock);
         block.used = 1;
         block.elements.putat(0, data);
         insert(block);
     }

     void insertInBlock(oid_t oid, dbCursor< dbTimeSeriesBlock<T> >& blocks, T const& data)
     {
         time_t t = data.time();
         int i, n = blocks->used;

         T const* e =  blocks->elements.get();
         int l = 0, r = n;
         while (l < r)  {
             i = (l+r) >> 1;
             if (t > e[i].time()) { 
                 l = i+1;
             } else { 
                 r = i;
             }
         }
         assert(l == r && (l == n || e[l].time() >= t));
         if (r == 0) { 
             if (e[n-1].time() - t > maxBlockTimeInterval || n == maxElementsInBlock) { 
                 addNewBlock(oid, data);
                 return;
             }
             blocks->blockId = generateBlockId(oid, t);
         } else if (r == n) {
             if (t - e[0].time() > maxBlockTimeInterval || n == maxElementsInBlock) { 
                 addNewBlock(oid, data);
                 return;
             } 
         }
         if ((size_t)n == blocks->elements.length()) { 
             if (n == maxElementsInBlock) { 
                 T* u = blocks->elements.update();
                 addNewBlock(oid, u[n-1]);
                 for (i = n; --i > r; ) { 
                     u[i] = u[i-1];
                 }
                 u[r] = data;
                 blocks.update();
                 return;
             }
             blocks->elements.resize(n + minElementsInBlock < maxElementsInBlock ? n + minElementsInBlock : maxElementsInBlock);
         }
         T* u = blocks->elements.update();
         for (i = n; i > r; i--) { 
             u[i] = u[i-1];
         }
         u[r] = data;
         blocks->used += 1;
         blocks.update();
     }

     dbDatabase& db;
     int         maxElementsInBlock;
     int         minElementsInBlock;
     time_t      maxBlockTimeInterval;     
     dbQuery     selectBlock;
     dbQuery     selectBlockReverse; 
};
    

/**
 * Time series forward iterator 
 */
template<class T>
class dbTimeSeriesIterator { 
  public:
    /**
     * Start iteration through elements belonging to the specified range.
     * @param processor pointer to time series processor
     * @param oid time series identifer (OID of the object associated with this time series)
     * @param from inclusive low bound for element timestamp (set 0 to disable this criteria)
     * @param till inclusive high bound for element timestamp (set INFINITE_TIME to disable this criteria)
     */
    void start(dbTimeSeriesProcessor<T>* processor, oid_t oid, time_t from, time_t till) { 
        first = pos = -1;
        this->till = till;
        if (processor->_openIteratorCursor(blocks, oid, from, till)) { 
            do { 
                int n = blocks->used;
                T const* e =  blocks->elements.get();
                int l = 0, r = n;
                while (l < r)  {
                    int i = (l+r) >> 1;
                    if (from > e[i].time()) { 
                        l = i+1;
                    } else { 
                        r = i;
                    }
                }
                assert(l == r && (l == n || e[l].time() >= from)); 
                if (l < n) { 
                    if (e[l].time() <= till) {
                        first = pos = l;
                    }
                    return;
                }
            } while (blocks.next());
        }        
    }
            
    /**
     * Get current iterator element
     * @return <code>true</code> if there is current element, <code>false</code> otherwise
     */
    bool current(T& elem) { 
        if (pos >= 0) { 
            elem = blocks->elements[pos];
            return true;
        }
        return false;
    }
    
    /**
     * Move iterator position to next element.
     * @return <code>true</code> if next element exists, <code>false</code> otherwise
     */
    bool next() { 
        if (pos >= 0) { 
            if (++pos == blocks->used) { 
                if (!blocks.next()) { 
                    pos = -1;
                    return false;
                }
                pos = 0;
            }
            if (blocks->elements[pos].time() <= till) {
                return true;
            }
            pos = -1;
        }
        return false;
    }

    /**
     * Reset iterator to the initial state
     */
    void reset() { 
        blocks.first();
        pos = first;
    }
    
    /**
     * Iterator costructor. If current() or next() method will always return false if
     * them are invoked prior to start()
     */
    dbTimeSeriesIterator() {
        first = pos = -1;
    }
  private:
    dbCursor< dbTimeSeriesBlock<T> > blocks;
    int                              pos;
    int                              first;
    time_t                           till;
};
    
/**
 * Time series reverse iterator 
 */
template<class T>
class dbTimeSeriesReverseIterator { 
  public:
    /**
     * Start iteration through elements belonging to the specified range.
     * @param processor pointer to time series processor
     * @param oid time series identifer (OID of the object associated with this time series)
     * @param from inclusive low bound for element timestamp (set 0 to disable this criteria)
     * @param till inclusive high bound for element timestamp (set INFINITE_TIME to disable this criteria)
     */
    void start(dbTimeSeriesProcessor<T>* processor, oid_t oid, time_t from, time_t till) { 
        last = pos = -1;
        this->from = from;
        if (processor->_openIteratorCursor(blocks, oid, from, till)) { 
            do { 
                int n = blocks->used;
                blocks.last();
                T const* e =  blocks->elements.get();
                int l = 0, r = n;
                while (l < r)  {
                    int i = (l+r) >> 1;
                    if (till >= e[i].time()) { 
                        l = i+1;
                    } else { 
                        r = i;
                    }
                }
                assert(l == r && (l == n || e[l].time() > till)); 
                if (l > 0) {
                    if (e[l-1].time() >= from) {
                        last = pos = l-1;
                    }
                    return;
                }
            } while (blocks.prev());
        }        
    }
            
    /**
     * Get current iterator element
     * @return <code>true</code> if there is current element, <code>false</code> otherwise
     */
    bool current(T& elem) { 
        if (pos >= 0) { 
            elem = blocks->elements[pos];
            return true;
        }
        return false;
    }
    
    /**
     * Move iterator position to next element.
     * @return <code>true</code> if next element exists, <code>false</code> otherwise
     */
    bool next() { 
        if (pos >= 0) { 
            if (--pos < 0) {
                if (!blocks.prev()) { 
                    return false;
                }
                pos = blocks->used-1;
            }
            if (blocks->elements[pos].time() >= from) {
                return true;
            }
            pos = -1;
        }
        return false;
    }

    /**
     * Reset iterator to the initial state
     */
    void reset() { 
        blocks.last();
        pos = last;
    }
    
    /**
     * Iterator costructor. If current() or next() method will always return false if
     * them are invoked prior to start()
     */
    dbTimeSeriesReverseIterator() {
        last = pos = -1;
    }
  private:
    dbCursor< dbTimeSeriesBlock<T> > blocks;
    int                              pos;
    int                              last;
    time_t                           from;
};
    
END_FASTDB_NAMESPACE

#endif

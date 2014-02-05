//-< TESTTIMESERIES.CPP >--------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Post Relational Database Management System)                      *   /\|  *
//                                                                   *  /  \  *
//                          Created:     30-Mar-2009   K.A. Knizhnik * / [] \ *
//                          Last update: 30-Mar-2009   K.A. Knizhnik * GARRET *
//-------------------------------------------------------------------*--------*
// Example of using time series
//-------------------------------------------------------------------*--------*

#include "fastdb.h" 
#include "timeseries.h"
#include <stdio.h>
#include <time.h>
#include <math.h>

USE_FASTDB_NAMESPACE

const int N_QUOTES = 10000000;
const int N_ELEMENTS_PER_BLOCK = 100;
const int QUOTE_BUF_SIZE = 100;

class Stock {
  public:
    char const* name;
    TYPE_DESCRIPTOR((KEY(name, INDEXED)));
};


class Quote {
  public:
    int   timestamp;
    float low;
    float high;
    float open;
    float close;
    int   volume;

    time_t time() const { return timestamp; } // this method should be defined 

    TYPE_DESCRIPTOR((FIELD(timestamp), FIELD(low), FIELD(high), FIELD(open), FIELD(close), FIELD(volume)));
};

typedef dbTimeSeriesBlock<Quote>  DailyBlock;

REGISTER_TEMPLATE(DailyBlock);
REGISTER(Stock);

inline int random(unsigned mod) { return rand() % mod; }
inline float fmax(float x, float y) { return x > y ? x : y; }
inline float fmin(float x, float y) { return x < y ? x : y; }

int main(int argc, char* argv[])
{
    dbDatabase db;
    if (db.open(_T("timeseries"))) {
        dbTimeSeriesProcessor<Quote> proc(db, N_ELEMENTS_PER_BLOCK, N_ELEMENTS_PER_BLOCK, N_ELEMENTS_PER_BLOCK);
        Quote quote;

        Stock stock;
        stock.name = "AAD";
        oid_t stockId = insert(stock).getOid();
        time_t start = time(NULL);
        srand(2009);
        for (int i = 0; i < N_QUOTES; i++) { 
            quote.timestamp = i;
            quote.open = (float)random(10000)/100;
            quote.close = (float)random(10000)/100;
            quote.high = fmax(quote.open, quote.close);
            quote.low = fmin(quote.open, quote.close);
            quote.volume = random(1000);
            proc.add(stockId, quote); // add new element in time series
        }
        db.commit();
        printf("Elapsed time for importing %d quotes: %ld seconds\n", N_QUOTES, (long)(time(NULL) - start));
                
        Quote quotes[QUOTE_BUF_SIZE];
        srand(2009);
        start = time(NULL);
        for (int i = 0; i < N_QUOTES; i += QUOTE_BUF_SIZE) { 
            // select quotes for the specified interval
            int n = (int)proc.getInterval(stockId, i, i+QUOTE_BUF_SIZE-1, quotes, QUOTE_BUF_SIZE);
            assert(n == QUOTE_BUF_SIZE);
            for (int j = 0; j < n; j++) {
                assert(quotes[j].timestamp == i + j);
                assert((int)floor(quotes[j].open*100 + 0.5) == random(10000));
                assert((int)floor(quotes[j].close*100 + 0.5) == random(10000));
                assert(quotes[j].volume == random(1000));
            }
        } 
        printf("Elapsed time for traversing %d quotes: %ld seconds\n", N_QUOTES, (long)(time(NULL) - start));
        db.close();
        return 0;
    }
    return 1;
}  

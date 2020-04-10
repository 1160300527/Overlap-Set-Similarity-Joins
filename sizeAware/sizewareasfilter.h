#ifndef SIZEWAREASFILTER_H_INCLUDED
#define SIZEWAREASFILTER_H_INCLUDED


typedef struct CANDIDATESET
{
    long long  nPairs;
    long long  nSize;
    int *      candidate;
}CANDIDATESET;

typedef struct LONGPARA
{
   int ** relation;
   int ** invertedIndex;
   int  * relLarge;
   int    thresh;
   int    fThresh;
   long long rValueLong;
}LONGPARA;

typedef struct SHORTPARA
{
   int ** relation;
   int  * cand;
   int    thresh;
   long long rValueShort;
}SHORTPARA;



long long filterAndRefine(SSTATISTICS * stat, int** rel, int threshold);

/*****sizeAwareJoin is taken as a filter**********/
long long  sizeawareFilter(int** rel, int threshold, int fThreshold);
int *      ScanCountforLongSets(int **rel, int threshold, int *relLarge, int **inverted);
int *      filterBlockDedup(int **rel, int **inverted, int threshold, HEAP *hbt);

#endif // SIZEWAREASFILTER_H_INCLUDED

#ifndef FILTER_H_INCLUDED
#define FILTER_H_INCLUDED

#ifndef  ALPHA
#define  ALPHA  0.9
#endif


typedef  struct  FILTER
{
    int   nSubset;
    int * set2Class;   //an array of int, ith set in the relation be labeled to belong set2Class[i]th class
                       //set2Class[0]  is the number of classes
    char* interClass;
}FILTER;



long long  filterAllPairsJoin(int ** rel, int threshold);
long long  newFilterAllPairsJoin(int ** rel, int threshold);
void filterCreate(int ** rel, int threshold, FILTER * filter, int which,double *rate);
int  freToBucket(int frequency, int threshold);
int  freToBucketNew(int frequency, int threshold);
void freeFilter(FILTER * fPtr);
#endif // FILTER_H_INCLUDED

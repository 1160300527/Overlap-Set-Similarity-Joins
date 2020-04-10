#ifndef DIVIDESKIP_H_INCLUDED
#define DIVIDESKIP_H_INCLUDED


typedef struct SETINVERTED
{
    int value;
    int inverted_list;
    bool operator > (const SETINVERTED&a) const
    {
        return value>a.value;
    }
} SetInverted;

typedef struct SHORTSET
{
    int id;
    int num;
}ShortSet;


ShortSet* MergeSkip(int c,int **inverted,int R);
long long  DivideSkip(int** rel, int threshold);
int  **getInverted(int ** rel);
int BinarySearchInvert(int start,int ending,int *rel,int value);
void   DivideSkipFreeInverted(int ** index);
#endif // DIVIDESKIP_H_INCLUDED

#ifndef SIZEAWARE_H_INCLUDED
#define SIZEAWARE_H_INCLUDED



typedef struct  CSUBSET{
    int   *celement;             //c-subset的元素
    int   *celindex;             //c-subset各个元素在R中的下标
    int    r;                    //所属的集合
}CSUBSET;

typedef struct HEAP //定义堆的顺序存储类型
{
    CSUBSET * csubset; //定义指向动态数组空间的指针
    int len; //定义保存堆长度的变量,即数组长度，数组下标从0开始
    int maxsize;    //用于保存初始化时所给的动态数组空间的大小
}HEAP;

typedef struct MyStack{
	int *relList;
	int StackSize;

}MyStack;

void InitHeap(struct HEAP* hbt, int ms);
void ClearHeap(struct HEAP* hbt);
int EmptyHeap(struct HEAP* hbt);
void InsertHeap(struct HEAP* hbt, CSUBSET x, int c);
CSUBSET DeleteHeap(struct HEAP* hbt, int c);

long long  SizeAwareJoin(int** rel, int threshold);

int  **GetInvertedIndexinRs(int ** rel, int * relSmall);
int  **GetInvertedIndexinAllR(int ** rel);

int *GetTimeCostForLarge(int **rel, int **inverted);


void       SizeAwareFreeInverted(int ** iIndex);
//int GetResultsFromLargesets(int **rel, int threshold, int *relLarge);
int ScanCountforRl(int **rel, int threshold, int *relLarge, int **inverted);
void GetLargeAndSmallsets(int **rel, int x);

int GetSizeBoundary(int **rel, int c, int minsize, int maxsize, int *TimeCostForLarge);
long long EstimateTimeCostForLarge(int *TimeCostForLarge,int x,int **rel);
long long EstimateTimeCostForSmall(int x,int **rel,int c);
int   GetIntersectionSize(int *r, int *s);
long long GetCombinatorialNum(int m, int n);
long long BlockDedupForEstimating(int **rel, int **inverted, int threshold,HEAP *hbt,int b);
long long HeapDedupForEstimating(int **Rs, int c,HEAP *hbt);


int **BlockDedup(int **rel, int **inverted, int threshold,HEAP *hbt);
int **HeapDedup(int **Rs, int c,HEAP *hbt);
int  compareSetsSa(int * fSet, int * sSet,int c);
int  validAsetSizeAware(int * aSet);
CSUBSET binarySearchCsubset(int **Rs, CSUBSET rctop, int Rtemp, int c);
int binarySearchSa( int * data, int target, int sPos);

//int SizeAwarePartition(int **rel, int left, int right);
void sortRelSetByNum(int **rel, int start, int end);
int sortAndGetResultFromSmall(int **rel, int start, int end,int **relS);
void combination(int *Rs,HEAP *hbt,int num,int len,int start,int *result,int r);

#endif // SIZEAWARE_H_INCLUDED

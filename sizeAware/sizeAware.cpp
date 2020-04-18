#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <math.h>
#include  <time.h>
#include  <iostream>
#include  "dataload.h"
#include  "sizeAware.h"

//#ifndef  L_debug
//#define  L_debug
//#endif // L_debug

extern FILE * logFp;
extern FILE *  logs2;
using namespace std;

int *relLarge;
int *relSmall;
int binarySearchFlag;
FILE *  sizeawarelog;
void writeSmallToFile(int ** rel, int *relSmall);
void  sortRelLex(int ** aRel, int pStart, int pEnd);
int  compareSets(int * fSet, int * sSet);
void output(CSUBSET myCsubset,int c);
long long  SizeAwareJoin(int ** rel, int threshold)
/*****************************************************************************************************/
/***Join algorithm in sigmod2018 paper； Overlap Set Similarity Joins with Theoretical Guarantees*****/
/***Implemented by Rongxin Liu: a master student who won't pay much on research                  *****/
/*****************************************************************************************************/
{

    long long   rValue;
    int       **inverted;
    int       **invertedSmall;
    HEAP       *hbt =  ( HEAP  *)malloc(sizeof(HEAP));
    int x;                                                      //size boundary
    size_t      start, beginLarge, finishLarge, finishSmall, end;

    int        *TimeCostForLarge;     //对所有的集合R,计算其所包含的元素的倒排索引长度和
    int         minsize = rel[1][0];  //the smallest set size in R
    int         maxsize = rel[1][0];
    int       **resultSmall;



    printf("\n\n####################     SIZEAWARE    ############################\n");
    printf("sizeAware is ready to run........\n");
    fprintf(logFp, "\n\n####################    SIZEAWARE   ############################\n");
    fprintf(logFp, "sizeAware is ready to run........\n");
    start = clock();

    for(int i=2; i<=rel[0][0]; i++)
    {
        if(rel[i][0] < minsize)
        {
            minsize = rel[i][0];
        }

        if(rel[i][0] > maxsize)
        {
            maxsize = rel[i][0];
        }
    }
    rValue = 0;       //the number of join results
    sortRelLex(rel,1,rel[0][0]);
    sortRelSetByNum(rel,1, rel[0][0]);


    inverted = GetInvertedIndexinAllR(rel);                        //获取所有元素的倒排索引
    TimeCostForLarge = GetTimeCostForLarge(rel,inverted);
    x = GetSizeBoundary(rel,threshold,minsize,maxsize,TimeCostForLarge);
    printf("The size boundary is %d\n",x);
    fprintf(logFp, "The size boundary is %d\n",x);
    relLarge = NULL;
    relSmall = NULL;
    GetLargeAndSmallsets(rel,x);

    writeSmallToFile(rel,relSmall);

    printf("The number of big set is %d\n",relLarge[0]);
    printf("The number of small set is %d\n",relSmall[0]);
    fprintf(logFp, "The number of big set is %d\n",relLarge[0]);
    fprintf(logFp, "The number of small set is %d\n",relSmall[0]);
    beginLarge = clock();
    printf("begin to process:%3f\n",(beginLarge-start)/(double)CLOCKS_PER_SEC);
    fprintf(logFp, "begin to process:%3f\n",(beginLarge-start)/(double)CLOCKS_PER_SEC);

    rValue = rValue + ScanCountforRl(rel,threshold,relLarge,inverted);

    finishLarge=clock();
    printf("finish large set");
    SizeAwareFreeInverted(inverted);
    finishLarge=clock();

    printf("To process the large set cost %3f seconds!\n", (finishLarge-beginLarge)/(double)CLOCKS_PER_SEC);
    fprintf(logFp, "To process the large set cost %3f seconds!\n", (finishLarge-beginLarge)/(double)CLOCKS_PER_SEC);


    invertedSmall = GetInvertedIndexinRs(rel,relSmall);
//    for(int i=1;i<=invertedSmall[0][0];i++)
//    {
//        cout<<invertedSmall[i][1]<<":";
//        for(int j=2;j<=invertedSmall[i][0];j++)
//        {
//            cout<<invertedSmall[i][j]<<"\t";
//        }
//        cout<<endl;
//    }
    resultSmall = BlockDedup(rel,invertedSmall,threshold,hbt);
    rValue = rValue + sortAndGetResultFromSmall(resultSmall,1, resultSmall[0][0],rel);
    finishSmall = clock();
    printf("To process the small set cost %3f seconds!\n", (finishSmall-finishLarge)/(double)CLOCKS_PER_SEC);
    fprintf(logFp, "To process the small set cost %3f seconds!\n", (finishSmall-finishLarge)/(double)CLOCKS_PER_SEC);

    printf("finish to process:%3f\n",(finishSmall-start)/(double)CLOCKS_PER_SEC);
    fprintf(logFp, "finish to process:%3f\n",(finishSmall-start)/(double)CLOCKS_PER_SEC);

    free(TimeCostForLarge);
    free(hbt);                                        //因为在HeapDedup结束时已经释放了堆中的内容
    SizeAwareFreeInverted(invertedSmall);
    free(relLarge);
    free(relSmall);
    freeRelation(resultSmall);
    end = clock();
    printf("SizeAwareJoin algorithm generates %lld results in %3f seconds!\n", rValue, (end-start)/(double)CLOCKS_PER_SEC);
    fprintf(logFp, "SizeAwareJoin algorithm generates %lld results in %3f seconds!\n", rValue, (end-start)/(double)CLOCKS_PER_SEC);
    fprintf(logs2,"%3f ",(end-start)/(double)CLOCKS_PER_SEC);
    return  rValue;
}


/*
  获取Rs中所有元素的倒排索引,inverted[0][0]表示倒排索引中e元素个数（即多少行），inverted[i][0]-1表示该元素e对应多少个集合R，inverted[i][1]表示
  该元素的值
*/
int  **GetInvertedIndexinRs(int ** rel, int * relSmall)
{
    int ** inverted;
    inverted      = (int**)malloc(sizeof(int*));
    inverted[0]   = (int*) malloc(sizeof(int));
    inverted[0][0]=0;
    //memorySize   += sizeof(int*)+sizeof(int);
    for(int i=1; i<= relSmall[0]; i++)
    {
        for(int j=1; j<= rel[relSmall[i]][0]; j++)
        {
            if(rel[relSmall[i]][j] > inverted[0][0])
            {
                //memorySize += (rel[i][j]-inverted[0][0])*sizeof(int*);
                inverted    = (int **)realloc(inverted, (rel[relSmall[i]][j]+1)*sizeof(int*));
                for(int k= inverted[0][0]+1; k<= rel[relSmall[i]][j]; k++)
                {
                    inverted[k]    = (int*)malloc(sizeof(int)*2);
                    inverted[k][0] = 1;
                    //memorySize    +=sizeof(int);
                }
                inverted[0][0] = rel[relSmall[i]][j];
            }

            //memorySize += sizeof(int);
            inverted[ rel[relSmall[i]][j] ][0]++;
            inverted[ rel[relSmall[i]][j] ][1] = rel[relSmall[i]][j];
            inverted[ rel[relSmall[i]][j] ]  = (int *)realloc(inverted[ rel[relSmall[i]][j] ], (inverted[ rel[relSmall[i]][j] ][0]+2)*sizeof(int));
            inverted[ rel[relSmall[i]][j] ][ inverted[ rel[relSmall[i]][j] ][0] ] =  relSmall[i];
        }
    }
    //to check whether each loaded set is really a set
    int invertLength=inverted[0][0];
    for(int i= inverted[0][0]; i>=1; i--)
    {
        if(validAsetSizeAware(inverted[i]) == 0)
        {
            free(inverted[i]);                   //invalid set is removed
            inverted[i] = NULL;
            invertLength--;                //the # of sets is adjusted
            //inverted[0][0]--;
        }
    }
    int ** fullInverted=(int **)malloc((invertLength+1)*sizeof(int*));
    int currentIndex=1;
    fullInverted[0]=(int*) malloc(sizeof(int));
    fullInverted[0][0]=invertLength;
    for(int i=1; i<=invertLength; i++)
    {
        int j;
        for(j=currentIndex; j<inverted[0][0]; j++)
        {
            if(inverted[j]!=NULL)
            {
                break;
            }
        }
        fullInverted[i]=inverted[j];
        currentIndex=j+1;
    }
    free(inverted);
    return  fullInverted;
}


/*
  获取R中所有元素的倒排索引,inverted[0][0]表示倒排索引中e元素个数（即多少行）,inverted[i][0]表示该元素e对应多少个集合R
*/
int  **GetInvertedIndexinAllR(int ** rel)
{
    int ** inverted;
    inverted      = (int**)malloc(sizeof(int*));
    inverted[0]   = (int*) malloc(sizeof(int));
    inverted[0][0]=0;
    //memorySize   += sizeof(int*)+sizeof(int);
    for(int i=1; i<= rel[0][0]; i++)
    {
        for(int j=1; j<= rel[i][0]; j++)
        {
            if(rel[i][j] > inverted[0][0])
            {
                //memorySize += (rel[i][j]-inverted[0][0])*sizeof(int*);
                inverted    = (int **)realloc(inverted, (rel[i][j]+1)*sizeof(int*));
                for(int k=inverted[0][0]+1; k<= rel[i][j]; k++)
                {
                    inverted[k]    = (int*)malloc(sizeof(int));
                    inverted[k][0] = 0;
                    //memorySize    +=sizeof(int);
                }
                inverted[0][0] = rel[i][j];
            }

            //memorySize += sizeof(int);
            inverted[ rel[i][j] ][0]++;
            inverted[ rel[i][j] ]  = (int *)realloc(inverted[ rel[i][j] ], (inverted[ rel[i][j] ][0]+1)*sizeof(int));
            inverted[ rel[i][j] ][ inverted[ rel[i][j] ][0] ] =  i;
        }
    }


    //to check whether each loaded set is really a set
    /*
    for(int i= inverted[0][0]; i>=1; i--)
    {
        if(validAsetSizeAware(inverted[i]) == 0)
        {
           free(inverted[i]);                   //invalid set is removed
           inverted[i] = inverted[ inverted[0][0] ];
           inverted[0][0] -- ;                  //the # of sets is adjusted
        }
    }*/

    //test
    /*
    printf("all inverted index below:\n");
    printf("%d\n",inverted[0][0]);
    for(int i=1; i<=inverted[0][0]; i++)
    {
        printf("e%d ",i);
        printf("%d ",inverted[i][0]);
        for(int j=1; j<=inverted[i][0]; j++)
        {
            printf("%d ",inverted[i][j]);
        }
        printf("\n");
    }*/

    return  inverted;
}


void   SizeAwareFreeInverted(int ** iIndex)
{
    if(iIndex == NULL || iIndex[0]==NULL)
        return;
    for(int i=1; i<= iIndex[0][0]; i++)
    {
        if(iIndex[i]!=NULL)
        {
            free(iIndex[i]);
        }
    }
    free(iIndex[0]);
    free(iIndex);
}


/*
  对所有的集合R,计算其所包含的元素的倒排索引长度和
*/
int *GetTimeCostForLarge(int **rel, int **inverted)
{
    int *TimeCostForLarge = (int *)malloc(sizeof(int)*rel[0][0]);         //下标从0开始,0对应rel下标为1的集合
    memset(TimeCostForLarge, 0, sizeof(int)*(rel[0][0]));


    for(int i=1; i<=rel[0][0]; i++)
    {
        for(int j=1; j<=rel[i][0]; j++)
        {
            TimeCostForLarge[i-1] = TimeCostForLarge[i-1] + inverted[rel[i][j]][0];

            //printf("TimeCostForLarge[%d] = %d,e is %d,size is %d\n",i,TimeCostForLarge[i-1],rel[i][j],inverted[rel[i][j]][0]);
        }
        //printf("TimeCostForLarge[%d] = %d\n",i,TimeCostForLarge[i-1]);
    }

    return TimeCostForLarge;

}


/*
  用于在GetSizeBoundary函数中估计大集合的timecost
  x为设定的假设sizeboundary
*/
long long EstimateTimeCostForLarge(int *TimeCostForLarge,int x,int **rel)
{
    long long cost = 0;
    for(int i=1; i<=rel[0][0]; i++)
    {
        if(rel[i][0] >= x)
        {
            cost = cost + TimeCostForLarge[i-1];
        }
    }

    return cost;
}


/*
  用于在GetSizeBoundary函数中估计小集合的timecost
  x为设定的假设sizeboundary
*/
long long EstimateTimeCostForSmall(int x,int **rel,int c)         //c为threshold
{
    int indexs = 1;
    int *relSmallTemp = (int *)malloc(sizeof(int));
    long long TotalCost = 0;
    long long ResultGenerationCost = 0;
    long long HeapAndSearchCost = 0;

    relSmallTemp[0] = 0;

    //获得小集合，保存在relSmallTemp中,relSmallTemp[0]保存小集合个数
    for(int i=1; i<=rel[0][0]; i++)
    {
        if(rel[i][0] < x)
        {
            relSmallTemp[0]++;
            relSmallTemp = (int *)realloc(relSmallTemp, (relSmallTemp[0]+1)*sizeof(int));
            relSmallTemp[indexs] = i;
            indexs++;
        }
    }

    int RsSize = relSmallTemp[0];

    int y = RsSize*RsSize*0.001;                                        //抽样次数
    if(RsSize < 2)
    {
        return 0;
    }

    int R1 = 0;                //relSmallTemp中的下标
    int R2 = 0;
    int Pi = 0;

    //抽样y次,循环计算ResultGenerationCost
    srand((unsigned)time(NULL));
    for(int i=1; i<=y; i++)
    {
        R1 = (rand() % RsSize) + 1;
        R2 = (rand() % RsSize) + 1;
        if(R1==R2)
        {
            i--;
            continue;
        }
        else
        {
            Pi = GetIntersectionSize(rel[ relSmallTemp[R1] ],rel[ relSmallTemp[R2] ]);
            //printf("Pi:%d\n",Pi);
            if(Pi < c)
            {
                continue;
            }
            else if(Pi == c)
            {
                ResultGenerationCost = ResultGenerationCost + 1;

            }
            else
            {
                //计算大数量级的组合数
                ResultGenerationCost = ResultGenerationCost + GetCombinatorialNum(c,Pi);
            }

        }
    }


    ResultGenerationCost = (long long)(((double)ResultGenerationCost/y)*RsSize*(RsSize-1)/2);
    cout<<"Result generation:"<<ResultGenerationCost<<endl;
    fprintf(logFp,"Result generation:%lld\n",ResultGenerationCost);
    //针对当前的x得到的relSmallTemp,建立relSmallTemp的倒排索引
    int **invertedSmallTemp;
    invertedSmallTemp = GetInvertedIndexinRs(rel,relSmallTemp);

    int TheNumberOfBlocks = invertedSmallTemp[0][0];                   //块的数目
    int z = TheNumberOfBlocks*0.05;                                                         //块抽样次数
    int b = 0;
    HEAP  *hbtTest =  ( HEAP  *)malloc(sizeof(HEAP));

    for(int i=1; i<=z; i++)
    {
        b = (rand() % TheNumberOfBlocks) + 1;                          //抽样所得block下标
        HeapAndSearchCost = HeapAndSearchCost + BlockDedupForEstimating(rel,invertedSmallTemp,c,hbtTest,b);

    }

    HeapAndSearchCost = (long long)((double)HeapAndSearchCost/z) * TheNumberOfBlocks;

    TotalCost = ResultGenerationCost + HeapAndSearchCost;
    free(relSmallTemp);
    free(hbtTest);
    return TotalCost;
}


/*
  BlockDedup算法  Rtmp中Rtmp[i][0]-1表示这个数组所含元素个数，Rtmp[i][1]表示R的下标,返回对该块的堆调整cost以及二叉查找cost
*/
long long BlockDedupForEstimating(int **rel, int **inverted, int threshold,HEAP *hbt,int b)
{
    int **Rtmp;
    long long HeapAndSearchCost = 0;


    //对每一个在inverted中的I(e),得到Rtmp，然后得到c-subset的倒排索引表
    Rtmp      = (int **)malloc(sizeof(int *));
    Rtmp[0]   = (int *)malloc(sizeof(int));
    Rtmp[0][0]= 0;
    for(int j=2; j<=inverted[b][0]; j++)
    {
        //对每一个inverted[i][j],计算rel[inverted[i][j]]中大于inverted[i][1]的项并付给Rtmp
        int index=rel[inverted[b][j]][0]+1;   //记录rel[inverted[i][j]]中第一个大于e的那个元素的下标（rel[inverted[i][j]]中元素是递增的）
        int ntmp;
        int flag = 0;
        for(int k=1; k<=rel[inverted[b][j]][0]; k++)
        {
            if(rel[inverted[b][j]][k]>inverted[b][1])
            {
                index = k;
                flag = 1;               //用于判断是否找到大于e的index
                break;
            }
        }
        ntmp = rel[inverted[b][j]][0] - index + 1;
        if(ntmp>=threshold-1 && flag==1)                        //加入Rtmp中
        {
            Rtmp[0][0]++;
            Rtmp = (int **)realloc(Rtmp, (Rtmp[0][0]+1)*sizeof(int *));

            Rtmp[Rtmp[0][0]] = (int *)malloc(sizeof(int)*(ntmp+2));
            Rtmp[Rtmp[0][0]][0] = ntmp+1;
            Rtmp[Rtmp[0][0]][1] = inverted[b][j];
            for(int p=2; p<=Rtmp[Rtmp[0][0]][0]; p++)
            {
                Rtmp[Rtmp[0][0]][p] = rel[inverted[b][j]][index++];
            }
        }

    }

    if(Rtmp[0][0]!=0)
    {
        //c为HeapDedup中的c
        int c = threshold-1;
        HeapAndSearchCost = HeapDedupForEstimating(Rtmp,c,hbt);

    }

    freeRelation(Rtmp);



    return HeapAndSearchCost;             //此处是返回c-subset的倒排索引
}

void combination(int *Rs,HEAP *hbt,int num,int len,int start,int *result,int r)
{
    for(int i=start; i<=Rs[0]-num+1; i++)
    {
        result[num-1]=i;
        if(num-1==0)
        {
            CSUBSET myCsubset;
            myCsubset.celement = (int *)malloc(sizeof(int)*len);
            myCsubset.r = r;                               //这里存i而不存Rs[i][1]的原因的为了在后面对slim倒排索引的查找中更方便
            for(int j=len-1; j>=0; j--)
            {
                myCsubset.celement[len-1-j] = Rs[result[j]];
            }
            InsertHeap(hbt,myCsubset,len);
        }
        else
        {
            combination(Rs,hbt,num-1,len,start+1,result,r);
        }
    }
}

/*
  HeapDedup算法
  input:Rs:all the small sets;c:threshold
  output:a slimmed inverted index for Rs
*/
long long HeapDedupForEstimating(int **Rs, int c,HEAP *hbt)
{
    CSUBSET temp;
    int ** slim;
    long long HeapAndSearchCost = 0;
    long long HeapAdjustingCost = 0;
    long long BinarySearchCost = 0;

    //init heap
    InitHeap(hbt,Rs[0][0]);


    slim      = (int **)malloc(sizeof(int *));         //initialize space for slim inverted index
    slim[0]   = (int *)malloc(sizeof(int));            //slim[0]及其后每行第[0]号元素存储[0]后有多少项,然后存放c个元素表示不同c子集,然后存放R
    slim[0][0]=0;
    int *result = (int *)malloc(sizeof(int)*c);
    for(int i=1; i<=Rs[0][0]; i++)
    {

        CSUBSET myCsubset;
        myCsubset.celement = (int *)malloc(sizeof(int)*c);
        myCsubset.r = i;                               //这里存i而不存Rs[i][1]的原因的为了在后面对slim倒排索引的查找中更方便
        for(int j=2; j<=c+1; j++)
        {
            myCsubset.celement[j-2] = Rs[i][j];
            //printf("%d ",myCsubset.celement[j-2]);
        }

        InsertHeap(hbt,myCsubset,c);
        HeapAdjustingCost = HeapAdjustingCost + c*log(hbt->len);
    }



    //printf("heap len = %d and Heap below\n",hbt->len);

    temp = hbt->csubset[0];
    while (!EmptyHeap(hbt))                                 //依次删除堆顶元素并显示出来，直到堆空为止
    {
        CSUBSET rcmin,rctop,rcR;                            //rcR为经折半查找找到的在R中不小于rctop的csubset
        int Rtemp = 0;
        HeapAdjustingCost = HeapAdjustingCost + (c+1)*log(hbt->len);
        //pop H to get rc(min) and suppose it is from R
        rcmin = DeleteHeap(hbt,c);

        rctop = hbt->csubset[0];

        //append R to slim inverted index[rc(min)]
        if(slim[0][0]==0 || compareSetsSa(rcmin.celement,temp.celement,c)==1)                  //如果新弹出的rcmin比temp大或者slim[0][0]==0,则添加slim[i]
        {
            slim[0][0]++;
            slim = (int **)realloc(slim, (slim[0][0]+1)*sizeof(int *));
            slim[slim[0][0]] = (int *)malloc(sizeof(int)*(c+1));
            slim[slim[0][0]][0] = c;
            for(int i=1; i<=c; i++)
            {
                slim[slim[0][0]][i] = rcmin.celement[i-1];
            }
        }

        slim[slim[0][0]][0]++;
        slim[slim[0][0]] = (int *)realloc(slim[slim[0][0]], (slim[slim[0][0]][0]+1)*sizeof(int));
        slim[slim[0][0]][slim[slim[0][0]][0]] = rcmin.r;

        if(compareSetsSa(rcmin.celement,rctop.celement,c)!=0)
        {
            //foreach R in slim[rcmin]
            for(int i=c+1; i<=slim[slim[0][0]][0]; i++)
            {
                Rtemp = slim[slim[0][0]][i];
                rcR = binarySearchCsubset(Rs,rctop,Rtemp,c);
                BinarySearchCost = BinarySearchCost + (c+1)*log(Rs[Rtemp][0]-1);

                //没找到,不插入
                if(rcR.r==0)
                {
                    continue;
                }
                else
                {
                    InsertHeap(hbt,rcR,c);
                    HeapAdjustingCost = HeapAdjustingCost + (c+1)*log(hbt->len);
                }


            }
            //free(temp.celement);
            temp = rcmin;                         //记录上一次pop的rcmin

        }
        else
        {
            //free(temp.celement);
            temp = rcmin;                         //记录上一次pop的rcmin
            continue;
        }



    }

    //printf("\n");

    ClearHeap(hbt);
    HeapAndSearchCost = HeapAdjustingCost + BinarySearchCost;
    freeRelation(slim);
    free(result);
    return HeapAndSearchCost;
}

/*
  get size boundary x   c为threshold
*/
int GetSizeBoundary(int **rel, int c, int minsize, int maxsize, int *TimeCostForLarge)
{

    int x = c;


    long long benefit = 0;
    long long cost = 0;
    long long m = 0;           //对应z`
    long long n = 0;           //对应y`
    long long p = 0;           //对应z
    long long q = 0;           //对应y
    int k=3,index=0;
    long long benefits[k];
    long long costs[k];

    if(minsize > c)
    {
        x = minsize;
    }

    m = EstimateTimeCostForSmall(x,rel,c);
    n = EstimateTimeCostForLarge(TimeCostForLarge,x,rel);


    while(x <= maxsize)
    {
        p = EstimateTimeCostForSmall(x+1,rel,c);
        q = EstimateTimeCostForLarge(TimeCostForLarge,x+1,rel);

        printf("x:%d,p=%lld,q=%lld\n",x+1,p,q);
        fprintf(logFp, "x:%d,p=%lld,q=%lld\n",x,p,q);

        benefit = n - q;

        if(p<=m)
        {
            p=(p+m)/2;
            cost=cost*1.5;
        }
        else
        {

            cost = p - m;
        }
        if(benefit <= cost)
        {
            break;
        }

        x++;

        n = q;
        m = p;
    }
    //x = 38;
    return x;
}



/*
  返回两个集合相同元素个数
*/

int   GetIntersectionSize(int *r, int *s)
{
    int rValue = 0;
    int rPos, sPos, i;
    rPos =1;
    sPos =1;
    while(rPos<= r[0] && sPos<= s[0])
    {
        if(r[rPos]== s[sPos])
        {
            rValue++;
            rPos++;
            sPos++;
            continue;
        }
        if(r[rPos]< s[sPos])
        {
            i= binarySearch(r, s[sPos], rPos+1);
            if(i==-1)
            {
                rPos++;
                sPos++;
            }
            else
            {
                rValue++;
                rPos = i+1;
                sPos++;
            }
        }
        else
        {
            i= binarySearch(s, r[rPos], sPos+1);
            if(i==-1)
            {
                rPos++;
                sPos++;
            }
            else
            {
                rValue++;
                sPos = i+1;
                rPos++;
            }
        }
    }
    return rValue;
}


/*
   求组合数
*/

long long GetCombinatorialNum(int m, int n)
{
    long long ans = 1;
    if(m > n-m)
        m = n-m;
    for(int i = n-m+1; i <= n; i++)
        ans *= i;
    for(int j = 1; j <= m; j++)
        ans /= j;
    //cout<<m<<","<<n<<endl;
    return ans;

}


/*
  获取指向各个large和small set的索引
*/
void GetLargeAndSmallsets(int **rel, int x)
{
    int indexl = 1;
    int indexs = 1;
    relLarge = (int *)malloc(sizeof(int));
    relSmall = (int *)malloc(sizeof(int));
    relLarge[0] = 0;
    relSmall[0] = 0;
    for(int i=1; i<=rel[0][0]; i++)
    {
        if(rel[i][0]>=x)
        {
            relLarge[0]++;
            relLarge = (int *)realloc(relLarge, (relLarge[0]+1)*sizeof(int));
            relLarge[indexl] = i;
            indexl++;
        }

        if(rel[i][0]<x)
        {
            relSmall[0]++;
            relSmall = (int *)realloc(relSmall, (relSmall[0]+1)*sizeof(int));
            relSmall[indexs] = i;
            indexs++;
        }
    }
}


/*
  deal with large sets,get results by large sets,这个地方去重可以用relLarge[0]*relLarge[0]的矩阵,但是内层循环要判断是否来自relLarge
  暴力法
*/
/*
int GetResultsFromLargesets(int **rel, int threshold, int *relLarge)
{
    int isSimilaritySets = 0;
    int rValue = 0;
    int **Processed;

    Processed  = (int **)malloc(sizeof(int *)*(rel[0][0]));

    for(int i=0;i<rel[0][0];i++)
    {
        Processed[i] = (int *)malloc(sizeof(int)*(rel[0][0]));
        memset(Processed[i], 0, sizeof(int)*(rel[0][0]));
    }
    for(int i=1;i<=relLarge[0];i++)
    {
        for(int j=1;j<=rel[0][0];j++)
        {
            if(relLarge[i]==j)
            {
                continue;
            }
            isSimilaritySets = verifyIntersection(rel[relLarge[i]],rel[j],threshold);

            if(isSimilaritySets==1 && Processed[relLarge[i]-1][j-1]==0 && Processed[j-1][relLarge[i]-1]==0)                                //是相似集
            {
                rValue++;
                Processed[relLarge[i]-1][j-1] = 1;
                Processed[j-1][relLarge[i]-1] = 1;
                printf("results from large sets:");
                printf("(%d,%d)\n",relLarge[i],j);
            }
        }
    }

    //释放Processed占用的内存
    for(int i=0; i< rel[0][0]; i++)
    {
        if(Processed[i]==NULL)continue;
        free(Processed[i]);
    }
    free(Processed);

    return rValue;
}
*/

/*
  针对Rlarge以及全集R的Scancount算法
*/

int ScanCountforRl(int **rel, int threshold, int *relLarge, int **inverted)
{
    int rValue = 0;

    //ScanCount
    //对于每个largeSet
//    for(int i=1;i<=inverted[0][0];i++)
//    {
//        cout<<i<<":";
//        for(int j=1;j<=inverted[i][0];j++)
//        {
//            cout<<inverted[i][j]<<"\t";
//        }
//        cout<<endl;
//    }
    for(int i=1; i<=relLarge[0]; i++)
    {
        int *countR;
        //each time a same malloc, move from here outside the loop, just keep memset is ok
        countR = (int *)malloc(sizeof(int)*(rel[0][0]+1));
        memset(countR, 0, sizeof(int)*(rel[0][0]+1));
        countR[0] = rel[0][0];

        for(int j=1; j<=rel[relLarge[i]][0]; j++)
        {
            for(int k=1; k<=inverted[rel[relLarge[i]][j]][0]; k++)
            {
                /****
                // if( inverted[rel[relLarge[i]][j]][k] <= relLarge[i] || countR[inverted[rel[relLarge[i]][j]][k]]<0 )
                //{
                //    continue;
                //}   ***/
                countR[inverted[rel[relLarge[i]][j]][k]]++;
                //cout<<inverted[rel[relLarge[i]][j]][k]<<"-"<<relLarge[i]<<":"<<countR[inverted[rel[relLarge[i]][j]][k]]<<endl;
                if(relLarge[i]>inverted[rel[relLarge[i]][j]][k] && countR[inverted[rel[relLarge[i]][j]][k]] == threshold)
                    //if(relLarge[i]!=inverted[rel[relLarge[i]][j]][k] && countR[inverted[rel[relLarge[i]][j]][k]] == threshold && Processed[inverted[rel[relLarge[i]][j]][k]-1][relLarge[i]-1] == 0 && Processed[relLarge[i]-1][inverted[rel[relLarge[i]][j]][k]-1] == 0)
                {
                    rValue++;
#ifdef  L_debug
                    fprintf(sizeawarelog,"results from large sets:");
                    fprintf(sizeawarelog,"(%d,%d)\n",relLarge[i],inverted[rel[relLarge[i]][j]][k]);
#endif // L_debug
                    //Processed[inverted[rel[relLarge[i]][j]][k]-1][relLarge[i]-1] = 1;
                    //Processed[relLarge[i]-1][inverted[rel[relLarge[i]][j]][k]-1] = 1;
                    /******/
                    //countR[inverted[rel[relLarge[i]][j]][k]] = -1;
                    /******/
                }
            }
        }
        free(countR);
    }


    return rValue;


}


/*
  BlockDedup算法  Rtmp中Rtmp[i][0]-1表示这个数组所含元素个数，Rtmp[i][1]表示R的下标
  psize为relSmall[0]的值,即小集合数目
*/
int **BlockDedup(int **rel, int **inverted, int threshold,HEAP *hbt)
{
    int **Rtmp;
    int ** slim;
    //int rValue = 0;

    int **resultSmall = (int **)malloc(sizeof(int *));            //存放结果的二元组,从下标为1开始
    resultSmall[0] = (int *)malloc(sizeof(int));
    resultSmall[0][0] = 0;


    //对每一个在inverted中的I(e),得到Rtmp，然后得到c-subset的倒排索引表
    for(int i=1; i<=inverted[0][0]; i++)
    {
        Rtmp      = (int **)malloc(sizeof(int *));
        Rtmp[0]   = (int *)malloc(sizeof(int));
        Rtmp[0][0]= 0;
        for(int j=2; j<=inverted[i][0]; j++)
        {
            //对每一个inverted[i][j],计算rel[inverted[i][j]]中大于inverted[i][1]的项并付给Rtmp
            int index=0;   //记录rel[inverted[i][j]]中第一个大于e的那个元素的下标（rel[inverted[i][j]]中元素是递增的）
            int ntmp;
            int flag = 0;
            for(int k=1; k<=rel[inverted[i][j]][0]; k++)
            {
                if(rel[inverted[i][j]][k]>inverted[i][1])
                {
                    index = k;
                    flag = 1;               //用于判断是否找到大于e的index
                    break;
                }
            }
            ntmp = rel[inverted[i][j]][0] - index + 1;
            if(ntmp>=threshold-1 && flag==1)                        //加入Rtmp中
            {
                Rtmp[0][0]++;
                Rtmp = (int **)realloc(Rtmp, (Rtmp[0][0]+1)*sizeof(int *));

                Rtmp[Rtmp[0][0]] = (int *)malloc(sizeof(int)*(ntmp+2));
                Rtmp[Rtmp[0][0]][0] = ntmp+1;
                Rtmp[Rtmp[0][0]][1] = inverted[i][j];
                for(int p=2; p<=Rtmp[Rtmp[0][0]][0]; p++)
                {
                    Rtmp[Rtmp[0][0]][p] = rel[inverted[i][j]][index];
                    index++;
                }
            }

        }

        //Rtmp不为空
#ifdef  L_debug
        fprintf(sizeawarelog,"all Rtmp sets below for I(%d):\n",inverted[i][1]);
        fprintf(sizeawarelog,"%d\n",Rtmp[0][0]);
        for(int m=1; m<=Rtmp[0][0]; m++)
        {
            fprintf(sizeawarelog,"%d ",Rtmp[m][0]);
            for(int j=1; j<=Rtmp[m][0]; j++)
            {
                fprintf(sizeawarelog,"%d ",Rtmp[m][j]);
            }
            fprintf(sizeawarelog,"\n");
        }
#endif // L_debug

        if(Rtmp[0][0]>1)
        {
            //c为HeapDedup中的c
            int c = threshold-1;
//            for(int j=1; j<=Rtmp[0][0]; j++)
//            {
//                cout<<inverted[i][1]<<"-"<<Rtmp[j][1]<<":";
//                for(int k=2; k<=Rtmp[j][0]; k++)
//                {
//                    cout<<Rtmp[j][k]<<"\t";
//                }
//                cout<<endl;
//            }
            slim = HeapDedup(Rtmp,c,hbt);

#ifdef  L_debug
            fprintf(sizeawarelog,"all slim inverted index below:\n");
            fprintf(sizeawarelog,"%d\n",slim[0][0]);
            for(int m=1; m<=slim[0][0]; m++)
            {
                fprintf(sizeawarelog,"%d ",slim[m][0]);
                for(int j=1; j<=slim[m][0]; j++)
                {
                    fprintf(sizeawarelog,"%d ",slim[m][j]);
                }
                fprintf(sizeawarelog,"\n");
            }
#endif // L_debug

            for(int k=1; k<=slim[0][0]; k++)
            {
                //各个c-subset的倒排索引中有不止c+1个元素(前c个元素存c-subset,后面存集合),即集合大于1个
                if(slim[k][0]>c+1)
                {
                    //从slim下标c+1开始
                    for(int j=c+1; j<=slim[k][0]-1; j++)
                    {
                        for(int m=j+1; m<= slim[k][0]; m++)
                        {
#ifdef  L_debug
                            fprintf(sizeawarelog,"results from small sets:");
                            fprintf(sizeawarelog,"(%d,%d)\n",Rtmp[slim[k][j]][1],Rtmp[slim[k][m]][1]);
#endif // L_debug
                            resultSmall[0][0]++;

                            resultSmall = (int **)realloc(resultSmall, (resultSmall[0][0]+1)*sizeof(int *));
                            resultSmall[resultSmall[0][0]] = (int *)malloc(sizeof(int)*2);

                            if(Rtmp[slim[k][j]][1] > Rtmp[slim[k][m]][1])
                            {
                                //存结果二元组,供后续排序使用,第一位比第二位大
                                resultSmall[resultSmall[0][0]][0] = Rtmp[slim[k][j]][1];
                                resultSmall[resultSmall[0][0]][1] = Rtmp[slim[k][m]][1];
                            }
                            if(Rtmp[slim[k][j]][1] < Rtmp[slim[k][m]][1])
                            {
                                //存结果二元组,供后续排序使用,第一位比第二位大
                                resultSmall[resultSmall[0][0]][1] = Rtmp[slim[k][j]][1];
                                resultSmall[resultSmall[0][0]][0] = Rtmp[slim[k][m]][1];
                            }

                        }
                    }
                }
            }

        }

        if(Rtmp[0][0]>1)
        {
            freeRelation(slim);
        }
        freeRelation(Rtmp);
    }

    return resultSmall;
}

/*
  HeapDedup算法
  input:Rs:all the small sets;c:threshold
  output:a slimmed inverted index for Rs
*/
int **HeapDedup(int **Rs, int c,HEAP *hbt)
{
    CSUBSET temp;
    int ** slim;
    //int flag = 0;
    //init heap
    InitHeap(hbt,Rs[0][0]);


    slim      = (int **)malloc(sizeof(int *));         //initialize space for slim inverted index
    slim[0]   = (int *)malloc(sizeof(int));            //slim[1]及其后每行第[0]号元素存储[0]后有多少项,然后存放c个元素表示不同c子集,然后存放R
    slim[0][0]=0;

    //insert all the min-subsets of small sets to H
    fprintf(sizeawarelog,"InsertHeap first:\n");
    for(int i=1; i<=Rs[0][0]; i++)
    {
        CSUBSET myCsubset;
        myCsubset.celement = (int *)malloc(sizeof(int)*c);
        myCsubset.r = i;                               //这里存i而不存Rs[i][1]的原因的为了在后面对slim倒排索引的查找中更方便


        //fprintf(sizeawarelog,"InsertHeap first:");
        for(int j=2; j<=c+1; j++)
        {
            myCsubset.celement[j-2] = Rs[i][j];
            //fprintf(sizeawarelog,"%d ",myCsubset.celement[j-2]);
        }
        //fprintf(sizeawarelog,"(Rs = %d)\n",myCsubset.r);
        //fprintf(sizeawarelog,"\n");
        InsertHeap(hbt,myCsubset,c);
    }

    //printf("heap len = %d and Heap below\n",hbt->len);
    /*
    for(int i=0;i<hbt->len;i++)
    {
        for(int j=0;j<c;j++)
        {
            printf("%d ",hbt->csubset[i].celement[j]);
        }
        printf("\n");
    }*/
    temp=hbt->csubset[0];
    while (!EmptyHeap(hbt))                                 //依次删除堆顶元素并显示出来，直到堆空为止
    {
        CSUBSET rcmin,rctop,rcR;                            //rcR为经折半查找找到的在R中不小于rctop的csubset
        int Rtemp = 0;

        //pop H to get rc(min) and suppose it is from R
        rcmin = DeleteHeap(hbt,c);

        //rctop = hbt->csubset[0];

#ifdef  L_debug
        fprintf(sizeawarelog,"POP:");
        for(int j=0; j<c; j++)
        {
            fprintf(sizeawarelog,"%d ",rcmin.celement[j]);
        }
        fprintf(sizeawarelog,"(Rs = %d)\n",rcmin.r);
#endif

        //append R to slim inverted index[rc(min)]
        if(slim[0][0]==0 || compareSetsSa(rcmin.celement,temp.celement,c)!=0)                  //如果新弹出的rcmin比temp大或者slim[0][0]==0,则添加slim[i]
        {
            /*for test
            printf("rcmin :");
            for(int i=0;i<c;i++)
            {
                printf("%d ",rcmin.celement[i]);
            }
            printf("(Rs = %d)\n",rcmin.r);

            printf("temp :");
            for(int i=0;i<c;i++)
            {
                printf("%d ",temp.celement[i]);
            }
            printf("(Rs = %d)\n",temp.r);

            printf("lrx111111 slim[0][0]=%d  compare=%d\n",slim[0][0],compareSetsSa(rcmin.celement,temp.celement,c));*/

            slim[0][0]++;
            slim = (int **)realloc(slim, (slim[0][0]+1)*sizeof(int *));
            slim[slim[0][0]] = (int *)malloc(sizeof(int)*(c+1));
            slim[slim[0][0]][0] = c;
            for(int i=1; i <=c; i++)
            {
                slim[slim[0][0]][i] = rcmin.celement[i-1];
            }
        }
        int index = slim[0][0];
        int number = slim[index][0]+1;
        slim[index][0]++;
        slim[index] = (int *)realloc(slim[index], (number+1)*sizeof(int));
        slim[index][number] = rcmin.r;
        if(!EmptyHeap(hbt))
        {
            //flag = 0;
            rctop = hbt->csubset[0];
        }
        else
        {
            //flag = 1;
            continue;
        }

        if(compareSetsSa(rcmin.celement,rctop.celement,c)!=0)
        {
            //foreach R in slim[rcmin]
            for(int i=c+1; i<=slim[slim[0][0]][0]; i++)
            {
                Rtemp = slim[slim[0][0]][i];
                rcR = binarySearchCsubset(Rs,rctop,Rtemp,c);
                //没找到,不插入
                if(rcR.r==0)
                {
                    continue;
                }
                else
                {
#ifdef  L_debug
                    fprintf(sizeawarelog,"InsertHeap rcR:");
                    for(int i=0; i<c; i++)
                    {
                        fprintf(sizeawarelog,"%d ",rcR.celement[i]);
                    }
                    fprintf(sizeawarelog,"(Rs = %d)\n",rcR.r);
#endif
                    InsertHeap(hbt,rcR,c);
                }


            }

            //free(temp.celement);
            temp = rcmin;                         //记录上一次pop的rcmins
        }

        else
        {
            //free(temp.celement);
            temp = rcmin;                         //记录上一次pop的rcmin
            continue;
        }

    }

    //printf("\n");

    ClearHeap(hbt);
    return slim;
}

void output(CSUBSET myCsubset,int c)
{
    for(int i=0;i<c;i++)
    {
        cout<<myCsubset.celement[i]<<"\t";
    }
    cout<<endl;
}
/*
  在Rs中找到一个不小于rctop的c子集
*/
CSUBSET binarySearchCsubset(int **Rs, CSUBSET rctop, int Rtemp, int c)
{
    //
    CSUBSET rcR;
    binarySearchFlag = -1;
    //int binarySearchFlag2 = -1;
    int /**lastResult,**/ j, targetCpy, posCpy;
    //int myflag = 0;
    int result = -1;
    int sPos = 2;                   //Rs中每一行从2号下标开始,1号下标存的是Rs在全集中的下标

    //int rctopScript = 0;
    //int RsScript = 0;

    rcR.celement = (int *)malloc(sizeof(int)*c);
    rcR.r = Rtemp;                                     //这里存i而不存Rs[i][1]的原因的为了在后面对slim倒排索引的查找中更方便

    //binarySearchFlag = 0;

    //对rctop，从e1开始在Rs中二分查找，找到了就接着找下一个，直到找不到ei就在Rs中得到一个大于ei的最小的eai的下标a
    //ei到ec这c-i+1个元素由Rs中的a到a+c-i下标组成，如果a+c-i越界了，那么在Rs中找不到不小于rctop的c子集，就不插入
    result = -1;
    posCpy = sPos;
    targetCpy=-1;
    for(int i=0; i<c; i++)
    {
        //lastResult = result;
        result = binarySearchSa(Rs[Rtemp],rctop.celement[i],sPos);

        if(binarySearchFlag==1)
        {
            /**e_i = Rs[Rtemp][result] ******************/

            if(Rs[Rtemp][0]-result >= c-i)
            {
                /***if e_i is ignored, we can still obtain the c-subset by copying following eles***/
                targetCpy = i;
                posCpy    = result+1;
            }

            if((Rs[Rtemp][0]-result+1) < (c-i-1))
            {
                /****even we copy the eles following Rs[Rtemp][result], we could not obtain a c-subset***/
                /****it is unnecessary to search further                                              ***/
                if(targetCpy>=0)/** a c-subset have been found already***/
                {
                    for(j =0; targetCpy+j<c; j++)
                    {
                        rcR.celement[targetCpy+j] = Rs[Rtemp][posCpy+j];
                    }
                    return rcR;
                }
                else/***(lastResult==-1) || posCpy == -1, copy is also not possible***/
                {
                    /**such c-subset does not exist here**/
                    rcR.r = 0;
                    return rcR;
                }
            }
            else
            {
                sPos = result + 1;
                rcR.celement[i] = rctop.celement[i];
                continue;
            }
        }
        else if(binarySearchFlag==0)
        {
            /**e_i does not appear in set Rs**/
            if(result<= Rs[Rtemp][0])
            {
                /***Rs[Rtemp][0] > e_i**/
                if(Rs[Rtemp][0]-result < c-i-1)
                {
                    /****but from here can not match a c-subset***/
                    if(targetCpy >=0)
                    {
                        for(j =0; targetCpy+j<c; j++)
                        {
                            rcR.celement[targetCpy+j] = Rs[Rtemp][posCpy+j];
                        }
                        return rcR;
                    }
                    else /***(lastResult == -1)***/
                    {
                        /******no space to backtrack****/
                        rcR.r =0;
                        return rcR;
                    }
                }
                else
                {
                    /****from here can match a c-subset***/
                    for(j =0; i+j<c; j++)
                    {
                        rcR.celement[i+j] = Rs[Rtemp][result+j];
                    }
                    return rcR;

                }
            }
            else   /***Rs[Rtemp][...]<e_i****/
            {
                if(targetCpy >=0)
                {
                    for(j =0; targetCpy+j<c; j++)
                    {
                        rcR.celement[targetCpy+j] = Rs[Rtemp][posCpy+j];
                    }
                    return rcR;
                }
                else /***(lastResult == -1)***/
                {
                    rcR.r =0;
                    return rcR;
                }
            }
        }
    }
    return rcR;
}

int binarySearchSa( int * data, int target, int sPos)
/******************************************************************/
/*******to search target in data between sPos and end    **********/
/*******If found, return mid                             **********/
/*******Else      return low                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           **********/
/******************************************************************/
{
    int mid;
    int low, high;
    low  = sPos;
    high = data[0];
    while(low <= high)
    {
        mid = (low+high)/2;
        if(target == data[mid])
        {
            binarySearchFlag = 1;
            return mid;
        }
        else if(target < data[mid])
        {
            high = mid-1;
        }
        else
        {
            low = mid+1;
        }
    }
    binarySearchFlag = 0;
    //return a position pos such that   data[pos]<= target
    return low;
}

int  compareSetsSa(int * fSet, int * sSet,int c)    //c为fSet和sSet的长度
/***************************************************************************/
/****to compare two ordered sets lexicographically                  ********/
/**** return 0, if the two sets are the same                        ********/
/**** return 1, if fSet> sSet                                       ********/
/**** return -1, otherwise                                          ********/
/***************************************************************************/
{
    int i;
    i=0;
    while(i<c)
    {
        if(fSet[i] < sSet[i])
        {
            return -1;
        }
        else if(fSet[i] > sSet[i])
        {
            return 1;
        }
        i++;
    }
    return 0;
}

int    validAsetSizeAware(int * aSet)
/******************************************************************/
/*********to remove repeated items from the set and then **********/
/*********to determine whether a set is valid            **********/
/**********return 0, if it is an empty set               **********/
/**********return 1, otherwise                           **********/
/******************************************************************/
{
    if(aSet == NULL || aSet[0] == 1)
    {
        return 0;
    }
    return 1;
}


//1、初始化堆
void InitHeap(struct HEAP* hbt, int ms)
{
    if (ms <= 0)
    {
        printf("数组长度参数不合适，需重新给定！\n");
        exit(1);
    }
    hbt->csubset = (CSUBSET *)malloc(sizeof(CSUBSET)*ms);
    if (!hbt->csubset)
    {
        printf("用于动态分配的内存空间用完，退出运行！\n");
        exit(1);
    }
    hbt->maxsize = ms;
    hbt->len = 0;
}

//2、清除堆
void ClearHeap(struct HEAP* hbt)
{
    if (hbt->csubset != NULL)
    {
        for(int i=0; i<hbt->len; i++)
        {
            free(hbt->csubset[i].celement);
        }
        free(hbt->csubset);
        hbt->len = 0;
        hbt->maxsize = 0;
    }
}

//3、检查一个堆是否为空
int EmptyHeap(struct HEAP* hbt)
{
    if (hbt->len == 0)
        return 1;
    else
        return 0;
}



//4、向堆中插入一个元素
void InsertHeap(struct HEAP* hbt, CSUBSET x, int c)
{
    int i;
    if (hbt->len == hbt->maxsize)                     //若堆满，将数组空间扩展为原来的2倍
    {
        CSUBSET *p;
        p = (CSUBSET *)realloc(hbt->csubset, 2*hbt->maxsize*sizeof(CSUBSET));
        if (!p)
        {
            printf("存储空间用完！\n");
            exit(1);
        }
        //printf("存储空间已扩展为原来的2倍！\n");
        hbt->csubset = p;
        hbt->maxsize = 2*hbt->maxsize;
    }
    hbt->csubset[hbt->len] = x;                     //向堆尾添加新元素
    hbt->len++;                                     //堆长度加1
    i = hbt->len - 1;                               //i指向待调整元素的位置，即其数组下标，初始指向新元素所在的堆尾位置
    while (i != 0)
    {
        int j = (i - 1) / 2;                        //j指向下标为i的元素的双亲
        //if (x >= hbt->csubset[j]) //若新元素大于待调整元素的双亲，则比较调整结束，退出循环
        if(compareSetsSa(x.celement,hbt->csubset[j].celement,c)==1 || compareSetsSa(x.celement,hbt->csubset[j].celement,c)==0)
        {
            break;
        }
        hbt->csubset[i] = hbt->csubset[j];          //将双亲元素下移到待调整元素的位置
        i = j;                                      //使待调整位置变为其双亲位置，进行下一次循环
    }
    hbt->csubset[i] = x;                            //把新元素调整到最终位置
}

//5、从堆中删除堆顶元素并返回
CSUBSET DeleteHeap(struct HEAP* hbt, int c)
{
    CSUBSET temp, x;
    int i, j;
    /*
    if (hbt->len == 0)
    {
        //printf("堆已空，退出运行！\n");
        //exit(1);
        return 0;                                   //堆空返回0
    }*/
    temp = hbt->csubset[0];                         //暂存堆顶元素
    hbt->len--;
    if (hbt->len == 0)                              //若删除操作后堆为空则返回
        return temp;
    x = hbt->csubset[hbt->len];                     //将待调整的原堆尾元素暂存x中，以便放入最终位置
    i = 0;                                          //用i指向待调整元素的位置，初始指向堆顶位置
    j = 2 * i + 1;                                  //用j指向i的左孩子位置，初始指向下标为1的位置
    while (j <= hbt->len - 1)                       //寻找待调整元素的最终位置，每次使孩子元素上移一层，调整到孩子为空时止
    {
        //if (j < hbt->len - 1 && hbt->csubset[j] > hbt->csubset[j+1])//若存在右孩子且较小，使j指向右孩子
        if(j < hbt->len - 1 && compareSetsSa(hbt->csubset[j].celement,hbt->csubset[j+1].celement,c)==1)
            j++;
        //if (x <= hbt->csubset[j]) //若x比其较小的孩子还小，则调整结束，退出循环
        if(compareSetsSa(hbt->csubset[j].celement,x.celement,c)==1 || compareSetsSa(hbt->csubset[j].celement,x.celement,c)==0)
            break;
        hbt->csubset[i] = hbt->csubset[j];          //否则，将孩子元素移到双亲位置
        i = j;                                      //将待调整位置变为其较小的孩子位置
        j = 2 * i + 1;                              //将j变为新的待调整位置的左孩子位置，继续下一次循环
    }
    hbt->csubset[i] = x;                            //把x放到最终位置
    return temp;                                    //返回原堆顶元素
}


/*

*/
/*
void   sortRelSetByNum(int **rel, int startIndex, int endIndex)
{
    int i,j,pos;
    int  *temp;
   // printf("%d %d\n",startIndex,endIndex);
    if(startIndex >= endIndex) return;

    pos         = startIndex + rand()%(endIndex - startIndex);            //randomized position between start and end
    temp        = rel[pos];                                      //swap element
    rel[pos]    = rel[endIndex];
    rel[endIndex]    = temp;

    i = startIndex-1;                                            //to partion
    for(j=startIndex; j<endIndex; j++)
    {
        if( rel[j][0] <= rel[endIndex][0])
        {
            i++;
            temp      = rel[j];
            rel[j]    = rel[i];
            rel[i]    = temp;
        }
    }
    i++;
    temp        = rel[endIndex];
    rel[endIndex]    = rel[i];
    rel[i]      = temp;

    sortRelSetByNum(rel, startIndex, i-1);                       //to iterate
    sortRelSetByNum(rel, i+1, endIndex);
}
*/

/*
   用非递归的快排排序
*/
/*
//初始化栈
void  InitStack(MyStack *s, int **rel)
{
	int *relList = (int*)malloc(rel[0][0] * sizeof(int));
	if (relList == NULL)
	{
		return;
	}
	s->relList = relList;
	s->StackSize = 0;
}

//将元素加入栈
void PushStack(MyStack *s,int d,int **rel)
{
	if (s->StackSize > rel[0][0])
	{
		return;
	}
	else
    {
        s->relList[s->StackSize++] = d;
    }
}

//POP
void PopStack(MyStack *s)
{
	if (s->StackSize == 0)
	{
		return;
	}
	else
		s->StackSize--;
}

//返回栈顶元素中的集合大小
int TopStack(MyStack *s)
{
	return s->relList[s->StackSize-1];

}

//Empty
int EmptyStack(MyStack *s)
{
	return s->StackSize == 0;
}

//获得pivot
int SizeAwarePartition(int **rel, int left, int right)
{
    int *pivot = rel[left];
    while (left < right)
    {
        while (left < right && rel[right][0] >= pivot[0])
        {
            right--;
        }
        if(left < right)
        {
            rel[left++] = rel[right];
        }
        while (left < right && rel[left][0] <= pivot[0])
        {
            left++;
        }
        if(left < right)
        {
            rel[right--] = rel[left];
        }
    }
    rel[left] = pivot;
    return left;
}*/

void sortRelSetByNum(int **rel, int start, int end)
{
    int * sBegin, *eEnd, low, high,pos, *ptrTemp,m, i,j,flag;

    m=0;

    sBegin  = (int*) malloc(rel[0][0] * sizeof(int));
    eEnd    = (int*) malloc(rel[0][0] * sizeof(int));

    sBegin[0] = start;
    eEnd[0]   = end;
    srand(time(NULL));
    while(m >= 0)
    {
        low = sBegin[m];   //pop an iteration
        high= eEnd[m];
        m--;

        if(low >= high)
        {
            continue;
        }
        pos = low + rand()%(high-low+1);
        ptrTemp   = rel[high];
        rel[high] = rel[pos];
        rel[pos]  = ptrTemp;

        flag = 0;
        i = low-1;
        for(j=low; j<high; j++)
        {
            if(rel[j][0] <= rel[high][0])
            {
                i++;

                if(rel[j][0] == rel[high][0])
                {
                    flag++;
                }
                if(i==j)
                {
                    continue;
                }
                ptrTemp   = rel[j];
                rel[j]    = rel[i];
                rel[i]    = ptrTemp;
            }
        }
        if( flag == high-low)
        {
            continue;   //all between low and high equals to the partition element
        }
        i++;
        ptrTemp    = rel[high];
        rel[high]  = rel[i];
        rel[i]     = ptrTemp;

        m++;   //push
        sBegin[m] = low;
        eEnd[m]   = i-1;
        m++;
        sBegin[m] = i+1;
        eEnd[m]   = high;
    }

    free(sBegin);
    free(eEnd);
}


int sortAndGetResultFromSmall(int **rel, int start, int end,int **relS)
{
    int * sBegin, *eEnd, low, high,pos, *ptrTemp,m, i,j,flag,rValue;

    m=0;

    sBegin  = (int*) malloc(rel[0][0] * sizeof(int));
    eEnd    = (int*) malloc(rel[0][0] * sizeof(int));

    sBegin[0] = start;
    eEnd[0]   = end;
    srand(time(NULL));
    while(m >= 0)
    {
        low = sBegin[m];   //pop an iteration
        high= eEnd[m];
        m--;

        if(low >= high)
        {
            continue;
        }
        pos = low + rand()%(high-low+1);

        ptrTemp   = rel[high];
        rel[high] = rel[pos];
        rel[pos]  = ptrTemp;

        flag = 0;
        i = low-1;
        for(j=low; j<high; j++)
        {
            //if(rel[j][0] <= rel[high][0])
            if(compareSetsSa(rel[j],rel[high],2) <= 0)
            {
                i++;

                //if(rel[j][0] == rel[high][0]){flag++;}
                if(compareSetsSa(rel[j],rel[high],2) == 0)
                {
                    flag++;
                }
                if(i==j)
                {
                    continue;
                }
                ptrTemp   = rel[j];
                rel[j]    = rel[i];
                rel[i]    = ptrTemp;
            }
        }
        if( flag == high-low)
        {
            continue;   //all between low and high equals to the partition element
        }
        i++;
        ptrTemp    = rel[high];
        rel[high]  = rel[i];
        rel[i]     = ptrTemp;

        m++;   //push
        sBegin[m] = low;
        eEnd[m]   = i-1;
        m++;
        sBegin[m] = i+1;
        eEnd[m]   = high;
    }
    rValue = 0;
    for(int i=1; i<=rel[0][0]; i++)
    {
        int *temp;
        if(i == 1)
        {
            rValue++;
            temp = rel[i];
            continue;
        }

        if(compareSetsSa(rel[i],temp,2) == 0)
        {
            temp = rel[i];
            continue;
        }
        else
        {
            rValue++;
            temp = rel[i];
        }
    }
    free(sBegin);
    free(eEnd);
    return rValue;
}


void writeSmallToFile(int ** rel, int *relSmall)
/******************************************************************************/
/*****Function:write the small set to file,observe the order of small set *****/
/*****Input:   relSmall contains all small set we need to write           *****/
/*****Output:  Null                                                       *****/
{
    FILE *smallSet = fopen("small.txt","w");
    if(smallSet==NULL)
    {
        printf("Can't open the file!\n");
        return;
    }
    for(int i=1; i<=relSmall[0]; i++)
    {
        fprintf(smallSet,"%d:\t",relSmall[i]);
        for(int j=1; j<=rel[relSmall[i]][0]; j++)
        {
            fprintf(smallSet,"%d\t",rel[relSmall[i]][j]);
        }
        fprintf(smallSet,"\n");
    }
    return;
}

void  sortRelLex(int ** aRel, int pStart, int pEnd)
/*****************************************************************************/
/******aRel are sorted in lexicographic order of sets             ************/
/******i th set in the new order is the oMap[i] th set in original order******/
/*****************************************************************************/
{
    int * sBegin, *eEnd,*ptrTemp, low, high,pos, iTemp,m, i,j;

    m=0;

    sBegin  = (int*) malloc(pEnd * sizeof(int));
    eEnd    = (int*) malloc(pEnd * sizeof(int));

    sBegin[0] = pStart;
    eEnd[0]   = pEnd;
    srand(time(NULL));
    //quick sort
    while(m >= 0)
    {
        low = sBegin[m];   //pop an iteration
        high= eEnd[m];
        m--;

        if(low >= high)
        {
            continue;
        }
        pos = low + rand()%(high-low+1);


        ptrTemp=aRel[high];
        aRel[high]=aRel[pos];
        aRel[pos]=ptrTemp;

        i = low-1;
        for(j=low; j<high; j++)
        {
            if(compareSets(aRel[j], aRel[high]) <= 0 )
            {
                i++;

                ptrTemp=aRel[j];
                aRel[j]=aRel[i];
                aRel[i]=ptrTemp;
            }
        }
        i++;

        ptrTemp=aRel[high];
        aRel[high]=aRel[i];
        aRel[i]=ptrTemp;

        m++;   //push
        sBegin[m] = low;
        eEnd[m]   = i-1;
        m++;
        sBegin[m] = i+1;
        eEnd[m]   = high;
    }

    free(sBegin);
    free(eEnd);
}

int  compareSets(int * fSet, int * sSet)
/***************************************************************************/
/****to compare two ordered sets lexicographically                  ********/
/**** return 0, if the two sets are the same                        ********/
/**** return 1, if fSet> sSet                                       ********/
/**** return -1, otherwise                                          ********/
/***************************************************************************/
{
    int i;
    i=1;
    while((i<= fSet[0]) && (i<= sSet[0]))
    {
        if(fSet[i] < sSet[i])
        {
            return -1;
        }
        else if(fSet[i] > sSet[i])
        {
            return 1;
        }
        i++;
    }
    if(fSet[0]==sSet[0])
    {
        return 0;
    }
    else if(fSet[0] < sSet[0])
    {
        return -1;
    }
    return 1;
}



#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <math.h>
#include  <time.h>
#include  <pthread.h>
#include  "sizeAware.h"
#include  "dataload.h"
#include  "sizewareasfilter.h"
#include  "allPairs.h"

extern FILE * logFp;
extern int  * relLarge;
extern int  * relSmall;
extern int    binarySearchFlag;

void   radixSortCandidate(int * candidates, int maxValue);
void * verifyShortInsect_thread(void * para);
void * joinLong_Thread(void * para);


long long filterAndRefine(SSTATISTICS * stat, int** rel, int threshold)
{
    long long rValue=0;
    double    rmFactor;
    int **    saRel;
    int saThreshold, rmThreshold, i,j,k, avgEst;
    size_t tStart, tEnd;

    printf("\n\n##################    FILTERandREFINE   ##########################\n");
    printf("filterANDrefine is ready to run........\n");
    fprintf(logFp, "\n\n##################   FILTERandREFINE   ##########################\n");
    fprintf(logFp, "filterANDrefine is ready to run........\n");

    avgEst = 0;
    for(i=2; i<= stat->biasTurnFre; i++)
    {
        //avgEst += (stat->histgram[i])*i*(i-1);
        //avgEst += (i*stat->histgram[i]/((double)rel[0][0]))*log(i*(stat->histgram[i]))/((double)log(log(i*(stat->histgram[i]))));
        avgEst   += (stat->histgram[i])*i;
        printf("%d-th\t", i);
        printf("%d \n", (stat->histgram[i]));

    }

    avgEst = ceil(avgEst/((double)rel[0][0]));
    printf("Arbitrary two sets have %d common elements \n", avgEst);

    if( avgEst <=1 || threshold < 2*avgEst)
    {
        return AllPairsJoin(rel, threshold);
    }

    if(threshold > stat->avgLen)
    {
        return  SizeAwareJoin(rel, threshold);
    }

    rmThreshold= ceil(stat->avgLen - 2*avgEst);

    tStart = clock();
    //rmFactor = 0;//(log2(stat->minHighFre)-log2(stat->maxLowFre))/(double)(log2(stat->hFrePos)-log2(stat->lFrePos));

    //printf("rmFactor: %8f 1-4Frequency: %d  3-4Frequency: %d  \n ", rmFactor, stat->maxLowFre, stat->minHighFre);

    //rmThreshold = stat->avgBiasTurnLen;//(int)threshold*rmFactor;
    //saThreshold   = ceil(ALPHA*(log2(rel[0][0])/2)/fmax( ceil(log2(log2(3*0.8*threshold)/(double)2))-1, 1));
    saThreshold = threshold - rmThreshold;
    if(saThreshold <=0)
    {
       return  SizeAwareJoin(rel, threshold);
    }

    printf("filterANDrefine removes %d hfe in each set to generate candidates!\n", rmThreshold);
    fprintf(logFp, "filterANDrefine removes %d hfe in each set to generate candidates!\n", rmThreshold);

    /*****sort relation by theirs size to guarantee consistent order of all sets****/
    sortRelSetByNum(rel, 1, rel[0][0]);

    printf("The input sets are sorted!\n");
    fprintf(logFp, "The input sets are sorted!\n");

    /*****construct the relation sent to sizeAware algorithm************************/
    saRel   = (int **)malloc((rel[0][0]+1)*sizeof(int *));
    saRel[0]= (int *)malloc(1*sizeof(int));
    saRel[0][0]=rel[0][0];
    for(i=1; i<=rel[0][0]; i++)
    {
        if(rel[i][0] <= rmThreshold)
        {
            saRel[i]    = (int *)malloc(1*sizeof(int));
            saRel[i][0] = 0;
        }
        else
        {
            saRel[i]    = (int *)malloc((rel[i][0]-rmThreshold +1)*sizeof(int));
            memcpy(saRel[i], rel[i], (rel[i][0]-rmThreshold +1)*sizeof(int));
            saRel[i][0] -= rmThreshold;
        }
    }
    printf("filterANDrefine has pruned %d highest frequency elements in each set!\n", rmThreshold);
    fprintf(logFp, "filterANDrefine has pruned %d highest frequency elements in each set!\n", rmThreshold);


    rValue  += sizeawareFilter(rel, saThreshold, threshold);

    tEnd = clock();
    printf("RefineAndFilter generates %d results in %8f seconds!\n", rValue, (tEnd-tStart)/(double)CLOCKS_PER_SEC);
    fprintf(logFp, "RefineAndFilter generates %d results in %8f seconds!\n", rValue, (tEnd-tStart)/(double)CLOCKS_PER_SEC);
    freeRelation(saRel);
    return rValue;
}

long long sizeawareFilter(int** rel, int threshold, int fThreshold)
{
    int       * candidates;
    long long   rValue;
    int       **inverted;
    int       **invertedSmall;
    HEAP       *hbt =  ( HEAP  *)malloc(sizeof(HEAP));
    int x,tThread;
    LONGPARA   lPara;
    SHORTPARA  sPara;
    pthread_t  tidp1,tidp2;
    void **    retVal;                                                     //size boundary

    int        *TimeCostForLarge;     //对所有的集合R,计算其所包含的元素的倒排索引长度和
    int         minsize = rel[1][0];  //the smallest set size in R
    int         maxsize = rel[1][0];



    printf("sizeAwareFilter is called to generate candidate pairs........\n");
    fprintf(logFp, "sizeAwareFilter is called to generate candidate pairs........\n");

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
    /******sets have been sorted in their sizes********/
    //sortRelSetByNum(rel,1, rel[0][0]);
    inverted = GetInvertedIndexinAllR(rel);
    TimeCostForLarge = GetTimeCostForLarge(rel,inverted);
    x = GetSizeBoundary(rel,threshold,minsize,maxsize,TimeCostForLarge);
    relLarge = NULL;
    relSmall = NULL;

    GetLargeAndSmallsets(rel,x);

    /****we plan to create a new thread to execute this task***/
    lPara.relation = rel;
    lPara.thresh   = threshold;
    lPara.fThresh  = fThreshold;
    lPara.relLarge = relLarge;
    lPara.rValueLong = 0;
    lPara.invertedIndex = inverted;

    pthread_create(&tidp1, NULL, &joinLong_Thread, &lPara);
    /*
    candidates = ScanCountforLongSets(rel,threshold,relLarge,inverted);
    SizeAwareFreeInverted(inverted);


    if(candidates[0] > 0)
    {
        for(int i=1; i<=candidates[0]; i++)
        {
            if(verifyIntersection( rel[ candidates[2*i] ], rel[ candidates[2*i+1] ], fThreshold) == 1)
            {
                rValue++;
            }
        }
    }
    printf("filterANDrefine have generate %lld results when join long sets!\n",rValue);
    fprintf(logFp, "filterANDrefine have generate %lld results when join long sets!\n",rValue);
    free(candidates);
    */


    invertedSmall = GetInvertedIndexinRs(rel,relSmall);
    candidates = filterBlockDedup(rel,invertedSmall,threshold,hbt);

    int k=candidates[0];
    if(k>0)
    {
        printf("filterANDrefine have generated %d the candidate pairs from short sets!\n", candidates[0]);
        fprintf(logFp, "filterANDrefine have generated %d the candidate pairs from short sets!\n", candidates[0]);
        radixSortCandidate(candidates, rel[0][0]);
        printf("filterANDrefine have sorted the candidate pairs!\n");
        fprintf(logFp, "filterANDrefine have sorted the candidate pairs!\n");

        candidates[0] = 1;
        for(int i=2; i<=k; i++)
        {
            if(candidates[2*i] != candidates[ 2*candidates[0]]  || candidates[2*i+1] != candidates[ 2*candidates[0]+1])
            {
                candidates[0]++;
                candidates[ 2*candidates[0]]   = candidates[2*i];
                candidates[ 2*candidates[0]+1] = candidates[2*i+1];
            }
        }

        printf("filterANDrefine have removed the repeated candidate pairs!\n");
        fprintf(logFp, "filterANDrefine have removed the repeated candidate pairs!\n");
        /******call verification to check it whether a real results ***/
        candidates[1] = candidates[0]/2;
        sPara.cand = candidates;
        sPara.relation = rel;
        sPara.thresh   = fThreshold;
        sPara.rValueShort = 0;
        pthread_create(&tidp2, NULL, &verifyShortInsect_thread, &sPara);

        for(int i=1; i<= candidates[1]; i++)
        {
            if(verifyIntersection(rel[ candidates[2*i]], rel[ candidates[2*i+1]], fThreshold)==1)
            {
                rValue++;
            }
        }
        tThread = pthread_join(tidp2, retVal);
        if(tThread != 0)
        {
            printf("Thread tor verify short join can not be created!\n");
        }
        rValue += sPara.rValueShort;
    }
    printf("main short: %lld ", rValue);

    tThread = pthread_join(tidp1, retVal);
    if(tThread != 0)
    {
        printf("Thread tor long join can not be created!\n");
    }
    rValue += lPara.rValueLong;
    free(hbt);                                        //因为在HeapDedup结束时已经释放了堆中的内容
    SizeAwareFreeInverted(invertedSmall);
    free(relLarge);
    free(relSmall);
    free(candidates);
    printf("\n", rValue);
    return rValue;
}

void * verifyShortInsect_thread(void * para)
{
    int ** rel = ((SHORTPARA*)para)->relation;
    int *  candidates = ((SHORTPARA*)para)->cand;
    int    fThreshold = ((SHORTPARA*)para)->thresh;
    long long rValue  = 0;

    for(int i=candidates[1]+1; i<= candidates[0]; i++)
    {
        if(verifyIntersection(rel[ candidates[2*i]], rel[ candidates[2*i+1]], fThreshold)==1)
        {
            rValue++;
        }
    }
    ((SHORTPARA*)para)->rValueShort = rValue;
    printf("short result: %lld ", rValue);
    return ((void *)NULL);
}

void * joinLong_Thread(void * para)
{
    int threshold = ((LONGPARA *) para)->thresh;
    int fThreshold = ((LONGPARA *) para)->fThresh;
    int ** rel = ((LONGPARA *) para)->relation;
    int ** inverted = ((LONGPARA *) para)->invertedIndex;
    int *  candidates;
    long long rValue=0;


    candidates = ScanCountforLongSets(rel,threshold,relLarge,inverted);
    SizeAwareFreeInverted(inverted);

    /*********************************/
    if(candidates[0] > 0)
    {
        for(int i=1; i<=candidates[0]; i++)
        {
            if(verifyIntersection( rel[ candidates[2*i] ], rel[ candidates[2*i+1] ], fThreshold) == 1)
            {
                rValue++;
            }
        }
    }

    ((LONGPARA *) para)->rValueLong = rValue;
    printf("long result: %lld ", rValue);
    free(candidates);
    return ((void *)NULL);
}




void radixSortCandidate(int * candidates, int maxValue)
{
    int * tCand = (int *)malloc(2*(candidates[0]+1)*sizeof(int));
    int * tCount= (int *)malloc((maxValue+1)*sizeof(int));
    for(int pass=1; pass>=0; pass--)
    {
        memset(tCount, 0,(maxValue+1)*sizeof(int));
        memcpy(tCand, candidates,2*(candidates[0]+1)*sizeof(int));
        for(int i=1; i<= candidates[0]; i++)
        {
            tCount[ tCand[2*i+pass] ] ++;
        }
        for(int i=1; i<= maxValue; i++)
        {
            tCount[i] += tCount[i-1];
        }
        for(int i= candidates[0]; i>=1; i--)
        {
            candidates[ 2*tCount[  tCand[2*i+pass]  ]    ]   = tCand[2*i];
            candidates[ 2*tCount[  tCand[2*i+pass]  ]+1  ]   = tCand[2*i+1];
            tCount[tCand[2*i+pass]]--;
        }
    }
    free(tCand);
    free(tCount);
}

int * ScanCountforLongSets(int **rel, int threshold, int *relLarge, int **inverted)
{
    int * cand = (int *)malloc(2*sizeof(int));
    cand[0] = 0;
    cand[1] = 0;
    for(int i=1; i<=relLarge[0]; i++)
    {
        int *countR;
        //each time a same malloc, move from here outside the loop, just keep memset is ok
        countR = (int *)malloc(sizeof(int)*(rel[0][0])+1);
        memset(countR, 0, sizeof(int)*(rel[0][0])+1);
        countR[0] = rel[0][0];
        for(int j=1; j<=rel[relLarge[i]][0]; j++)
        {
            for(int k=1; k<=inverted[rel[relLarge[i]][j]][0]; k++)
            {
                countR[inverted[rel[relLarge[i]][j]][k]]++;
                if(relLarge[i]>inverted[rel[relLarge[i]][j]][k] && countR[inverted[rel[relLarge[i]][j]][k]] == threshold)
                {
                    cand[0]++;
                    cand    = (int*)realloc(cand, 2*(cand[0]+1)*sizeof(int));
                    cand[2*cand[0]  ] = (relLarge[i]<inverted[rel[relLarge[i]][j]][k])? (relLarge[i]):(inverted[rel[relLarge[i]][j]][k]);
                    cand[2*cand[0]+1] = (relLarge[i]<inverted[rel[relLarge[i]][j]][k])? (inverted[rel[relLarge[i]][j]][k]):(relLarge[i]);
                }
            }
        }
        free(countR);
    }
    return cand;
}

int *   filterBlockDedup(int **rel, int **inverted, int threshold, HEAP *hbt)
{
    int **Rtmp;
    int ** slim;
    int * cand = (int*)malloc(2*sizeof(int));
    cand[0]=0;
    cand[1]=0;

    //对每一个在inverted中的I(e),得到Rtmp，然后得到c-subset的倒排索引表
    for(int i=1; i<=inverted[0][0]; i++)
    {
        Rtmp      = (int **)malloc(sizeof(int *));
        Rtmp[0]   = (int *)malloc(sizeof(int));
        Rtmp[0][0]= 0;
        for(int j=2; j<=inverted[i][0]; j++)
        {
            //对每一个inverted[i][j],计算rel[inverted[i][j]]中大于inverted[i][1]的项并付给Rtmp
            int index;   //记录rel[inverted[i][j]]中第一个大于e的那个元素的下标（rel[inverted[i][j]]中元素是递增的）
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
        if(Rtmp[0][0]>1)
        {
            //c为HeapDedup中的c
            int c = threshold-1;
            slim = HeapDedup(Rtmp,c,hbt);

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
                            cand[0]++;
                            cand    = (int*)realloc(cand, 2*(cand[0]+1)*sizeof(int));
                            cand[2*cand[0]  ] = (Rtmp[slim[k][j]][1] < Rtmp[slim[k][m]][1])? Rtmp[slim[k][j]][1]: Rtmp[slim[k][m]][1];
                            cand[2*cand[0]+1] = (Rtmp[slim[k][j]][1] < Rtmp[slim[k][m]][1])? Rtmp[slim[k][m]][1]: Rtmp[slim[k][j]][1];
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
    return cand;
}



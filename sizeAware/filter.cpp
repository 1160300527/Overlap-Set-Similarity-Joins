#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <math.h>
#include  <time.h>
#include  "filter.h"
#include  "dataload.h"
#include  "allPairs.h"

extern FILE * logFp;

/*******************************************************************/
/**global array, to map bucket labels to the range of frequencies***/
int  lowerBound[16]= {0,1,3,5, 9,13,17,21,29,37,45,53,61,69,77,85};
int  upperBound[16]= {0,2,4,8,12,16,20,28,36,44,52,60,68,76,84,100};
char mask[8]       = {(char)0x01, (char)0x02, (char)0x04,(char)0x08,(char)0x10,(char)0x20,(char)0x40,(char)0x80};

#ifndef PRIME_NUM
#define PRIME_NUM  24
#endif // PRIME_NUM
int  prime[PRIME_NUM]= { 103,    83,     73,    43,    53,   67,   79,   89,
                         109,   127,    139,   149,   157,  173,  199,  223,
                         241,   257,    271,   283,   293,  307,  317,  337
                       };
/*******************************************************************/
/*******************************************************************/
/****declaration of internal functions *****************************/
int  fre2Bucket(int frequency);
int  fre2BucketNew(int frequency);
int  compareVectors(int dim, int * vector1, int * vector2);
void sortVector(int **vector, int* map, int start, int end);
void radixSortVector(int ** vector, int * map);
void radixSortVectorAndRelation(int ** vector, int ** rel, int threshold);

long long  newFilterAllPairsJoin(int ** rel, int threshold)
{
    long long rValue, mFiltered, oFiltered, vFiltered, pFiltered;
    int ** vector;
    int ** inverted;
    bool * Processed;
    char * filter;
    int    hashParaA, hashParaB, aPrime,i,j,k,join,current,lastClass,compare, rThreshold,fThreshold;
    int  * set2Class;
    size_t     tStart, tEnd;
    printf("\n\nfilterAllpairs is ready to run........\n");
    fprintf(logFp, "\n\n filterAllpairs is ready to run........\n");
    tStart = clock();
    srand(time(NULL));
    aPrime    = prime[rand()%PRIME_NUM];
    hashParaA = rand()%aPrime;
    hashParaB = rand()%aPrime;

    //to map each set to a vector
    vector    = (int **)malloc(sizeof(int*)*(rel[0][0]+1));
    vector[0] = (int *)malloc(sizeof(int)*2);
    vector[0][0]=rel[0][0];
    /*************************rThreshold is used to build filter**************/
    /*************************fThreshold is used in filter      **************/
    fThreshold= ceil(ALPHA*(log2(rel[0][0])/2)/fmax( ceil(log2(log2(3*0.8*threshold)/(double)2))-1, 1));
    rThreshold= threshold - fThreshold;
    /**********can be enlarged further but vector[0][1]>= threshold+1 must hold*******/
    //vector[0][1]=threshold+1;
    vector[0][1]= (int) 2*fThreshold;
    for(i=1; i<= rel[0][0]; i++)
    {
        vector[i] = (int *)malloc(sizeof(int)*(vector[0][1]));
        memset(vector[i], 0, sizeof(int)*(vector[0][1]));
        /**map rel[i] to a vector of frequencies, rThreshold high frequency elements are removed**/
        for(j=1; j<= rel[i][0]-rThreshold; j++)
        {
            vector[i][ ((hashParaA*rel[i][j]+hashParaB)%aPrime)%vector[0][1]]++;
        }
        for(j=0; j<vector[0][1]; j++)
        {
            /****vector[i][j] = 0,1,..., or fThreshold must hold           ***/
            //vector[i][j] = (vector[i][j]> threshold)?threshold: vector[i][j];
            vector[i][j] = freToBucket(vector[i][j], fThreshold);
        }
    }
    printf("All sets have been mapped to %d-dim vectors!\n",vector[0][1]);
    radixSortVectorAndRelation(vector, rel, fThreshold);
    printf("vectors have been sorted!\n");
    set2Class = (int *)malloc(sizeof(int)*(rel[0][0]+1));
    current   =  1;
    set2Class[1]=current;
    for(i=2; i<=vector[0][0]; i++)
    {
        compare = compareVectors(vector[0][1], vector[i-1], vector[i]);
        if(compare == 0)
        {
            set2Class[i] = current;
        }
        else if (compare <0)      //it must be compare = -1
        {
            current++;   // a new class begins
            set2Class[i] = current;
        }
        else
        {
            printf("Error occurs during sorting the vectors!\n");
        }
    }
    set2Class[0] = current;     //the number of classes are recorded in set2Class[0];
    printf(" %d classes are obtained among %d sets!\n", set2Class[0], rel[0][0] );
    tEnd = clock();
    printf("filterAllPairJoin prepares info for a powerful filter in %5f seconds!\n",(tEnd-tStart)/(double)CLOCKS_PER_SEC);
    fprintf(logFp, "filterAllPairJoin prepares information for a powerful filter in %8f seconds!\n", (tEnd-tStart)/(double)CLOCKS_PER_SEC);

    /*******begin to perform join with online construct filter****************************************/
    rValue    =  0;
    mFiltered =  0;
    oFiltered =  0;
    vFiltered =  0;
    pFiltered =  0;
    lastClass = -1;

    Processed = (bool*)malloc(sizeof(bool)*(rel[0][0]+1));
    filter    = (char*)malloc(sizeof(char)*(set2Class[0]+1));
    inverted      = (int**)malloc(sizeof(int*));//inverted index
    inverted[0]   = (int*) malloc(sizeof(int));
    inverted[0][0]=0;

    for(i=1; i<= rel[0][0]; i++)
    {
        memset(Processed, 0, sizeof(bool)*(rel[0][0])+1);
        /****if we meet a new class, the filter must be rebuild****/
        if(lastClass != set2Class[i] )
        {
            memset(filter, 0, sizeof(char)*(set2Class[0]+1));
            lastClass = set2Class[i];
        }
        for(j=1; j<= rel[i][0]-threshold+1; j++)
        {
            if(rel[i][j] > inverted[0][0])
            {
                inverted    = (int **)realloc(inverted, (rel[i][j]+1)*sizeof(int*));
                for(k= inverted[0][0]+1; k<= rel[i][j]; k++)
                {
                    inverted[k]    = (int*)malloc(sizeof(int));
                    inverted[k][0] = 0;
                }
                inverted[0][0] = rel[i][j];
            }
            inverted[ rel[i][j] ][0]++;
            inverted[ rel[i][j] ]  = (int *)realloc(inverted[ rel[i][j] ], (inverted[ rel[i][j] ][0]+1)*sizeof(int));
            inverted[ rel[i][j] ][ inverted[ rel[i][j] ][0] ] =  i;

            /******the new indexed set is ignored****************/
            for(join =1; join< inverted[ rel[i][j] ][0]; join++)
            {
                int anotherSet = inverted[ rel[i][j] ][join];
                if(Processed[ anotherSet])
                {
                    /*******this tuple is pruned for repeatition ****/
                    pFiltered++;
                    continue;
                }
                else if(filter[ set2Class[anotherSet] ] == ((char)0x02))
                {
                    /*******this tuple is pruned by master filter ****/
                    mFiltered++;
                    continue;
                }

                /***********to set master filter if possible   *******************/
                if(filter[ set2Class[anotherSet] ] == ((char)0x00))
                {
                    int innerProduct=0;
                    for(int f=0; f<vector[0][1];  f++)
                    {
                        if(vector[i][f]<= vector[anotherSet][f])
                        {
                            innerProduct += upperBound[vector[i][f]];
                        }
                        else
                        {
                            innerProduct += upperBound[vector[anotherSet][f]];
                        }
                        //innerProduct += ((vector[i][f]<vector[anotherSet][f])? vector[i][f]:vector[anotherSet][f]);
                    }
                    if( innerProduct < fThreshold)
                    {
                        mFiltered++;
                        filter[ set2Class[anotherSet] ] = (char)0x02;
                        Processed[anotherSet] = true;
                        continue;
                    }
                    else
                    {
                        filter[ set2Class[anotherSet] ] = (char)0x01;
                    }
                }
                /***********to perform verification****************************/
                if( verifyIntersection(rel[i], rel[anotherSet], threshold)==1)
                {
                    /*********obtain a tuple in the join results***************/
                    rValue++;
                }
                else
                {
                    /*************filtered by verification*********************/
                    vFiltered++;
                }
                Processed[ inverted[ rel[i][j] ][join] ]= true;
            }
        }
    }

    /*****************to record or show the final results  *********************/
    tEnd = clock();
    printf("filterAllPairJoin generates %lld results in %8f seconds!\n",rValue, (tEnd-tStart)/(double)CLOCKS_PER_SEC);
    printf("                 Precossed       avoid %lld repeated candidate pairs!\n",pFiltered);
    printf("                 Master filter removes %lld candidate pairs among %lld!\n",mFiltered, rValue+vFiltered+mFiltered);
    printf("                 Other  filters remove %lld candidate pairs!\n",oFiltered);
    printf("                 Verification  removes %lld candidate pairs!\n",vFiltered);
    fprintf(logFp, "filterAllPairJoin filters %d and generates %lld results in %8f seconds!\n",rValue, (tEnd-tStart)/(double)CLOCKS_PER_SEC);
    fprintf(logFp, "                 Precossed       avoid %lld repeated candidate pairs!\n",pFiltered);
    fprintf(logFp, "                 Master filter removes %lld candidate pairs among %lld !\n",mFiltered, rValue+vFiltered+mFiltered);
    fprintf(logFp, "                 Other  filters remove %lld candidate pairs!\n",oFiltered);
    fprintf(logFp, "                 Verification  removes %lld candidate pairs!\n",vFiltered);
    fflush(logFp);
    freeRelation(vector);
    free(filter);
    free(Processed);
    AllPairsFreeInverted(inverted);
    free(set2Class);
    return rValue;
}


void radixSortVectorAndRelation(int ** vector, int ** rel, int threshold)
{
    int dim = vector[0][1];
    int pass, i;
    int  * count   = (int * ) malloc((threshold+1)*sizeof(int));
    int ** tVector = (int **) malloc((vector[0][0]+1)*sizeof(int*));
    int ** tRel    = (int **) malloc((rel[0][0]+1)*sizeof(int*));

    for(pass=dim -1; pass>=0; pass--)
    {
        memset(count, 0, sizeof(int)*(threshold+1));
        memcpy(tVector, vector, (vector[0][0]+1)*sizeof(int*));
        memcpy(tRel, rel, (rel[0][0]+1)*sizeof(int*));

        for(i=1; i<= vector[0][0]; i++)
        {
            count[ vector[i][pass] ] ++;
        }
        for(i=1; i<= threshold; i++)
        {
            count[i] += count[i-1];
        }
        for(i=tVector[0][0]; i>=1 ; i--)
        {
            vector[ count[ tVector[i][pass]] ] = tVector[i];
            rel   [ count[ tVector[i][pass]] ] = tRel[i];
            count[ tVector[i][pass]]--;
        }
    }
    free(tVector);
    free(tRel);
    free(count);
    return;
}


long long  filterAllPairsJoin(int ** rel, int threshold)
{
    long long  rValue;
    int       **inverted;
    bool      *Processed;
    size_t     start, end, fStart, fEnd;
    FILTER    *filter;
    int        iClass, anotherSet, jClass, nFilter, byte, bit,temp, filtered;
    char       fResult;
    double     rate;

#define   getFilter(aFilter, row, col, r)                         \
    {                                                             \
        temp = (row-1)*aFilter.set2Class[0]+(col-1);              \
        byte = temp>>3;                                           \
        bit  = temp%8;                                            \
        r    = aFilter.interClass[byte] & mask[bit];              \
    }
    /*****cost of the filter is takeen as 15  *********************/
    /************************************ 4 to compute temp********/
    /************************************ 3 to compute byte *******/
    /************************************ 3 to compute bit  *******/
    /************************************ 5 to compute r    *******/
    /****cost of verification is taken as 5*threshold       *******/
    /************************************ 4 to fetch elements  ****/
    /***********************************  1 to compare         ****/

    printf("\n\n filterAllpairs is ready to run........\n");
    fprintf(logFp, "\n\n filterAllpairs is ready to run........\n");
    start = clock();
    printf("filterAllPairJoin algorithm begins with the inverted index built incrementally....\n");
    fprintf(logFp, "filterAllPairJoin algorithm begins with the inverted index built incrementally....\n");
    rValue   = 0;       //the number of join results
    filtered = 0;
    Processed = (bool*)malloc(sizeof(bool)*(rel[0][0]+1)); //space used to avoid repeated results

    if(threshold<=15)
    {
        nFilter = 0;
        fResult= (char)0x01;
        printf("filterAllPairJoin  won't use any filter !\n");
        fprintf(logFp, "filterAllPairJoin  won't use any filter!\n");
    }
    else
    {
        fStart = clock();
        printf("filterAllPairJoin tries to create 1-th filters!\n");
        fprintf(logFp, "filterAllPairJoin tries to create 1-th filters!\n");
        filter = (FILTER *)malloc(threshold*sizeof(FILTER));
        filterCreate(rel, threshold, ((FILTER*)filter)+0, 0,  &rate);
        /***Assume 1/20 candidate can be filtered by our filter****/
        /***if actually larger, our algorithm can beat allpairs****/
        nFilter = ceil(rate*0.1*2*5*threshold/15);
        fResult= (char)0x00;
        if (nFilter<=0)
        {
            nFilter =1;
        }
        for(int i=1; i<nFilter; i++)
        {
            printf("filterAllPairJoin tries to create %d-th filters!\n",i+1);
            fprintf(logFp, "filterAllPairJoin tries to create %d-th filters!\n",i+1);
            filterCreate(rel, threshold, ((FILTER*)filter)+i, i,  &rate);
        }
        fEnd = clock();
        printf("filterAllPairJoin has created %d filters in %6f seconds\n",nFilter,(fEnd-fStart)/((double)CLOCKS_PER_SEC));
        fprintf(logFp, "filterAllPairJoin has created %d filters in %6f seconds\n",nFilter, (fEnd-fStart)/((double)CLOCKS_PER_SEC));
    }

    inverted      = (int**)malloc(sizeof(int*));//inverted index
    inverted[0]   = (int*) malloc(sizeof(int));
    inverted[0][0]=0;
    //memorySize   += sizeof(int*)+sizeof(int);
    for(int i=1; i<= rel[0][0]; i++)
    {
        memset(Processed, 0, sizeof(bool)*(rel[0][0])+1);
        for(int j=1; j<= rel[i][0]-threshold+1; j++)
        {
            for(i=2; i<=rel[0][0]; i++)
            {
                if(rel[i][j] > inverted[0][0])
                {
                    //memorySize += (rel[i][j]-inverted[0][0])*sizeof(int*);
                    inverted    = (int **)realloc(inverted, (rel[i][j]+1)*sizeof(int*));
                    for(int k= inverted[0][0]+1; k<= rel[i][j]; k++)
                    {
                        inverted[k]    = (int*)malloc(sizeof(int));
                        inverted[k][0] = 0;
                        // memorySize    +=sizeof(int);
                    }
                    inverted[0][0] = rel[i][j];
                }

                //memorySize += sizeof(int);
                inverted[ rel[i][j] ][0]++;
                inverted[ rel[i][j] ]  = (int *)realloc(inverted[ rel[i][j] ], (inverted[ rel[i][j] ][0]+1)*sizeof(int));
                inverted[ rel[i][j] ][ inverted[ rel[i][j] ][0] ] =  i;

                //to perform join with this inverted list
                for(int join =1; join< inverted[ rel[i][j] ][0]; join++)
                {
                    anotherSet = inverted[ rel[i][j] ][join];
                    if(Processed[ anotherSet ])
                    {
                        continue;
                    }
                    for(int f=0; f<nFilter;  f++)
                    {
                        iClass = filter[f].set2Class[i];
                        jClass = filter[f].set2Class[anotherSet];
                        if(iClass == jClass)
                        {
                            continue;
                        }
                        getFilter(filter[f], jClass, iClass, fResult);
                        if( fResult == ((char)0x00))
                        {
                            Processed[ inverted[ rel[i][j] ][join] ]= true;
                            break;
                        }
                    }
                    if( fResult ==((char)0x00))
                    {
                        filtered++;
                        continue;
                    }
                    if( verifyIntersection(rel[i], rel[anotherSet], threshold)==1)
                    {
                        rValue++;
                        Processed[ inverted[ rel[i][j] ][join] ]= true;
                    }
                }
            }
        }
    }
    for(int i=0; i<nFilter; i++)
    {
        free(filter[i].interClass);
        free(filter[i].set2Class);
    }

    free(Processed);
    AllPairsFreeInverted(inverted);
    end = clock();
    printf("filterAllPairJoin filters %d and generates %lld results in %3f seconds!\n",filtered, rValue, (end-start)/(double)CLOCKS_PER_SEC);
    fprintf(logFp, "filterAllPairJoin filters %d and generates %lld results in %3f seconds!\n", filtered, rValue, (end-start)/(double)CLOCKS_PER_SEC);
    return  rValue;
}


void filterCreate(int ** rel, int threshold, FILTER * filter, int which, double *rate)
/******************************************************************************************/
/**to create a filter for given relation rel, threshold with paraA and paraB    ***********/
/**Input: rel  a collection of sets                                             ***********/
/**       threshold, the parameter for the similarity join                      ***********/
/**       filter,    the pointer to the target filter                           ***********/
/**       hashParaA and hashParaB is used for hash function: ax+b mod k         ***********/
/**Output: target filter to be built                                            ***********/
/**Note:   to guarantee pairwise independent filters, hashParaAs and Bs must be careful ***/
/******************************************************************************************/
{
    int k,i,j,t,compare, current;
    int ** vector;
    int *  orderMap;
    int *  set2Class;
    char*  isOrNot;
    size_t start,end;
    int hashParaA, hashParaB, aPrime, byte, bit, temp;

#define setFilter(row, col)                       \
    {                                             \
        temp =(row-1)*set2Class[0] + (col-1);     \
        byte = temp>>3;                           \
        bit  = temp%8;                            \
        isOrNot[byte] |= mask[bit];               \
    }

    printf("FILTER_CREATE begin to run...\n");
    fprintf(logFp, "FILTER_CREATE begin to run...\n");
    start = clock();
    k  =  ceil((2*log2(rel[0][0])/3)/fmax( ceil(log2(log2(3*threshold+1)/2)), 1));
    filter->nSubset = k;

    srand(time(NULL));
    aPrime    = prime[(rand()+which)%PRIME_NUM];
    hashParaA = (rand()+which)%aPrime;
    hashParaB = (rand()+which)%aPrime;

    orderMap = (int *)malloc(sizeof(int)*(rel[0][0]+1));
    orderMap[0]=0;

    //to map each set to a vector
    vector    = (int **)malloc(sizeof(int*)*(rel[0][0]+1));
    vector[0] = (int *)malloc(sizeof(int)*2);
    vector[0][0]=rel[0][0];
    vector[0][1]=k;
    for(i=1; i<= rel[0][0]; i++)
    {
        orderMap[i]=i;
        vector[i] = (int *)malloc(sizeof(int)*k);
        memset(vector[i], 0, sizeof(int)*k);

        //map rel[i] to a vector of frequencies
        for(j=1; j<= rel[i][0]; j++)
        {
            vector[i][ ((hashParaA*rel[i][j]+hashParaB)%aPrime)%k]++;
        }

        //map each frequency to a bucket label
        for(j=0; j<k; j++)
        {
            vector[i][j] = freToBucket(vector[i][j], threshold);
        }
    }
    printf("all sets have been mapped to %d-dim vectors!\n",k);
    //to sort the vectors and record the change of the order of vectors, current it cost too much time
    //sortVector(vector, orderMap, 1, rel[0][0]);
    radixSortVector(vector, orderMap);
    printf("vectors have been sorted!\n");
    set2Class = (int *)malloc(sizeof(int)*(rel[0][0]+1));
    current   =  1;
    set2Class[orderMap[1]]=1;
    for(i=2; i<=rel[0][0]; i++)
    {
        compare = compareVectors(k, vector[current], vector[i]);
        if(compare == 0)
        {
            set2Class[ orderMap[i] ] = current;
            free(vector[i]);                    // a redundant vector be deleted
        }
        else       //it must be compare = -1
        {
            current++;   // a new class begins
            vector[current] = vector[i];
            set2Class[ orderMap[i] ] = current;
        }
    }
    printf("The number of classes is %d versus %d!\n", current, rel[0][0] );
    set2Class[0] = current;     //the number of classes are recorded in set2Class[0];
    vector[0][0] = current;
    filter->set2Class = set2Class;
    free(orderMap);             // ordermap is no longer needed

#ifdef  DEBUG
    printf("n is %d, # of classes is %d\n", rel[0][0], current*current);
    fprintf(logFp, "n is %d, # of classes is %d\n", rel[0][0], current*current);
#endif
    isOrNot = (char*)malloc(sizeof(char)* (((current+1)*(current+1) /8+1)));   //the design garantees that n> nClass^2
    if(isOrNot == NULL)
    {
        exit(1);
    }
    memset(isOrNot, 0, sizeof(char)* (((current+1)*(current+1) /8+1)));
    compare = 0;
    for(i=1; i<=set2Class[0]; i++)
    {
        for(j=i; j<=set2Class[0]; j++)
        {
            //if vector[i] vector[j] >= c, then isOrNot[i-1][j-1] = true;
            current = 0;
            for(t=0; t<k; t++)
            {
                if(vector[i][t]<= vector[j][t])
                {
                    current += upperBound[vector[i][t]];
                }
                else
                {
                    current += upperBound[vector[j][t]];
                }
            }
            if(current>= threshold)
            {
                compare++;
                setFilter(i,j);
                setFilter(j,i);
                //isOrNot[ (i-1)*set2Class[0] +(j-1)] = true;
                //isOrNot[ (j-1)*set2Class[0] +(i-1)] = true;
            }
        }
    }
    (*rate) = 1- compare/((double)(set2Class[0]+1)*set2Class[0]/2);
    printf("The filtering ability of the filter is %3f!\n", (*rate));
    filter->interClass = isOrNot;
    freeRelation(vector);
    end = clock();
    printf("FilterCreate finished in %3f seconds!\n",  (end-start)/(double)CLOCKS_PER_SEC);
    fprintf(logFp, "FilterCreate finished in %3f seconds!\n", (end-start)/(double)CLOCKS_PER_SEC);
}


void radixSortVector(int ** vector, int* map)
{
    int dim = vector[0][1];
    int pass, i,j;
    int count[100];
    int ** tVector = (int **)malloc((vector[0][0]+1)*sizeof(int*));
    int *  tMap    = (int *) malloc((vector[0][0]+1)*sizeof(int));

    for(pass=dim -1; pass>=0; pass--)
    {
        memset(count, 0, sizeof(int)*100);
        memcpy(tVector, vector, (vector[0][0]+1)*sizeof(int*));
        memcpy(tMap, map, (vector[0][0]+1)*sizeof(int));

        for(i=1; i<= vector[0][0]; i++)
        {
            count[ vector[i][pass] ] ++;
        }
        for(i=1; i<= 99; i++)
        {
            count[i] += count[i-1];
        }
        for(i=tVector[0][0]; i>=1; i--)
        {
            vector[ count[ tVector[i][pass]] ] = tVector[i];
            map[    count[ tVector[i][pass]] ] = tMap[i];
            count[ tVector[i][pass]]--;
        }
    }
    free(tVector);
    free(tMap);
}


void sortVector(int **vector, int* map, int start, int end)
{

    int i,j,m,pos,low,high,flag,iTemp, compare, * vTemp, *sBegin, *eEnd;



    m=0;

    sBegin  = (int*) malloc(end * sizeof(int));
    eEnd    = (int*) malloc(end * sizeof(int));
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
        pos         = low + rand()%(high - low+1);            //randomized position between start and end

        iTemp       = map[pos];                                //swap element
        map[pos]    = map[high];
        map[high]    = iTemp;

        vTemp       = vector[pos];
        vector[pos] = vector[high];
        vector[high] = vTemp;

        flag = 0;
        i = low-1;                                            //to partion
        for(j=low; j<high; j++)
        {
            compare = compareVectors(vector[0][1],vector[j], vector[high]);
            if( compare <= 0)    //vector[j]<= vector[high]
            {
                i++;
                if(compare == 0)
                {
                    flag++;
                }
                if(i==j)
                {
                    continue;
                }
                iTemp     = map[j];
                map[j]    = map[i];
                map[i]    = iTemp;

                vTemp     = vector[j];
                vector[j] = vector[i];
                vector[i] = vTemp;
            }
        }
        if( flag == high-low)
        {
            continue;   //all between low and high equals to the partition element
        }
        i++;
        iTemp     = map[high];
        map[high]  = map[i];
        map[i]    = iTemp;

        vTemp     = vector[high];
        vector[high]=vector[i];
        vector[i] = vTemp;

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



int  compareVectors(int dim, int * vector1, int * vector2)
/**********************************************************/
/** return 0 if vector1=vector2                  **********/
/** return -1if vector1<vector2                  **********/
/** return 1 if vector1>vector2                  **********/
/**********************************************************/
{
    for(int i=0; i<dim; i++)
    {
        if(vector1[i]>vector2[i])
        {
            return 1;
        }
        else if (vector1[i]<vector2[i])
        {
            return -1;
        }
    }
    return 0;
}


void freeFilter(FILTER * fPtr)
{
    if(fPtr->set2Class != NULL)
    {
        free(fPtr->set2Class);
    }
    if(fPtr->interClass !=NULL)
    {
        free(fPtr->interClass);
    }
}


int  freToBucket(int frequency, int threshold)
{
    return (frequency>threshold)?fre2Bucket(threshold): fre2Bucket(frequency);
}

int  freToBucketNew(int frequency, int threshold)
{
    return (frequency>threshold)?fre2BucketNew(threshold): fre2BucketNew(frequency);
}

int  fre2BucketNew(int frequency)
{
    if(frequency<=1)
    {
        return frequency;
    }
    else
    {
        return 2+ fre2Bucket(frequency -2);
    }
}


int  fre2Bucket(int frequency)
/*********************************************/
/**to map frequency to the label of a bucket**/
/*********************************************/
{
    int bucket,k;
    k=floor(log2(3*frequency+1)/2);
    bucket = (1<<k) -1 + (3*frequency- (1<<(2*k)) +1)/(3*(1<<k));
    return bucket;
}

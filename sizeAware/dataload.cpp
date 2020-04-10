#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <math.h>
#include  <time.h>
#include  "dataload.h"

using namespace std;
//#ifndef stuDEBUG
//#define stuDEBUG
//#endif // stuDEBUG
extern FILE *  logFp;
extern int  ** relS;
extern int  *  sortedSet;
extern int  *  setSize;

#ifdef  DEBUG
extern int  ** result1;
extern int  ** result2;
#endif // DEBUG

void   mapUniverseToNewOreder(ELEMENTMAP * eleMap);
//void   sortMapByCount(ELEMENTMAP * eleMap, int start, int end);
void   sortMapByCount(ELEMENTMAP * eleMap, int Max);
void   sortMapByElement(ELEMENTMAP * eleMap, int start, int end);
void   sortSet(int * set, int start, int end);
void   statOfUniverse(ELEMENTMAP * eleMap, SSTATISTICS * stat);
void   statOfRelationS( int ** relS, SSTATISTICS * stat);
int    validAset(int * aSet,ELEMENTMAP *eleMap, int flag);


void  loadData(char* sFile, SSTATISTICS * stat)
/**************************************************************************/
/********** created by JIZHOU on 2017.10.18                     ***********/
/********** Function: to load two relations from file           ***********/
/**********           all sets will be reorganized according to freqency***/
/**********           space for sets will be alloced here               ***/
/**********           these spaces should be freed elsewhere explicitly ***/
/**********           repeated items are removed from each set          ***/
/**********           empty sets are deleted                            ***/
/**********           invalid sets in R is deleted                      ***/
/**********           relS: an array of sets, # of sets is in relS[0][0]***/
/**********                 relS[i][0] is the # of elements in ith set  ***/
/**************************************************************************/
{
    ELEMENTMAP  *  eleMap;
    FILE        *  inFile;
    int         *  tRel;
    int            set, item, maxEle;
    size_t         start_time,end_time;
    long long      totalLen;

    printf("DATA is LOADING....\n");
    start_time = clock();
    fprintf(logFp, "/***************************************/\n");
    fprintf(logFp, "/***************************************/\n\n");
    fprintf(logFp, "DATA LOADING....!\n");

    relS = NULL;
    /**********              *RelationS is loaded first      **************************/
    printf("Relation S is loading from file %s....\n",sFile);
    inFile = fopen(sFile,"r");                         //open file to load relationS
    if(!inFile)
    {
        fprintf(logFp, "Error: the file where S relation is stored can not be opened!\n");
        fclose(logFp);
        exit(0);
    }                                                  //the file is opened'
    /********scan the dataset to compute space costs of each set***********************/
    tRel = (int *)malloc(sizeof(int));
    tRel[0] = 0;
    maxEle  = 0;
    while(!feof(inFile))                                //to load sets one by one
    {
      fscanf(inFile,"%d   %d\n", &set, &item);
        maxEle = (item>maxEle)?item:maxEle;
        if(set > tRel[0])                            //a new set is met here
        {
            tRel = (int *)realloc(tRel, (set+1)*sizeof(int));

            for(int j=tRel[0]+1; j<=set; j++)
            {
                tRel[j] = 0;
            }
            tRel[0]=set;
        }
        tRel[set] ++;
    }
    printf("space is ready!\n");
    /********the size of each set has been obtained***********************/

    relS      = (int **)malloc( (tRel[0]+1)*sizeof(int *));         //initialize space for relationS
    relS[0]   = (int *)malloc(sizeof(int));
    relS[0][0]= tRel[0];
    for(int i=1; i<= tRel[0]; i++)
    {
        relS[i] = (int*)malloc((tRel[i]+1)*sizeof(int));
        memset(relS[i], 0, (tRel[i]+1)*sizeof(int));
    }
    free(tRel);
    eleMap = ( ELEMENTMAP  *)malloc((maxEle+1)*sizeof(ELEMENTMAP));//initialize space to record element in the universe
    memset(eleMap, 0, (maxEle+1)*sizeof(ELEMENTMAP));
    eleMap[0].element = maxEle;                              //initialize the number of elements in the universe
    fseek(inFile, 0, SEEK_SET);
   /**********to load data actually*********/
    while(!feof(inFile))                                //to load sets one by one
    {
        fscanf(inFile,"%d   %d\n", &set, &item);        //an item in a set is loaded
       /*
        if(set > relS[0][0])                            //a new set is met here
        {
            relS = (int **)realloc(relS, (set+1)*sizeof(int *));
            for(int j=relS[0][0]+1; j<=set; j++)
            {
                relS[j]    = (int *)malloc(sizeof(int));
                relS[j][0] = 0;
            }
            relS[0][0] = set;                                       // to record the number of sets
        }*/
        relS[set][0]++;
        //relS[set] = (int *)realloc(relS[set], (relS[set][0]+1)*sizeof(int));
        relS[set][relS[set][0]] = item;

        /*
        if(item  > eleMap[0].element)                               // an new element is met here
        {
            eleMap = (ELEMENTMAP *) realloc(eleMap, (item+1)*sizeof(ELEMENTMAP));
            for(int j= eleMap[0].element+1; j<=item; j++)
            {
                eleMap[j].element = j;
                eleMap[j].count   = 0;
            }
            eleMap[0].element = item;                           //to maintain the number of items in the universe
        }*/
        eleMap[item].count++;                                   //item from the universe appears a more time
    }
    printf("Data have been loaded in to memory! Valid each set...\n");
    //to check whether each loaded set is really a set
    for(int i= relS[0][0]; i>=1; i--)
    {
        if(validAset(relS[i], eleMap,1) == 0)
        {
            free(relS[i]);                   //invalid set is removed
            relS[i] = relS[ relS[0][0] ];
            relS[0][0] -- ;                  //the # of sets is adjusted
        }
    }
    for(int i=eleMap[0].element;i>=1;i--)
    {
        if(eleMap[i].count<1)
        {
            eleMap[i]=eleMap[eleMap[0].element];
            eleMap[0].element--;
        }
    }
    eleMap=( ELEMENTMAP  *)realloc(eleMap,(eleMap[0].element+1)*sizeof(ELEMENTMAP));
    fprintf(logFp, "RelationS is loeded!!\n");
    fprintf(logFp, "    %d elements are there in the universe of relationS!\n", eleMap[0].element);
    fprintf(logFp, "    %d sets are there in relationS!\n", relS[0][0]);
    fclose(inFile);
    /**********              *RelationS is loaded                                      **************************/
    end_time = clock();
    printf("DATA LOADING is loaded in %f seconds!\n", (end_time-start_time)/((double)CLOCKS_PER_SEC));
    fprintf(logFp, "DATA LOADING is loaded in %f seconds!\n", (end_time-start_time)/((double)CLOCKS_PER_SEC));

    /**************to map relation S    *************************************************************************/
    printf("Relation S is mappping according to frequency of items....\n");
    //sortMapByCount(eleMap, 1, eleMap[0].element);      //to sort eleMap according to count
    sortMapByCount(eleMap, relS[0][0]);                  //to sort eleMap according to count
    printf("items are sorted in ascending order of frequencies\n");
    statOfUniverse(eleMap,stat);
    printf("The universe set are statisticed\n");
    mapUniverseToNewOreder(eleMap);                                //to map each element in the universe to a new order
    printf("new order of items are generated\n");
    sortMapByElement(eleMap, 1, eleMap[0].element);                 //to sort eleMap according to element
    printf("preparing work are done\n");
    printf("Each set will be mapped one by one....\n");

    totalLen= 0;
    sortedSet = (int *)malloc(sizeof(int)*(relS[0][0]+1));
    sortedSet[0]=relS[0][0];
    for(int i =1; i<=relS[0][0]; i++)
    {
        sortedSet[i]=i;
        for(int j=1; j<=relS[i][0]; j++)
        {
            if(relS[i][j] > stat->biasTurnPos )
            {
                totalLen++;
            }
        }
        sortSet(relS[i],1,relS[i][0]);
    }

//    /**********************Sorted the set by the size of set***********************/
//    int temp;
//    int minSize,minSet;
//    for(int i=1;i<=sortedSet[0];i++)
//    {
//        minSize=INT_MAX;
//        minSet=-1;
//        //printf("i:%d\n",i);
//        for(int j=i;j<=sortedSet[0];j++)
//        {
//            if(relS[j][0]<minSize)
//            {
//                minSet=j;
//                minSize=relS[j][0];
//            }
//        }
//        if(minSize<relS[i][0])
//        {
//            temp=sortedSet[i];
//            sortedSet[i]=sortedSet[minSet];
//            sortedSet[minSet]=temp;
//        }
//    }
//    setSize = (int*)malloc(sizeof(int)*(sortedSet[sortedSet[0]]+1));
//    setSize[0]=sortedSet[sortedSet[0]];
//    int currentSize=0;
//    for(int i=1;i<sortedSet[0];i++)
//    {
//        if(sortedSet[i]>currentSize)
//        {
//            for(int j=currentSize+1;j<=sortedSet[i];j++)
//            {
//                setSize[j]=i;
//            }
//            currentSize=sortedSet[i];
//        }
//    }
//    /**********************End to sort the set       ******************************/
    stat->avgBiasTurnLen = ceil( totalLen/((double) relS[0][0]));
    printf("average bias turn length: %d\n", stat->avgBiasTurnLen);
    fprintf(logFp, "average bias turn length: %d\n", stat->avgBiasTurnLen);
    statOfRelationS(relS,stat);

    /*
    printf("all sets below:\n");
    printf("%d\n",relS[0][0]);
    for(int i=1; i<=relS[0][0]; i++)
    {
        //printf("before");
        printf("%d ",relS[i][0]);
        for(int j=1; j<=relS[i][0]; j++)
        {
            printf("%d ",relS[i][j]);
        }
        printf("\n");
    }*/

    //sortRelLex(relS, eleMap, 1, relS[0][0]);
    /**************to relationS is mapped ***********************************************************************/
    free(eleMap);
    end_time = clock();
    printf("PREPROCESSION is completed for %d sets in %f seconds!\n",relS[0][0], (end_time-start_time)/((double)CLOCKS_PER_SEC));
    (logFp, "PREPROCESSION is completed for %d sets in %f seconds!\n",relS[0][0], (end_time-start_time)/((double)CLOCKS_PER_SEC));
    fprintf(logFp, "/***************************************/\n");
    fprintf(logFp, "/***************************************/\n\n");
    fflush(logFp);
}

void   statOfUniverse(ELEMENTMAP * eleMap, SSTATISTICS * stat)
/****************************************************************************************/
/***********to compute the statistics of the Universe *                        **********/
/***********elements in the universe should be sorted according to their frequencies*****/
/****************************************************************************************/
{
    size_t tStart, tEnd;
    int total=0;
    double accumulate,tempAc;
    double  deviation, avg;
    int i,tPos,low, high;

    tStart = clock();
    stat->uSize = eleMap[0].element;
    fprintf(logFp, "uSize: %d \n", stat->uSize);

    /**the average frequency is computed first**/
    for(i=1; i<=eleMap[0].element; i++)
    {
        total += eleMap[i].count;
    }
    avg = total/(double)eleMap[0].element;      //average of frequencies is computed here
    stat->avgFre = avg;                 //average of frequencies is computed here
    fprintf(logFp, "avgFre: %.1f ", avg);
    printf("  avgFre: %.1f ",avg);

    /**the standard deviation of  frequencies is computed then**/
    stat->biasDegree=0;
    stat->peakDegree=0;
    tPos =1;
    accumulate = 0;
    for(i=1; i<=eleMap[0].element; i++)
    {
        if(eleMap[i].count <= avg )
        {
            tPos=i;
        }
        accumulate += (eleMap[i].count-avg)*(eleMap[i].count-avg);
        stat->biasDegree += pow(eleMap[i].count-avg,3);
        stat->peakDegree += pow(eleMap[i].count-avg,4);
    }
    accumulate /= stat->uSize;
    stat->biasDegree /= stat->uSize;
    stat->peakDegree /= stat->uSize;
    deviation   = sqrt(accumulate);
    stat->devFre= deviation;                 //deviation  is computed here
    stat->biasDegree /= pow(deviation,3);
    stat->peakDegree /= pow(deviation,4);
    fprintf(logFp, "devFre: %.2f  biasDegree: %.2f   peakDegree: %.2f \n", deviation, stat->biasDegree, stat->peakDegree);
    printf("devFre: %.2f  biasDegree: %.2f   peakDegree: %.2f \n", deviation, stat->biasDegree, stat->peakDegree);

    /*****to partition the frequencies into low, middle and high parts, partition points are determined**/
    //low frequency
    accumulate = 0;
    i =1;
    stat->minLowFre = eleMap[1].count;
    while(i<=eleMap[0].element && accumulate < ALPHA*total )
    {
        accumulate += eleMap[i].count;
        i++;
    }
    stat->maxLowFre = eleMap[i-1].count;          // max low frequency

    stat->lFrePos = i;                            // partition point
    stat->minMiddleFre = eleMap[i].count;         // min middle frequency
    stat->avgLowFre = accumulate/(i-1);           //average of low frequencies
    tempAc = accumulate;

    // middle frequency
    while(i<=eleMap[0].element && accumulate < (1-ALPHA)*total)
    {
        accumulate += eleMap[i].count;
        i++;
    }
    stat->maxMiddleFre = eleMap[i-1].count;

    stat->hFrePos = i;
    stat->minHighFre = eleMap[i].count;
    stat->avgMiddleFre = (accumulate-tempAc)/(i - stat->lFrePos);
    tempAc = accumulate;
    stat->avgHighFre = (total - tempAc)/(eleMap[0].element - stat->hFrePos + 1);
    stat->maxHighFre = eleMap[eleMap[0].element].count;
    printf("    partition points for different frequencies has been  determined \n");


    /**to compute the deviation of low, middle and high frequencies respectively*/
    deviation = 0;
    for(i=1; i<stat->lFrePos; i++)
    {
        deviation += (eleMap[i].count - stat->avgLowFre)*(eleMap[i].count - stat->avgLowFre);
    }
    stat->devLowFre = sqrt(deviation/(stat->lFrePos-1));
    fprintf(logFp, "minLowFre: %d \n", stat->minLowFre);
    fprintf(logFp, "maxLowFre: %d \n", stat->maxLowFre);
    fprintf(logFp, "avgLowFre: %f \n", stat->avgLowFre);
    fprintf(logFp, "devLowFre: %f \n", stat->devLowFre);
    fprintf(logFp, "lFrePos:   %d \n", stat->lFrePos);

    deviation = 0;
    for(i=stat->lFrePos; i<stat->hFrePos; i++)
    {
        deviation += (eleMap[i].count - stat->avgMiddleFre)*(eleMap[i].count - stat->avgMiddleFre);
    }
    stat->devMiddleFre = sqrt(deviation/(stat->hFrePos-stat->lFrePos));
    fprintf(logFp, "minMiddleFre: %d \n", stat->minMiddleFre);
    fprintf(logFp, "maxMiddleFre: %d \n", stat->maxMiddleFre);
    fprintf(logFp, "avgMiddleFre: %f \n", stat->avgMiddleFre);
    fprintf(logFp, "devMiddleFre: %f \n", stat->devMiddleFre);
    fprintf(logFp, "hFrePos:   %d \n", stat->hFrePos);

    deviation = 0;
    for(i=stat->hFrePos; i<=eleMap[0].element; i++)
    {
        deviation += (eleMap[i].count - stat->avgHighFre)*(eleMap[i].count - stat->avgHighFre);
    }
    stat->devHighFre = sqrt(deviation/(eleMap[0].element-stat->hFrePos+1));
    fprintf(logFp, "minHighFre: %d \n", stat->minHighFre);
    fprintf(logFp, "maxHighFre: %d \n", stat->maxHighFre);
    fprintf(logFp, "avgHighFre: %f \n", stat->avgHighFre);
    fprintf(logFp, "devHighFre: %f \n", stat->devHighFre);
    tEnd = clock();
    fprintf(logFp, "The cost to compute statistics is %f seconds!\n",(tEnd-tStart)/((double)CLOCKS_PER_SEC));

    if(stat->biasDegree >0)
    {
        low = tPos+1;
        high= stat->uSize;
    }
    else if(stat->biasDegree < 0)
    {
        low = 1;
        high= tPos-1;
    }
    else
    {
        low = 1;
        high =0;
    }
    deviation = stat->devFre;

    //adjust the avg
    while(low<high)
    {
        tPos = (low+high)/2;
        for(i=1; i<=eleMap[0].element; i++)
        {
            stat->biasDegree += pow(eleMap[i].count-eleMap[tPos].count,3);
        }
        if(stat->biasDegree<0)
        {
            high = tPos-1;
        }
        else if(stat->biasDegree>0)
        {
            low = tPos+1;
        }
        else
        {
            break;
        }
    }
    while(  (tPos<stat->uSize) && (eleMap[tPos].count == eleMap[tPos+1].count) )
    {
        tPos ++;
    }
    stat->biasTurnPos = tPos;
    stat->biasTurnFre = eleMap[stat->biasTurnPos].count;

    stat->histgram = (int*)malloc((stat->biasTurnFre+1)*sizeof(int));
    memset(stat->histgram, 0, (stat->biasTurnFre+1)*sizeof(int));
    int biasCount =0;
    int  bias=eleMap[1].count;
    i=1;
    while( eleMap[i].count <= stat->biasTurnFre)
    {
        if(eleMap[i].count == eleMap[i+1].count)
        {
            biasCount ++;
        }
        else
        {
            stat->histgram[bias] = biasCount;
            bias = bias=eleMap[i+1].count;
            biasCount =1;
        }
        i++;
    }
    fprintf(logFp, "Turn at %d-th element in %d elements with frequency: %d \n", stat->biasTurnPos, stat->uSize, eleMap[stat->biasTurnPos].count);
    printf("Turn at %d-th element in %d elements with frequency: %d \n", stat->biasTurnPos, stat->uSize, eleMap[stat->biasTurnPos].count);
    fprintf(logFp,"%d;%.1f;%.2f;%.2f;%.2f;%f;%f;%d;%d;%d;%f;%f;%d;%d;%d;%f;%f\n",stat->uSize,avg,deviation,stat->biasDegree, stat->peakDegree,stat->avgLowFre,stat->devLowFre,stat->lFrePos,stat->minMiddleFre,stat->maxMiddleFre,stat->avgMiddleFre,stat->devMiddleFre,stat->hFrePos,stat->minHighFre,stat->maxHighFre,stat->avgHighFre,stat->devHighFre);
}

void   statOfRelationS(int ** relS, SSTATISTICS * stat)
/*******************************************************************/
/***** to get statistics of relation S                       *******/
/*******************************************************************/
{
    int i;
    double avg,dev;

    stat->nRelS = relS[0][0];
    avg = 0;
    for(i=1; i<= relS[0][0]; i++)
    {
        avg += relS[i][0];
    }
    avg  /= relS[0][0];

    dev = 0;
    for(i=1; i<= relS[0][0]; i++)
    {
        dev += (relS[i][0]-avg)*(relS[i][0]-avg);
    }
    dev /= relS[0][0];
    dev  = sqrt(dev);
    stat->avgLen = avg;
    fprintf(logFp,"average length of sets in relation S: %f \n", avg);
    printf("average length of sets: %.2f standard deviation: %.3f\n", avg, dev);
    stat->devLen = dev;
    fprintf(logFp,"standard deviation of set length in relation S: %f \n", dev);
}



void  freeRelation(int ** rel)
{
    int i;
    if(rel == NULL || rel[0]==NULL)
    {
        return;
    }
    for(i=1; i<= rel[0][0]; i++)
    {
        if(rel[i]==NULL)
        {
            continue;
        }
        free(rel[i]);
    }
    free(rel[0]);
    free(rel);
}


void   mapUniverseToNewOreder(ELEMENTMAP * eleMap)
/***********************************************************************/
/********to map each element in the universe into a new order  *********/
/******** the input eleMap should be ordered according to count*********/
/***********************************************************************/
{
    for(int i=1; i<= eleMap[0].element; i++)
    {
        eleMap[i].newElement = i;
    }
}


//void   sortMapByCount(ELEMENTMAP * eleMap, int start, int end)
void     sortMapByCount(ELEMENTMAP * eleMap, int Max)
/***********************************************************************************************/
/********to sort the eleMap to a new order according to the frequency of each element  *********/
/********it runs very slow if quickSort is adapted here directly                       *********/
/********instead, CountingSort is used                                                 *********/
/********since all values to be sorted lay between 0 and relS[0][0]                    *********/
/***********************************************************************************************/
{
    int i,j,k;
    int *counting;
    ELEMENTMAP * temp;

    counting  = (int *)malloc((Max+1)*sizeof(int));
    memset(counting, 0, (Max+1)*sizeof(int));
    temp = (ELEMENTMAP*)malloc((eleMap[0].element+1)*sizeof(ELEMENTMAP));
    memcpy(temp, eleMap,(eleMap[0].element+1)*sizeof(ELEMENTMAP));

    for(i=1; i<= temp[0].element; i++)
    {
        j= temp[i].count;
        counting[j]++;
    }
    //now counting[j] is the # of times j appears in eleMap
    for(i=1; i<=Max; i++)
    {
        counting[i] = counting[i]+ counting[i-1];
    }//accumulate has finished

    for(i=1; i<= temp[0].element; i++)
    {
        j= temp[i].count;
        k= counting[j];
        eleMap[k] = temp[i];
        counting[j]--;
    }
    free(counting);
    free(temp);

    /**int i,j,pos;
    ELEMENTMAP  temp;
    if(start >= end) return;

    pos         = start + rand()%(end - start);            //randomized position between start and end
    temp        = eleMap[pos];                             //swap element
    eleMap[pos] = eleMap[end];
    eleMap[end] = temp;

    i = start-1;                                            //to partion
    for(j=start; j<end; j++)
    {
        if( eleMap[j].count <= eleMap[end].count)
        {
            i++;
            temp      = eleMap[j];
            eleMap[j] = eleMap[i];
            eleMap[i] = temp;
        }
    }
    i++;
    temp        = eleMap[end];
    eleMap[end] = eleMap[i];
    eleMap[i]   = temp;

    sortMapByCount(eleMap, start, i-1);                       //to iterate
    sortMapByCount(eleMap, i+1, end);                **/
}
void   sortMapByElement(ELEMENTMAP * eleMap, int start, int end)
/***********************************************************************************************/
/********to sort the eleMap to a new order according to the frequency of each element  *********/
/********it runs slowly if quickSort is adapted here directly                          *********/
/***********************************************************************************************/
{
    ELEMENTMAP *  temp;
    int i,j;

    temp = (ELEMENTMAP *)malloc((eleMap[0].element+1)*sizeof(ELEMENTMAP));
    memcpy(temp, eleMap, (eleMap[0].element+1)*sizeof(ELEMENTMAP));
    //eleMap[0] keeps unchanged
    for(i=1; i<= temp->element; i++)
    {
        j= temp[i].element;
        eleMap[j] = temp[i];
    }
    free(temp);
    /**int i,j,pos;
    ELEMENTMAP  temp;
    if(start >= end) return;

    pos         = start + rand()%(end - start);            //randomized position between start and end
    temp        = eleMap[pos];                             //swap element
    eleMap[pos] = eleMap[end];
    eleMap[end] = temp;

    i = start-1;                                            //to partion
    for(j=start; j<end; j++)
    {
        if( eleMap[j].element <= eleMap[end].element)
        {
            i++;
            temp      = eleMap[j];
            eleMap[j] = eleMap[i];
            eleMap[i] = temp;
        }
    }
    i++;
    temp        = eleMap[end];
    eleMap[end] = eleMap[i];
    eleMap[i]   = temp;

    sortMapByElement(eleMap, start, i-1);                       //to iterate
    sortMapByElement(eleMap, i+1, end);**/
}

void   sortSet(int * set, int start, int end)
/**********************************    #ifdef  DEBUG
    resultCompare(result1, result2);
    #endif // DEBUG*************************************************************/
/********to sort a set in increseacing order of elements                               *********/
/********quickSort is adapted here directly                                            *********/
/***********************************************************************************************/
{
    int i,j,pos;
    int  temp;
    if(start >= end) return;

    pos         = start + rand()%(end - start);            //randomized position between start and end
    temp        = set[pos];                             //swap element
    set[pos]    = set[end];
    set[end]    = temp;

    i = start-1;                                            //to partion
    for(j=start; j<end; j++)
    {
        if( set[j] <= set[end])
        {
            i++;
            temp      = set[j];
            set[j]    = set[i];
            set[i]    = temp;
        }
    }
    i++;
    temp        = set[end];
    set[end]    = set[i];
    set[i]      = temp;

    sortSet(set, start, i-1);                       //to iterate
    sortSet(set, i+1, end);
}

//void  sortRelLex(int ** aRel, int * oMap, int pStart, int pEnd)
///*****************************************************************************/
///******aRel are sorted in lexicographic order of sets             ************/
///******i th set in the new order is the oMap[i] th set in original order******/
///*****************************************************************************/
//{
//    int * sBegin, *eEnd, low, high,pos, iTemp, *ptrTemp,m, i,j;
//
//    m=0;
//
//    sBegin  = (int*) malloc(pEnd * sizeof(int));
//    eEnd    = (int*) malloc(pEnd * sizeof(int));
//
//    sBegin[0] = pStart;
//    eEnd[0]   = pEnd;
//    srand(time(NULL));
//    while(m >= 0)
//    {
//        low = sBegin[m];   //pop an iteration
//        high= eEnd[m];
//        m--;
//
//        if(low >= high)
//        {
//            continue;
//        }
//        pos = low + rand()%(high-low+1);
//
//
//        iTemp      = oMap[high];
//        oMap[high] = oMap[pos];
//        oMap[pos]  = iTemp;
//
//        ptrTemp    = aRel[high];
//        aRel[high] = aRel[pos];
//        aRel[pos]  = ptrTemp;
//
//        i = low-1;
//        for(j=low; j<high; j++)
//        {
//            if(compareSets(aRel[j], aRel[high]) <= 0 )
//            {
//                i++;
//
//                iTemp   = oMap[j];
//                oMap[j] = oMap[i];
//                oMap[i] = iTemp;
//
//                ptrTemp    = aRel[j];
//                aRel[j]    = aRel[i];
//                aRel[i]    = ptrTemp;
//            }
//        }
//        i++;
//        iTemp     = oMap[high];
//        oMap[high] = oMap[i];
//        oMap[i]    = iTemp;
//
//        ptrTemp    = aRel[high];
//        aRel[high] = aRel[i];
//        aRel[i]    = ptrTemp;
//
//        m++;   //push
//        sBegin[m] = low;
//        eEnd[m]   = i-1;
//        m++;
//        sBegin[m] = i+1;
//        eEnd[m]   = high;
//    }
//
//    free(sBegin);
//    free(eEnd);
//    /**int i,j, pos, iTemp;
//    int * ptrTemp;
//
//    if(pStart >= pEnd) return;
//
//    pos = pStart + rand()%(pEnd - pStart);
//    iTemp      = oMap[pEnd];
//    oMap[pEnd] = oMap[pos];
//    oMap[pos]  = iTemp;
//
//    ptrTemp    = aRel[pEnd];
//    aRel[pEnd] = aRel[pos];
//    aRel[pos]  = ptrTemp;
//
//    i=pStart -1;
//    for(j=pStart; j<pEnd; j++)
//    {
//        if(compareSets(aRel[j], aRel[pEnd]) <= 0 )
//        {
//            i++;
//
//            iTemp   = oMap[j];
//            oMap[j] = oMap[i];
//            oMap[i] = iTemp;
//
//            ptrTemp    = aRel[j];
//            aRel[j]    = aRel[i];
//            aRel[i]    = ptrTemp;
//        }
//    }
//    i++;
//    iTemp     = oMap[pEnd];
//    oMap[pEnd] = oMap[i];
//    oMap[i]    = iTemp;
//
//    ptrTemp    = aRel[pEnd];
//    aRel[pEnd] = aRel[i];
//    aRel[i]    = ptrTemp;
//
//
//    sortRelLex(aRel, oMap, pStart, i-1);
//    sortRelLex(aRel, oMap, i+1, pEnd);**/
//}


int    validAset(int * aSet,ELEMENTMAP *eleMap, int flag)
/******************************************************************/
/*********to remove repeated items from the set and then **********/
/*********to determine whether a set is valid            **********/
/**********return 0, if it is an empty set               **********/
/**********return 1, otherwise                           **********/
/******************************************************************/
{
    int i,j;
    if(aSet == NULL || aSet[0] == 0)
    {
        return 0;
    }
    sortSet(aSet,1, aSet[0]);
    i=1;
    for(j=2; j<=aSet[0]; j++)
    {
        if(aSet[j] != aSet[j-1])
        {
            i++;
            aSet[i]=aSet[j];
        }
        else if (flag == 1)    //duplicate elements are removed, their count is decreased
        {
            eleMap[ aSet[j] ].count--;
        }
    }
    aSet[0] = i;

    return 1;
}


int binarySearch(const int *const data, int target, int sPos)
/******************************************************************/
/*******to search target in data between sPos and end    **********/
/*******If found, return a value >0                      **********/
/*******Else      return a value <=0                     **********/
/******************************************************************/
{
    int mid;
    int low, high;
    if(sPos>data[0]) return -1;
    low  = sPos;
    high = data[0];
    while(low <= high)
    {
        mid = (low+high)/2;
        if(target == data[mid])
        {
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
    return -1;
}



int   verifyIntersection(const int * const r, const int * const s, int threshold)
/**********************************************************************************/
/*******Input: two sorted sets, i.e., all elements in each set are sorted**********/
/*******to verify whether |r N S|>= threshold                             *********/
/*******If Yes,   return a value >0                                      **********/
/*******Else      return a value <=0                                     **********/
/**********************************************************************************/
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
    return (rValue>=threshold)?1:0;
}


int       verifyContainment(const int * const r, const int * const s)
/**********************************************************************************/
/*******to verify whether rth tuple in R is contained in sth tuple in S   *********/
/*******If Yes,   return a value >0                                      **********/
/*******Else      return a value <=0                                     **********/
/**********************************************************************************/
{
    int i,pos;
    pos = 1;
    i=1;
    while(i<=r[0])
    {
        pos = binarySearch(s, r[i], pos);
        /*#ifdef stuDEBUG
        if(pos <= 0)
        {
            printf("i:%d\n",i);
        }
        #endif // stuDEBUG*/
        if(pos <= 0 ) return -1;
        pos ++;
        i++;
    }
    return 1;
}

void  outputAset(const int * data,FILE *log)
{
    if(data == NULL) return;
    fprintf(log, "\nA set is: ");

    for(int i=1; i<= data[0]; i++)
    {
        fprintf(log, "%d,", data[i]);
    }
    fprintf(log, "\n");
}


void  outputRel(int ** rel,FILE *log)
{
    if(rel==NULL || rel[0]==NULL) return;
    for(int i=1; i<=rel[0][0]; i++)
    {
        outputAset(rel[i],log);
    }
}


void output_result(int **join, char *outfile)
{
    int i,j;
    FILE *fp;
    //printf("join[0][0]:%d\n",join[0][0]);
    fp = fopen(outfile,"w+");
    for(i=1; i<=join[0][0]; i++)
    {
        fprintf(fp,"node:%d\n",i);
        //printf("node:%d\n",i);
        for(j=1; j<=join[i][0]; j++)
        {
            fprintf(fp,"%d ",join[i][j]);
            //printf("%d ",join[i][j]);
        }
        fprintf(fp,"\n");
        //printf("\n");
    }
    fclose(fp);
}

void  free_join(int **join)
{
    int i,temp;
    temp = join[0][0];
    for(i = 0; i<=temp; i++)
    {
        free(join[i]);
    }
    free(join);
}
//int  compareSets(int * fSet, int * sSet)
///***************************************************************************/
///****to compare two ordered sets lexicographically                  ********/
///**** return 0, if the two sets are the same                        ********/
///**** return 1, if fSet> sSet                                       ********/
///**** return -1, otherwise                                          ********/
///***************************************************************************/
//{
//    int i;
//    i=1;
//    while((i<= fSet[0]) && (i<= sSet[0]))
//    {
//        if(fSet[i] < sSet[i])
//        {
//            return -1;
//        }
//        else if(fSet[i] > sSet[i])
//        {
//            return 1;
//        }
//        i++;
//    }
//    if(fSet[0]==sSet[0])
//    {
//        return 0;
//    }
//    else if(fSet[0] < sSet[0])
//    {
//        return -1;
//    }
//    return 1;
//}


#ifdef  DEBUG
void  resultCompare(int ** res1, int ** res2)
{
    int i,j;
    if(res1 == NULL || res2 == NULL)
    {
        return;
    }
    if((res1[0]== NULL && res2[0]!=NULL) || (res1[0] != NULL && res2[0] ==NULL) )
    {
        printf("Results to be compared are not from the same join!\n");
        return;
    }
    if(res1[0][0] != res2[0][0])
    {
        printf("Results to be compared are not from the same relation R!\n");
        return;
    }
    for(i=1; i<= res1[0][0]; i++)
    {
        sortSet(res1[i], 1, res1[i][0]);
        sortSet(res2[i], 1, res2[i][0]);
        j=1;
        while(j <= res1[i][0] && j <= res2[i][0])
        {
            if(res1[i][j] != res2[i][j])
            {
                if(res1[i][j] < res2[i][j])
                {
                    printf("Inconsistent result: r: %d, s:%d  in the first result but not in second result\n", i, res1[i][j]);
                    fprintf(logFp, "Inconsistent result: r: %d, s:%d  in the first result but not in second result\n", i, res1[i][j]);
                }
                else
                {
                    printf("Inconsistent result: r: %d, s:%d  in the second result but not in first result\n", i, res2[i][j]);
                    fprintf(logFp, "Inconsistent result: r: %d, s:%d  in the first result but not in second result\n", i, res2[i][j]);
                }
                return;
            }
            j++;
        }
        if(res1[i][0] > res2[i][0])
        {
            printf("Inconsistent result: r: %d, s:%d  in the first result but not in second result\n", i, res1[i][j]);
            fprintf(logFp, "Inconsistent result: r: %d, s:%d  in the first result but not in second result\n", i, res1[i][j]);
        }
        if(res1[i][0] < res2[i][0])
        {
            printf("Inconsistent result: r: %d, s:%d  in the second result but not in first result\n", i, res2[i][j]);
            fprintf(logFp, "Inconsistent result: r: %d, s:%d  in the first result but not in second result\n", i, res2[i][j]);
        }
    }
    return;
}
void initResult(const int ** const relR)
{
    int ** join;
    if(relR==NULL  || relR[0]==NULL)
    {
        return;
    }

    join = (int **)malloc((relR[0][0]+1)*sizeof(int *));
    for(int i = 0; i<= relR[0][0]; i++)
    {
        join[i] = (int *)malloc(sizeof(int));
        join[i][0] = 0;
    }
    join[0][0] = relR[0][0];
    if(result2 == NULL)
    {
        result2 = join;
    }
    else if(result2 != NULL && result1 == NULL)
    {
        result1 = result2;
        result2 = join;
    }
    else
    {
        freeRelation(result1);
        result1 = result2;
        result2 = join;
    }
    return ;
}
#endif

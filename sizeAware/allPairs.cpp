#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <math.h>
#include  <time.h>
#include  "allPairs.h"
#include  "dataload.h"

#ifndef  L_debug
#define  L_debug
#endif // L_debug

//using namespace std;
//extern long long memorySize;
long long        maxMem;
extern FILE * logFp;
extern FILE * logs2;



long long  AllPairsJoin(int ** rel, int threshold)
{
    long long rValue, vFiltered, pFiltered;
    int       **inverted;
    bool      *Processed;
    size_t    start, end;


    printf("\n\nAllPairJoin is ready to run........\n");
    fprintf(logFp, "\n\nAllPairJoin is ready to run........\n");
    start = clock();
    printf("AllPairJoin algorithm begins with the inverted index built incrementally....\n");
    fprintf(logFp, "AllPairJoin algorithm begins with the inverted index built incrementally....\n");
    rValue    = 0;       //the number of join results
    vFiltered = 0;
    pFiltered = 0;
    Processed = (bool*)malloc(sizeof(bool)*(rel[0][0]+1)); //space used to avoid repeated results

    inverted      = (int**)malloc(sizeof(int*));//inverted index
    inverted[0]   = (int*) malloc(sizeof(int));
    inverted[0][0]=0;

    char   logFile[50]="allPairslog.txt";
    FILE *  allPairslog;
    allPairslog = fopen(logFile,"w+");
    if(!allPairslog)
    {
        printf("Error: can not open the log file!\n");
        exit(0);
    }
    //memorySize   += sizeof(int*)+sizeof(int);
    for(int i=1; i<= rel[0][0]; i++)
    {
        memset(Processed, 0, sizeof(bool)*(rel[0][0])+1);
        for(int j=1; j<= rel[i][0]-threshold+1; j++)
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
                if(Processed[ inverted[ rel[i][j] ][join] ]) {pFiltered++; continue;}
                if( verifyIntersection(rel[i], rel[inverted[ rel[i][j] ][join] ], threshold)==1)
                {
                    rValue++;
                }
                else
                {
                    vFiltered++;
                }
                Processed[ inverted[ rel[i][j] ][join] ]= true;
            }
        }
    }
    free(Processed);
    AllPairsFreeInverted(inverted);
    end = clock();
    printf("AllPairJoin algorithm generates %lld results in %3f seconds!\n", rValue, (end-start)/(double)CLOCKS_PER_SEC);
    printf("            Processed     avoids %lld repeated candidate pairs \n", pFiltered);
    printf("            verification removes %lld candidate pairs among %lld total\n", vFiltered, vFiltered+rValue);
    fprintf(logFp, "AllPairJoin algorithm generates %lld results in %3f seconds!\n", rValue, (end-start)/(double)CLOCKS_PER_SEC);
    fprintf(logFp, "           Processed     avoids %lld repeated candidate pairs \n", pFiltered);
    fprintf(logFp,"            verification removes %lld candidate pairs among %lld total\n", vFiltered, vFiltered+rValue);
    fprintf(logs2,"%3f ",(end-start)/(double)CLOCKS_PER_SEC);
    #ifdef L_debug
    fflush(allPairslog);
    #endif // L_debug
    return  rValue;
}

void   AllPairsFreeInverted(int ** iIndex)
{
    if(iIndex == NULL || iIndex[0]==NULL) return;
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

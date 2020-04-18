#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <time.h>
#include  <math.h>
#include  <iostream>
#include "dataload.h"
#include "scanCount.h"

extern FILE *logFp;
extern FILE *logs2;

using namespace std;

long long  ScanCountJoin(int** rel, int threshold)
{
    int *countR;
    int **inverted;
    size_t      start, end;
    long long rValue;
    printf("\n\n####################     SCANCOUNT    ############################\n");
    printf("scanCount is ready to run........\n");
    fprintf(logFp, "\n\n####################    SCANCOUNT   ############################\n");
    fprintf(logFp, "scanCount is ready to run........\n");

    rValue=0;

    start=clock();
    inverted=getInvertedIndex(rel);

    cout<<"Finish inverted"<<endl;
    countR = (int *)malloc(sizeof(int)*(rel[0][0]+1));
    for(int i=1;i<=rel[0][0];i++)
    {
        memset(countR, 0, sizeof(int)*(rel[0][0]+1));
        for(int j=1;j<=rel[i][0];j++)
        {
            for(int k=1;k<=inverted[rel[i][j]][0];k++)
            {
                int t = inverted[rel[i][j]][k];
                countR[t]++;
                if(t>i&&countR[t]==threshold)
                {
                    rValue++;
                }
            }
        }
    }
    free(countR);
    ScanCountFreeInverted(inverted);

    end = clock();
    printf("scan-count algorithm generates %lld results in %3f seconds!\n", rValue, (end-start)/(double)CLOCKS_PER_SEC);
    fprintf(logFp, "scan-count algorithm generates %lld results in %3f seconds!\n", rValue, (end-start)/(double)CLOCKS_PER_SEC);
    fprintf(logs2,"%3f ",(end-start)/(double)CLOCKS_PER_SEC);
}


int  **getInvertedIndex(int ** rel)
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
    return  inverted;
}

void   ScanCountFreeInverted(int ** iIndex)
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

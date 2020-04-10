#include  <stdio.h>
#include  <stdlib.h>
#include  <time.h>
#include  <iostream>
#include "divideSkip.h"
#include "mergeSkip.h"

extern FILE*logFp;

using namespace std;

long long MergeSkip(int** rel, int threshold)
{
    int **inverted,**inverted_temp;
    size_t      start, end;
    long long rValue;
    ShortSet *candidate;
    int L;
    printf("\n\n####################     Merge-SKIP    ############################\n");
    printf("merge-skip is ready to run........\n");
    fprintf(logFp, "\n\n####################    Merge-SKIP   ############################\n");
    fprintf(logFp, "merge-skip is ready to run........\n");

    rValue=0;

    start=clock();
    inverted=getInverted(rel);
    cout<<"Finish inverted"<<endl;
    //outputInverted(inverted);

    for(int i=1; i<=rel[0][0]; i++)
    {
        inverted_temp = (int**)malloc((rel[i][0]+1)*sizeof(int*));
        inverted_temp[0] = (int *) malloc(sizeof(int));
        inverted_temp[0][0] = rel[i][0];
        for(int j = 1; j<=rel[i][0]; j++)
        {
            inverted_temp[j] = inverted[rel[i][j]];
        }
        //outputInverted(inverted_short);
        candidate = MergeSkip(threshold,inverted_temp,i);
        rValue+=candidate[0].num;
        free(candidate);
        free(inverted_temp[0]);
        free(inverted_temp);
    }

    DivideSkipFreeInverted(inverted);
    end = clock();
    printf("merge-skip algorithm generates %lld results in %3f seconds!\n", rValue, (end-start)/(double)CLOCKS_PER_SEC);
    fprintf(logFp, "merge-skip algorithm generates %lld results in %3f seconds!\n", rValue, (end-start)/(double)CLOCKS_PER_SEC);

}

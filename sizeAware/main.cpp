#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <iostream>
#include  <time.h>
#include  <vector>
#include  "filter.h"
#include  "dataload.h"
#include  "allPairs.h"
#include  "sizeAware.h"
#include  "sizewareasfilter.h"
#include  "scanCount.h"
#include  "divideSkip.h"
#include  "mergeSkip.h"
#include  "transferPy.h"
FILE *  logFp;
FILE *  logs2;
vector<double>args;
int  ** relS;
int  *  sortedSet;
int  *  setSize;

#ifdef stuDEBUG
int ** join;
#endif // stuDEBUG
#ifndef  L_debug
#define  L_debug
#endif // L_debug

int main(int argc, char *argv[])
//int main()
/*****************************************************************************/
/*******     Created on 2017.10.18                                     *******/
/*******     Run to compare performance of different metholds          *******/
/*****************************************************************************/
{
    int    thresh[13]  = {2,4,6,8,10,12,14,16,20,24,28,32,36};
    char   logFile[50]="logs\\log.txt";
    char   logFile2[50]="log\\log.txt";
    //char   sFile[50]="test.txt";//"AOL_out.txt";//"com-orkut.ungraph.txt"; //"Netflix_out.txt";
    char   sFile[50];//="AOL_out.txt";//"com-orkut.ungraph.txt"; //"Netflix_out.txt";
    SSTATISTICS  stat;
    int * setMap;

    strcpy(sFile, argv[1]);
    //to prepare log file where all information during running will be recorded
    logFp = fopen(logFile,"w+");
    logs2 = fopen(logFile2,"w+");
    if(!logFp)
    {
        printf("Error: can not open the log file!\n");
        exit(0);
    }

    loadData(sFile, &stat);
#ifdef  L_debug
    setMap  = (int *)malloc((relS[0][0]+1)*sizeof(int));
    for(int i=1; i<= relS[0][0]; i++)
    {
        setMap[i]=i;
    }
    size_t  sortTimeStart =clock();
#endif // L_debug
    for(int i=1; i<13; i++)
    {
        vector<double> t_args;
        t_args.assign(args.begin(),args.end());
        t_args.push_back(thresh[i]);
        cout<<transferPython(t_args)<<endl;
        printf("\n\n$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
        printf("$$$$$$$$$$$$$$$$$THRESHOLD: %3d      $$$$$$$$$$$$$$$$$$$$$\n", thresh[i]);
        printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
        fprintf(logFp,"\n\n$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
        fprintf(logFp,"$$$$$$$$$$$$$$$$$THRESHOLD: %3d      $$$$$$$$$$$$$$$$$$$$$\n", thresh[i]);
        fprintf(logFp,"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");

        SizeAwareJoin(relS, thresh[i]);
        //AllPairsJoin(relS, thresh[i]);
        ScanCountJoin(relS,thresh[i]);
        DivideSkip(relS,thresh[i]);
        //MergeSkip(relS,thresh[i]);
        fprintf(logs2,"\n");
        //newFilterAllPairsJoin(relS,10);
        //filterAndRefine(&stat,relS, thresh[i]);
    }
    fclose(logFp);
    fclose(logs2);
    freeRelation(relS);
    free(stat.histgram);

    return 1;
}

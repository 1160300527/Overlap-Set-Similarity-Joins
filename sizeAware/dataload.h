#ifndef UTILITY_H_INCLUDED
#define UTILITY_H_INCLUDED

#ifndef  ALPHA                   //parameter in freHashJoin
#define  ALPHA   0.25
#endif

#ifndef  KLF                     //parameter in ttjoin
#define  KLF     5
#endif


//#ifndef  DEBUG
//#define  DEBUG
//#endif

typedef struct  ELEMENTMAP{
    int   element;
    int   count;                //to record totally times of an element in relation R;
    int   newElement;           //each element will be mapped to a newElement after data is loaded
}ELEMENTMAP;

typedef struct  SSTATISTICS{
    int    uSize;                 //# of elements in the universe set;
    int    nRelS;                 //# of tuples in relS
    int    lFrePos;               //the position of low frequency  in mapped universe set
    int    hFrePos;               //the position of high frequency in mapped universe set
    int    minLowFre;
    int    maxLowFre;
    int    minHighFre;
    int    maxHighFre;
    int    minMiddleFre;
    int    maxMiddleFre;
    int    rIndexedLowFrePos;     //the low frequency from view of relation R
    int    biasTurnPos;
    int    biasTurnFre;
    int  * histgram;
    int    avgBiasTurnLen;
    double avgFre;                //the average frequency of each elements in the universe
    double devFre;                //the standard deviation of the frequencies
    double avgLen;                //avgLen of relation S;
    double devLen;                //the standard deviation of the lengths of all sets in Relation S
    double avgLowFre;             //the average of low frequencies
    double devLowFre;             //the standard deviation of low frequencies
    double avgHighFre;             //the average of low frequencies
    double devHighFre;             //the standard deviation of low frequencies
    double avgMiddleFre;           //the average of middle frequencies
    double devMiddleFre;           //the standard deviation of low frequencies
    double biasDegree;
    double peakDegree;

}SSTATISTICS;


void  loadDataSizeAware(char* sFile, SSTATISTICS * stat);
void  loadData(char* sFile, SSTATISTICS * stat);
void  freeRelation( int ** rel);
int   compareSets(int * fSet, int * sSet);
//int   sdjustzero(int ** aRel, int * oMap);
void  sortRelLex(int ** aRel, int * oMap, int pStart, int pEnd);
int   binarySearch(const int *const data, int target, int sPos);
int   verifyContainment(const int * const r, const int * const s);
int   verifyIntersection(const int * const r, const int * const s, int threshold);
void  outputAset(const int *data,FILE *log);
void  outputRel(int ** rel,FILE *log);
void  output_result(int **join, char *outfile);
void  free_join(int **join);

#ifdef  DEBUG
void  resultCompare(int ** res1, int ** res2);
void initResult(const int ** const relR);
#endif
#endif // UTILITY_H_INCLUDED

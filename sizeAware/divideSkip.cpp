#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <time.h>
#include  <math.h>
#include  <iostream>
#include  <queue>
#include  "dataload.h"
#include  "divideSkip.h"


#define param 0.0085

extern FILE *logFp;


using namespace std;
class InvertHeap
{
private:
    int** _invert;
    int _size;
    int _len;
    void swapInvert(int i,int j);
public:
    InvertHeap(int L);
    ~InvertHeap();
    int* add(int *List);
    void adjust();
    int **getInverted();
    int getTimes(int R);
};
void outputInverted(int **inverted);
int getParamL(int **inverted,int **rel,int R,int t);


long long  DivideSkip(int** rel, int threshold)
{
    int **inverted,**inverted_short;
    ShortSet *candidate;
    size_t      start, end;
    long long rValue;
    InvertHeap* heap;
    int L;
    printf("\n\n####################     DIVIDE-SKIP    ############################\n");
    printf("divide-skip is ready to run........\n");
    fprintf(logFp, "\n\n####################    DIVIDE-SKIP   ############################\n");
    fprintf(logFp, "divide-skip is ready to run........\n");

    rValue=0;

    start=clock();
    inverted=getInverted(rel);
    cout<<"Finish inverted"<<endl;
    //outputInverted(inverted);



    for(int i=1; i<=rel[0][0]; i++)
    {
        L=getParamL(inverted,rel,i,threshold);
        L=0;
        if(rel[i][0]<threshold||L==threshold)
        {
            continue;
        }
        inverted_short = (int **) malloc((rel[i][0]-L+1)*sizeof(int *));
        inverted_short[0] = (int *) malloc(sizeof(int));
        inverted_short[0][0] = 0;
        heap = new InvertHeap(L);
        for(int j=1; j<=rel[i][0]; j++)
        {
            int *invert_list = heap->add(inverted[rel[i][j]]);
            if(invert_list!=NULL)
            {
                inverted_short[0][0] = inverted_short[0][0]+1;
                inverted_short[inverted_short[0][0]] = invert_list;
            }
        }
        //outputInverted(inverted_short);
        candidate = MergeSkip(threshold-L,inverted_short,i);
//        cout<<candidate[0].num<<endl;
        for(int j = 1; j <= candidate[0].num; j++)
        {
            int times = heap->getTimes(candidate[j].id);
            if(times+candidate[j].num>=threshold)
            {
                rValue+=1;
            }
        }
        free(candidate);
        free(inverted_short[0]);
        free(inverted_short);
        delete heap;
    }

    DivideSkipFreeInverted(inverted);
    end = clock();
    printf("divide-skip algorithm generates %lld results in %3f seconds!\n", rValue, (end-start)/(double)CLOCKS_PER_SEC);
    fprintf(logFp, "divide-skip algorithm generates %lld results in %3f seconds!\n", rValue, (end-start)/(double)CLOCKS_PER_SEC);

}

ShortSet* MergeSkip(int c,int **inverted,int R)
{
    priority_queue<SetInverted,vector<SetInverted>,greater<SetInverted>>Heap;
    vector<int> visit;
    int *start,*ending;
    int n;
    ShortSet *Rs;
    SetInverted top,t;

    start = (int*) malloc((inverted[0][0]+1)*sizeof(int));
    ending = (int*) malloc((inverted[0][0]+1)*sizeof(int));

    Rs = (ShortSet*) malloc(sizeof(ShortSet));
    Rs[0].num = 0;

    for(int i = 1; i<=inverted[0][0]; i++)
    {
        ending[i] = inverted[i][0];
        start[i] = BinarySearchInvert(1,ending[i],inverted[i],R+1);
        if(start[i]<=ending[i])
        {
            SetInverted v;
            v.inverted_list = i;
            v.value = inverted[i][start[i]];
            Heap.push(v);
            start[i]++;
        }
    }
    if(Heap.size()<c)
    {
        free(start);
        free(ending);
        return Rs;
    }

    t = Heap.top();
    n = 0;
    while(Heap.size()+n>=c)
    {
        while(!Heap.empty())
        {
            top = Heap.top();
            if(t.value == top.value)
            {
                Heap.pop();
                visit.push_back(top.inverted_list);
                n++;
            }
            else
            {
                break;
            }
        }
        if(n>=c)
        {
            Rs[0].num = Rs[0].num+1;
            Rs = (ShortSet*) realloc(Rs,(Rs[0].num+1)*sizeof(ShortSet));
            Rs[Rs[0].num].id = t.value;
            Rs[Rs[0].num].num = n;
            n=0;
            while(!visit.empty())
            {
                int id = visit.back();
                visit.pop_back();
                if(start[id]<=ending[id])
                {
                    SetInverted v;
                    v.inverted_list = id;
                    v.value = inverted[id][start[id]];
                    Heap.push(v);
                    start[id] = start[id]+1;
                }
            }
        }
        else
        {
            t = top;
            int m = n;
            n = 0;
            for(int i = 0; i<c-m-1; i++)
            {
                top = Heap.top();
                Heap.pop();
                visit.push_back(top.inverted_list);
                if(top.value == t.value)
                {
                    n++;
                }
                else
                {
                    t = top;
                    n=1;
                }
            }
            int key = t.value;
            while(!visit.empty())
            {
                int id = visit.back();
                visit.pop_back();
                int index = BinarySearchInvert(start[id],ending[id],inverted[id],key);
                if(index<=ending[id])
                {
                    start[id] = index+1;
                    SetInverted v;
                    v.inverted_list = id;
                    v.value = inverted[id][index];
                    Heap.push(v);
                }
            }
        }

    }
    free(start);
    free(ending);
    return Rs;

}

int BinarySearchInvert(int start,int ending,int *invert,int value)
{
    int m=(start+ending)/2;
    while(start<=ending)
    {
        if(invert[m]>value)
        {
            ending=m-1;
            m=(start+ending)/2;
        }
        else if(invert[m]<value)
        {
            start=m+1;
            m=(start+ending)/2;
        }
        else
        {
            return m;
        }
    }
    if(invert[m]>value)
    {
        return m;
    }
    else
    {
        return ending+1;
    }
}
int  **getInverted(int ** rel)
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

void   DivideSkipFreeInverted(int ** index)
{
    if(index == NULL || index[0]==NULL)
        return;
    for(int i=1; i<= index[0][0]; i++)
    {
        if(index[i]!=NULL)
        {
            free(index[i]);
        }
    }
    free(index[0]);
    free(index);
}

void outputInverted(int **inverted)
{
    for(int i=1; i<=inverted[0][0]; i++)
    {
        cout<<i<<":";
        for(int j=1; j<=inverted[i][0]; j++)
        {
            cout<<inverted[i][j]<<"\t";
        }
        cout<<endl;
    }
}


int getParamL(int **inverted,int **rel,int R,int t)
{
    int max = 0;
    for(int i=1; i<=rel[R][0]; i++)
    {
        if(inverted[rel[R][i]][0]>max)
        {
            max=inverted[rel[R][i]][0];
        }
    }
    if(max==1)
    {
        return t;
    }
    return (int)t/(param*log2(max)+1);
}



void InvertHeap::swapInvert(int i,int j)
{
    int *temp = _invert[i];
    _invert[i] = _invert[j];
    _invert[j] = temp;
}

InvertHeap::InvertHeap(int L)
{
    _invert = (int **) malloc((L+1)*sizeof(int **));
    _invert[0] = (int *) malloc(sizeof(int));
    _invert[0][0] = 0;
    _size = 0;
    _len = L;
}

InvertHeap::~InvertHeap()
{
    free(_invert[0]);
    free(_invert);
}



int* InvertHeap::add(int *List)
{
    if(_len==0)
    {
        return List;
    }
    if(_size<_len)
    {
        int i;
        for(i = ++_size; i>1&&_invert[i/2][0]>List[0]; i/=2)
        {
            _invert[i]=_invert[i/2];
        }
        _invert[i]=List;
        _invert[0][0] = _size;
        return NULL;
    }
    else if(List[0]>_invert[1][0])
    {
        int *temp = _invert[1];
        _invert[1] = List;
        adjust();
        return temp;
    }
    else
    {
        return List;
    }
}

void InvertHeap::adjust()
{
    int left,right;
    int index = 1;
    while(index<=_size/2)
    {
        left = 2*index;
        right = 2*index+1;
        if(right>_size||_invert[left][0]<=_invert[right][0])
        {
            if(_invert[left][0]<_invert[index][0])
            {
                swapInvert(index,left);
                index = left;
            }
            else
            {
                break;
            }
        }
        else
        {
            if(_invert[right][0]<_invert[index][0])
            {
                swapInvert(index,right);
                index = right;
            }
            else
            {
                break;
            }
        }
    }

}

int InvertHeap::getTimes(int value)
{
    int start,ending,m,times;
    times=0;
    for(int i=1; i<=_size; i++)
    {
        start = 1;
        ending = _invert[i][0];
        m=(start+ending)/2;
        while(start<=ending)
        {
            if(_invert[i][m]>value)
            {
                ending=m-1;
                m=(start+ending)/2;
            }
            else if(_invert[i][m]<value)
            {
                start=m+1;
                m=(start+ending)/2;
            }
            else
            {
                times+=1;
                break;
            }
        }
    }
    return times;
}

int **InvertHeap::getInverted()
{
    return _invert;
}



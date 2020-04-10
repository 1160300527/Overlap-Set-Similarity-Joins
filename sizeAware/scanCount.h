#ifndef SCANCOUNT_H_INCLUDED
#define SCANCOUNT_H_INCLUDED

long long  ScanCountJoin(int** rel, int threshold);
int ** getInvertedIndex(int **rel);
void   ScanCountFreeInverted(int ** iIndex);
#endif // SCANCOUNT_H_INCLUDED

#include <iostream>
#include "Python.h"
#include "transferPy.h"
using namespace std;
int transferPython(vector<double>informations)
{
    int res = 0;
    Py_Initialize(); /*��ʼ��python������,���߱�����Ҫ�õ�python������*/
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('Dlls/')");

    PyObject* pModule = PyImport_ImportModule("loadModule");

    if (!pModule)
    {
        cout << "Python get module failed." << endl;
        return 0;
    }
    PyObject* pFunc = PyObject_GetAttrString(pModule,"predict");
    PyObject* pArgs = NULL;
    PyObject* pList = PyList_New(0);
    pArgs = PyTuple_New(1);
    cout<<informations.size()<<endl;
    while(!informations.empty())
    {
        PyList_Append(pList,Py_BuildValue("d",informations.back()));
        informations.pop_back();
    }
    PyTuple_SetItem(pArgs,0,pList);
    PyObject* pRet = PyEval_CallObject(pFunc,pArgs);

    PyArg_Parse(pRet,"i",&res);

    Py_Finalize(); /*����python���������ͷ���Դ*/

    return res;
}

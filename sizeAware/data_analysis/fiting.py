import matplotlib.pyplot as plt
import numpy as np
import math
import xlrd
import sys
from numpy import matrix

max_iters = 50000  # 梯度下级最大迭代次数
max_value = 0.0000001  # 停止迭代条件
step = 0.01  # 梯度下级步长
L = -8      #lambda的指数
l = math.e**L   #lambda

#解析解方法求解无正则项情况
def bf(X, Y):       
    X_T = X.transpose()
    XTX = np.dot(X_T, X)
    oppo = np.linalg.inv(XTX)
    XTY = np.dot(X_T, Y)
    return np.dot(oppo, XTY)


#解析解方法求解有正则项情况
def bf_reg(X, Y):   
    m=X.shape[1]-1
    X_T = X.transpose()
    XTX = np.dot(X_T, X)
    oppo = np.linalg.inv(XTX+(np.eye(m+1))*l)
    XTY = np.dot(X_T, Y)
    return np.dot(oppo, XTY)


#梯度下降求解无正则项情况
def gradient(X, Y): 
    m=X.shape[1]
    W = np.matrix(np.linspace(0, 0, m)).T
    iter = 0
    g = gra(X, Y, W)
    while((iter < max_iters) & (np.sqrt(np.dot(g.transpose(), g)) >= max_value)):
        W = W - step*g
        g = gra(X, Y, W)
        iter += 1
    return W


#梯度下降求解有正则项情况
def gradient_reg(X, Y): 
    m=X.shape[1]
    W = np.matrix(np.linspace(0, 0, m)).T
    iter = 0
    g = gra_reg(X, Y, W)
    while((iter < max_iters) & (np.sqrt(np.dot(g.transpose(), g)) >= max_value)):
        W = W - step*g
        g = gra_reg(X, Y, W)
        iter += 1
    return W


#共轭梯度求解无正则项情况
def conjugate(X, Y):    
    m=X.shape[1]
    XTY = np.dot(X.transpose(), Y)
    XTX = np.dot(X.transpose(), X)
    W = np.matrix(np.linspace(0, 0, m)).T
    R = XTY - np.dot(XTX, W)
    P = R
    k = 0
    while ((k <= m) & (np.sqrt(R.transpose().dot(R)) >= max_value)):
        A = (np.dot(R.transpose(), R)/P.transpose().dot(np.dot(XTX,P)))[0,0]
        W = W + A*P
        R2 = R - A*(np.dot(XTX,P))
        B = (R2.transpose().dot(R2)/R.transpose().dot(R))[0,0]
        P = R2+B*P
        R = R2
        k += 1
    return W


#共轭梯度求解有正则项情况
def conjugate_reg(X, Y):   
    m=X.shape[1] 
    XTY = np.dot(X.transpose(), Y)
    XTX = np.dot(X.transpose(), X) + l*np.eye(m)
    W = np.matrix(np.linspace(0, 0, m)).T
    R = XTY - np.dot(XTX, W)
    P = R
    k = 0
    while ((k <= m) & (np.sqrt(R.transpose().dot(R)) >= max_value)):
        A = (np.dot(R.transpose(), R)/P.transpose().dot(XTX.dot(P)))[0,0]
        W = W + A*P
        R2 = R - A*(XTX.dot(P))
        B = (R2.transpose().dot(R2)/R.transpose().dot(R))[0,0]
        P = R2+B*P
        R = R2
        k += 1
    return W


#根据X,Y,W求解无正则项情况下F(W)梯度
def gra(X, Y, W):      
    XT = X.transpose()
    return XT.dot(X.dot(W)-Y)


#根据X,Y,W求解有正则项情况下F(W)梯度
def gra_reg(X, Y, W):   
    XT = X.transpose()
    return np.dot(np.dot(XT, X), W)-np.dot(XT, Y)+l*W


#拟合优度评价
def ERMS(loss, num):            
    return np.sqrt(2*loss/num)


#计算拟合曲线上x的对应值
def answer(X, W):               
    return np.dot(X, W)


def loadSizeAwareData(argv,sheet_name):
    filePath = argv[1]
    file = xlrd.open_workbook(filePath)
    sheet = file.sheet_by_name(sheet_name)
    rows = sheet.row_values(0)
    row = sheet.nrows
    col = sheet.ncols
    for i in range(1, row):
        rows = np.vstack((rows, sheet.row_values(i)))
    a = matrix(np.array(rows).astype(float))
    return a[:,:col-4],(a[:,col-3])

def loadAllPairsData(argv,sheet_name):
    filePath = argv[1]
    file = xlrd.open_workbook(filePath)
    sheet = file.sheet_by_name(sheet_name)
    rows = sheet.row_values(0)
    row = sheet.nrows
    col = sheet.ncols
    for i in range(1, row):
        rows = np.vstack((rows, sheet.row_values(i)))
    a = matrix(np.array(rows).astype(float))
    return a[:,:col-4],a[:,col-2]

def loadMulData(argv,sheet_name):
    filePath = argv[1]
    file = xlrd.open_workbook(filePath)
    sheet = file.sheet_by_name(sheet_name)
    rows = sheet.row_values(0)
    row = sheet.nrows
    col = sheet.ncols
    for i in range(1, row):
        rows = np.vstack((rows, sheet.row_values(i)))
    a = matrix(np.array(rows).astype(float))
    return a[:,:col-4],a[:,col-1]


if __name__== '__main__':
    (X_train_size,Y_train_size)=loadSizeAwareData(sys.argv,"train")
    (X_train_all,Y_train_all)=loadAllPairsData(sys.argv,"train")
    (X_train_mul,Y_train_mul)=loadMulData(sys.argv,"train")
    (X_test_size,Y_test_size)=loadSizeAwareData(sys.argv,"test")
    (X_test_all,Y_test_all)=loadAllPairsData(sys.argv,"test")
    (X_test_mul,Y_test_mul)=loadMulData(sys.argv,"test")
    W_mul = conjugate_reg(X_train_mul,Y_train_mul)
    W_size = conjugate_reg(X_train_size,Y_train_size)
    W_all = conjugate_reg(X_train_all,Y_train_all)
    Y_size = np.dot(X_test_size,W_size)
    Y_all = np.dot(X_test_all,W_all)
    Y_mul = np.dot(X_test_mul,W_mul)
    R_test = map(lambda x: 1 if x>1 else 0,Y_test_mul)
    print(list(R_test))
import matplotlib.pyplot as plt
import numpy as np
import math
import sys
import xlrd
from numpy import matrix


step = 0.01  # 梯度下降步长
max_iter = 20000  # 梯度下降最大迭代次数
stop_value = 0.000001  # 梯度下降退出条件
L = -math.e**(-10)  # 正则项系数


# 当X维度为2时，用于画出分类图像
def draw(X0, X1, Y, W, choose):  # 画函数图像
    X = np.vstack((X0, X1))
    R = recall(X,Y,W)
    P = precision(X,Y,W)
    F1 = F1score(R,P)
    plt.title("Logistic function for test"+choose+"\nAccuracy="+str("%.2f" % accuracy(X, Y, W))+"    Precision=" +
              str("%.2f" % P)+"    Recall="+str("%.2f" % R)+"    F1 score="+str("%.2f"%F1))
    plt.scatter(X0.transpose().tolist()[
                1], X0.transpose().tolist()[2], label="Y = 0")
    plt.scatter(X1.transpose().tolist()[
                1], X1.transpose().tolist()[2], label="Y = 1")
    W = W.tolist()[0]
    plt.plot(X, -(W[0]+W[1]*X)/W[2])
    plt.legend()


# 牛顿法
def newton(X, Y):
    W = np.mat(np.linspace(0, 0, X.shape[1]))  # 1*(m+1)
    iter = 0
    while((iter < 100)):
        try:
            H_I = Hessian(X, W).I
        except:
            return W
        change = H_I.dot(gra(X, Y, W).T).T
        W = W + change
        if (change.dot(change.T) < stop_value):
            break
        iter += 1
    return W


# 有正则项的牛顿法
def newton_reg(X, Y):
    W = np.mat(np.linspace(0, 0, X.shape[1]))  # 1*(m+1)
    iter = 0
    while((iter < 100)):
        try:
            H_I = (Hessian(X, W)-L*np.ones((X.shape[1],X.shape[1]))).I
        except:
            return W
        change = H_I.dot((gra(X, Y, W)-L*W).T).T
        W = W + change
        if (change.dot(change.T) < stop_value):
            break
        iter += 1
    return W


# 牛顿法计算W并画出图像
def Newton(X, Y, X0_Test, X1_Test, Y_Test):
    W = newton(X, Y)
    W_reg = newton(X, Y)
    plt.subplot(121)
    draw(X0_Test, X1_Test, Y_Test, W, "by Newton method without regular")
    plt.subplot(122)
    draw(X0_Test, X1_Test, Y_Test, W_reg, "by Neton method with regular")
    plt.show()


# 求Hessian矩阵  X:n*(m+1)   W:1*(m+1)
def Hessian(X, W):
    temp = possibility(X, W)  # 1*n
    sum = np.multiply(temp, (1-temp))  # 1*n
    A = np.multiply(sum, np.eye(X.shape[0]))
    return X.T.dot(A).dot(X)  # (m+1)*(m+1)


# 梯度下降法
def gradient(X, Y):
    W = np.mat(np.linspace(0, 0, X.shape[1]))  # 1*(m+1)
    iter = 0
    while((iter < max_iter)):
        g = gra(X, Y, W)
        W = W + step*g
        if (g.dot(g.T) < stop_value):
            break
        iter += 1
    return W


# 带正则的梯度下降法
def gradient_reg(X, Y):
    W = np.mat(np.linspace(0, 0, X.shape[1]))  # 1*(m+1)
    iter = 0
    while((iter < max_iter)):
        g = gra(X, Y, W)
        W = W + step*g - L*W
        if (g.dot(g.T) < stop_value):
            break
        iter += 1
    return W


# 求梯度大小
def gra(X, Y, W):
    P = Y-possibility(X, W)  # 1*num
    update = X.T.dot(P.T)  # (m+1)*1
    return update.T


# X:num*(m+1)  W:1*(m+1) return:1*num
def possibility(X, W):
    Z = X.dot(W.T)  # num*1
    sum = Z.T
    temp = np.exp(sum)
    p = 1-1 / (1+temp)
    return p


# 梯度上升方法计算W值并画出图像
def Gradient(X, Y, X0_Test, X1_Test, Y_Test):
    W = gradient(X, Y)
    W_reg = gradient_reg(X, Y)
    plt.subplot(121)
    draw(X0_Test, X1_Test, Y_Test, W, "by gradient without regular")
    plt.subplot(122)
    draw(X0_Test, X1_Test, Y_Test, W_reg, "by gradient with regular")
    plt.show()


# 计算准确度
def accuracy(X, Y, W):
    sum = 0
    Y = Y.tolist()[0]
    for i in range(X.shape[0]):
        t = W.dot(X[i].T)
        if ((t > 0) & (Y[i] == 1)) | ((t < 0) & (Y[i] == 0)):
            sum += 1
    return sum/len(Y)


# 计算precision
def precision(X, Y, W):
    all_one = 0
    TP = 0
    Y = Y.tolist()[0]
    for i in range(X.shape[0]):
        t = W.dot(X[i].T)
        if(t > 0):
            all_one += 1
            if (Y[i] == 1):
                TP += 1
    return TP/all_one


# 计算recall值
def recall(X, Y, W):
    real_one = 0
    TP = 0
    Y = Y.tolist()[0]
    for i in range(X.shape[0]):
        t = W.dot(X[i].T)
        if(Y[i] == 1):
            real_one += 1
            if t > 0:
                TP += 1
    return TP/real_one


#计算F1score
def F1score(R,P):
    return 2*P*R/(P+R)

#计算特异度
def TNR(X,Y,W):
    real_one = 0
    TP = 0
    Y = Y.tolist()[0]
    for i in range(X.shape[0]):
        t = W.dot(X[i].T)
        if(Y[i] == 0):
            real_one += 1
            if t <= 0:
                TP += 1
    return TP/real_one


# 打印输出Accuracy,precision,recall
def print_data(X, Y, W):
    R = recall(X,Y,W)
    P = recall(X,Y,W)
    F1 = F1score(R,P)
    T = TNR(X,Y,W)
    print("accuracy:"+str("%.2f" % accuracy(X, Y, W)))  # 计算accuracy并输出
    print("precision:"+str("%.2f" % P))  # 计算precision并输出
    print("recall:"+str("%.2f" % R))  # 计算recall并输出
    print("TNR:"+str("%.2f"%T))
    print("F1 score:"+str("%2f"%F1))

def loadData(argv,sheet_name):
    filePath = argv[1]
    file = xlrd.open_workbook(filePath)
    sheet = file.sheet_by_name(sheet_name)
    rows = sheet.row_values(0)
    row = sheet.nrows
    col = sheet.ncols
    for i in range(1, row):
        rows = np.vstack((rows, sheet.row_values(i)))
    a = matrix(np.array(rows).astype(float))
    return a[:,:col-4],a[:,col-4].T

def trainSizeAware(argv):
# 从训练集文件获取训练样本
    (X,Y)=loadData(argv,"train")
    # 从测试集文件获取测试样本
    (X_test,Y_test) = loadData(argv,"test")
    # 计算通过不同优化方法得到的W
    print(type(X))
    W = gradient(X, Y)
    W_reg = gradient_reg(X, Y)
    W_Newton = newton(X, Y)
    W_Newton_Reg = newton_reg(X, Y)
    print("The data from UCI:00267")
    print("By Gradient Ascent Without Regular: ")
    print_data(X_test, Y_test, W)
    print("By Gradient Ascent With Regular:")
    print_data(X_test, Y_test, W_reg)
    print("By Newton Wethod Without Regular:")
    print_data(X_test, Y_test, W_Newton)
    print("By Newton Method With Regular:")
    print_data(X_test, Y_test, W_Newton_Reg)


# 对统计的数据进行训练和测试
if __name__ == '__main__':
    #data = getData(sys.argv)
    #content(data)
    #getTrainAndTest(sys.argv, data)
    trainSizeAware(sys.argv)

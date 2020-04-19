import matplotlib.pyplot as plt
import numpy as np
import math
import xlrd
import sys
from numpy import matrix
from sklearn.svm import SVC
from sklearn.linear_model import LogisticRegression
from sklearn.linear_model import LogisticRegressionCV
from sklearn.linear_model import RidgeCV
from sklearn.preprocessing import PolynomialFeatures
from sklearn.tree import DecisionTreeClassifier
from sklearn.tree import export_graphviz
import pydotplus
import os

all_type = 4
types = 4
os.environ['PATH']+=os.pathsep+'D:/graphviz/bin'

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
    return a[:,col-4]

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
    return a[:,col-3]

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
    return a[:,:col-5],a[:,col-2]

def getAllCost(Y,Cost):
    all_size = 0
    min_time = 0
    for i in range(Y.shape[0]):
        min_time+=Cost[i,Y[i,0]-1]
        all_size+=Cost[i,0]
    print("all time use sizeAware:"+str("%.2f"%all_size))
    print("min cost:"+str("%.2f"%min_time))

def getCost(Y_pre,Cost):
    sum=0
    for i in range(len(Y_pre)):
        sum+=Cost[i,Y_pre[i]-1]
    return sum

def svm_svc(X,Y,X_test,Y_test,weight,Cost):
    svc = SVC(kernel='rbf',class_weight='balanced')
    svc.fit(X,Y,sample_weight=weight)
    #svc.fit(X,Y)
    Y_pre=svc.predict(X_test)
    cost = getCost(Y_pre,Cost)
    print("########################################################svm_svc method######################################################")
    getAllCost(Y_test,cost_test)
    print("accuracy:"+str("%.2f" % svc.score(X_test,Y_test)))  # 计算accuracy并输出
    print("cost:"+str("%2f"%cost))
    print("############################################################################################################################")

def nonliner_svm_svc(X,Y,X_test,Y_test,weight,Cost):
    svc = SVC(kernel='rbf',class_weight='balanced')
    poly=PolynomialFeatures(degree=2)
    X=poly.fit_transform(X)
    X_test=poly.fit_transform(X_test)
    svc.fit(X,Y,sample_weight=weight)
    #svc.fit(X,Y)
    Y_pre=svc.predict(X_test)
    cost = getCost(Y_pre,Cost)
    print("########################################################non liner svm_svc method######################################################")
    getAllCost(Y_test,cost_test)
    print("accuracy:"+str("%.2f" % svc.score(X_test,Y_test)))  # 计算accuracy并输出
    print("cost:"+str("%2f"%cost))
    print("#####################################################################################################################################")

def getWeight(Cost):
    weight = []
    for i in range(Cost.shape[0]):
        max=0
        min = sys.maxsize
        for j in range(Cost.shape[1]):
            if(Cost[i,j]>max):
                max=Cost[i,j]
            if(Cost[i,j]<min):
                min=Cost[i,j]
        weight.append(abs(math.log(max-min)))
    return weight


def getY(Cost):
    Y = []
    for i in range(Cost.shape[0]):
        min = sys.maxsize
        index = 0
        for j in range(1,Cost.shape[1]):
            if(Cost[i,j]<min):
                min=Cost[i,j]
                index = j+1
        if(Cost[i,index-1]>=Cost[i,0]*1.2 or Cost[i,index-1]-Cost[i,0]>=1):
            index=1
        Y.append(index)
    return np.matrix(Y).T


def logistic(X,Y,X_test,Y_test,weight,Cost):
    logist = LogisticRegression(penalty='l2',solver='newton-cg')
    #logist_liblinear = LogisticRegression(penalty='l1',solver='liblinear')
    #logist_lbfgs = LogisticRegression(penalty='l2',solver='lbfgs')
    #logist_cv = LogisticRegressionCV(cv=10,penalty='l2',solver='newton-cg')
    #logist.fit(X,Y,sample_weight=weight)
    logist.fit(X,Y)
    Y_pre=logist.predict(X_test)
    cost = getCost(Y_pre,Cost)
    print("######################################################logistic regression###################################################")
    getAllCost(Y_test,cost_test)
    print("accuracy:"+str("%.2f" % logist.score(X_test,Y_test)))  # 计算accuracy并输出
    print("cost:"+str("%2f"%cost))
    print("############################################################################################################################")

def nonliner_logistic(X,Y,X_test,Y_test,weight):
    logist = LogisticRegression(penalty='l2',solver='newton-cg')
    #logist_liblinear = LogisticRegression(penalty='l1',solver='liblinear')
    #logist_lbfgs = LogisticRegression(penalty='l2',solver='lbfgs')
    #logist_cv = LogisticRegressionCV(cv=10,penalty='l2',solver='newton-cg')
    #logist.fit(X,Y,sample_weight=weight)
    max=0
    max_poly=0
    for i in range(2,4):
        poly=PolynomialFeatures(degree=i)
        X_poly=poly.fit_transform(X)
        X_test_poly=poly.fit_transform(X_test)
        logist.fit(X_poly,Y)
        accuracy = logist.score(X_test_poly,Y_test)
        Y_pre=logist.predict(X_test_poly)
        if accuracy>max:
            max=accuracy
            max_poly=i
        print("###############################################non liner logistc regression###############################################")
        getAllCost(Y_test,cost_test)
        print("poly:"+str("%d"%i))
        print("accuracy:"+str("%.2f" % accuracy))  # 计算accuracy并输出
        print("##########################################################################################################################")
    print("max poly:%d"%max_poly)

def fitting(X_train_size,Y_train_size,X_train_all,Y_train_all,X_test_size,X_test_all,Y_test):
    ridge_cv_size = RidgeCV(alphas=[1e-1,1e-2,1e-3,1e-4,1e-5,1e-6])
    ridge_cv_all = RidgeCV(alphas=[1e-1,1e-2,1e-3,1e-4,1e-5,1e-6])

    ridge_cv_size.fit(X_train_size,Y_train_size)
    ridge_cv_all.fit(X_train_all,Y_train_all)

    Y_pre_size = ridge_cv_size.predict(X_test_size)
    Y_pre_all  = ridge_cv_all.predict(X_test_all)

    
    Y = list(map(lambda x,y: 1 if x>1 else 0,Y_pre_all,Y_pre_size))
    for i in range(len(Y)):
        if ((Y_pre_all[i]-Y_pre_size[i])<2&(Y_pre_all[i]/Y_pre_size[i]<1.5)):
            Y[i]=0
    
    # R = recall(Y,Y_test)
    # P = recall(Y,Y_test)
    # F1 = F1score(R,P)
    # T = TNR(Y,Y_test)
    print("########################################fitting method##############################################")
    # print("accuracy:"+str("%.2f" % accuracy(Y,Y_test)))  # 计算accuracy并输出
    # print("precision:"+str("%.2f" % P))  # 计算precision并输出
    # print("recall:"+str("%.2f" % R))  # 计算recall并输出
    # print("TNR:"+str("%.2f"%T))
    # print("F1 score:"+str("%2f"%F1))
    print("####################################################################################################")

def nonliner_fitting(X_train_size,Y_train_size,X_train_all,Y_train_all,X_test_size,X_test_all,Y_test):
    ridge_cv_size = RidgeCV(alphas=[1e-1,1e-2,1e-3,1e-4,1e-5,1e-6])
    ridge_cv_all = RidgeCV(alphas=[1e-1,1e-2,1e-3,1e-4,1e-5,1e-6])

    poly=PolynomialFeatures(degree=2)
    X_train_all=poly.fit_transform(X_train_all)
    X_test_all=poly.fit_transform(X_test_all)
    X_train_size=poly.fit_transform(X_train_size)
    X_test_size=poly.fit_transform(X_test_size)

    ridge_cv_size.fit(X_train_size,Y_train_size)
    ridge_cv_all.fit(X_train_all,Y_train_all)

    Y_pre_size = ridge_cv_size.predict(X_test_size)
    Y_pre_all  = ridge_cv_all.predict(X_test_all)

    
    Y = list(map(lambda x,y: 1 if x>1 else 0,Y_pre_all,Y_pre_size))
    for i in range(len(Y)):
        if ((Y_pre_all[i]-Y_pre_size[i])<2&(Y_pre_all[i]/Y_pre_size[i]<1.5)):
            Y[i]=0
    
    # R = recall(Y,Y_test)
    # P = recall(Y,Y_test)
    # F1 = F1score(R,P)
    # T = TNR(Y,Y_test)
    print("########################################non liner fitting method##############################################")
    # print("accuracy:"+str("%.2f" % accuracy(Y,Y_test)))  # 计算accuracy并输出
    # print("precision:"+str("%.2f" % P))  # 计算precision并输出
    # print("recall:"+str("%.2f" % R))  # 计算recall并输出
    # print("TNR:"+str("%.2f"%T))
    # print("F1 score:"+str("%2f"%F1))
    print("####################################################################################################")
    

def tree(X,Y,X_test,Y_test,weight,Cost):
    decision_tree=DecisionTreeClassifier()
    decision_tree.fit(X,Y)
    accuracy=decision_tree.score(X_test,Y_test)
    Y_pre = decision_tree.predict(X_test)
    cost = getCost(Y_pre,Cost)
    # R = recall(Y_pre,Y_test)
    # P = recall(Y_pre,Y_test)
    # F1 = F1score(R,P)
    # T = TNR(Y_pre,Y_test)
    print("########################################decision tree##############################################")
    getAllCost(Y_test,cost_test)
    print("accuracy:"+str("%.2f" % accuracy))  # 计算accuracy并输出
    print("cost:"+str("%2f"%cost))
    print("####################################################################################################")
    feature_name=['size','avgFre','devFre','bias','peak','avgLowFre','devLowFre','lFrePos','minMiddleFre','maxMiddleFre','avgMiddleFre','devMiddleFre','hFrePos','minHighFre','maxHighFre','avgHighFre','devHighFre','setSize','avgLength','devLength','C']
    dot_data = export_graphviz(decision_tree, out_file=None,
                        filled=True,rounded=True,feature_names=feature_name,
                        impurity=False)
    graph = pydotplus.graph_from_dot_data(dot_data)
    graph.write_pdf("iris.pdf")



def getData(argv,sheet_name):
    filePath = argv[1]
    file = xlrd.open_workbook(filePath)
    sheet = file.sheet_by_name(sheet_name)
    rows = sheet.row_values(0)
    row = sheet.nrows
    for i in range(1, row):
        rows = np.vstack((rows, sheet.row_values(i)))
    a = matrix(np.array(rows).astype(float))
    a = a[:,:sheet.ncols]
    return a

if __name__== '__main__':
    if(len(sys.argv)>2):
        types = int(sys.argv[2])
    data_train = getData(sys.argv,"train")
    row_train = data_train.shape[0]
    col_train = data_train.shape[1]
    cost_train = data_train[:,col_train-all_type:col_train-all_type+types]

    data_test = getData(sys.argv,"test")
    row_test  = data_test.shape[0]
    col_test  = data_test.shape[1]
    cost_test = data_test[:,col_test-all_type:col_test-all_type+types]

    # Y_train_size= data_train[:,col_train-types]
    # Y_train_all = data_train[:,col_train-types+1]
    # Y_train_scan = data_train[:,col_train-types+2]
    # Y_train_divide = data_train[:,col_train-types+3]

    # Y_test_size = data_test[:,col_train-types]
    # Y_test_all  =data_test[:,col_train-types+1]
    # Y_test_scan = data_test[:,col_test-types+2]
    # Y_test_divide = data_test[:,col_test-types+3]

    X_train = data_train[:,:col_train-all_type-1]

    Y_train = getY(cost_train)

    X_test = data_test[:,:col_test-all_type-1]

    Y_test = getY(cost_test)

    weight = getWeight(cost_train)
    svm_svc(X_train,Y_train,X_test,Y_test,weight,cost_test)
    print("\n")
    nonliner_svm_svc(X_train,Y_train,X_test,Y_test,weight,cost_test)
    print("\n")
    logistic(X_train,Y_train,X_test,Y_test,weight,cost_test)
    print("\n")
    #nonliner_logistic(X_train,Y_train,X_test,Y_test,weight)
    #nonliner_logistic(X_train_real,Y_train_real,X_test_real,Y_test_real,weight)
    #fitting(X_train_size,Y_train_size,X_train_all,Y_train_all,X_test_size,X_test_all,Y_test)
    #nonliner_fitting(X_train_size,Y_train_size,X_train_all,Y_train_all,X_test_size,X_test_all,Y_test)
    tree(X_train,Y_train,X_test,Y_test,weight,cost_test)

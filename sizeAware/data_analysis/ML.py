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

# 计算准确度
def accuracy(Y_pre, Y):
    sum = 0
    Y = Y.T.tolist()[0]
    for i in range(len(Y)):
        if (Y_pre[i]==Y[i]):
            sum += 1
    return sum/len(Y)


# 计算precision
def precision(Y_pre, Y):
    all_one = 0
    TP = 0
    Y = Y.T.tolist()[0]
    for i in range(len(Y)):
        if  (Y_pre[i] ==1):
            all_one += 1
            if (Y[i] == 1):
                TP += 1
    return TP/all_one


# 计算recall值
def recall(Y_pre, Y):
    real_one = 0
    TP = 0
    Y = Y.T.tolist()[0]
    for i in range(len(Y)):
        if(Y[i] == 1):
            real_one += 1
            if Y_pre[i]==1:
                TP += 1
    return TP/real_one


#计算F1score
def F1score(R,P):
    return 2*P*R/(P+R)

#计算特异度
def TNR(Y_pre,Y):
    real_one = 0
    TP = 0
    Y = Y.T.tolist()[0]
    for i in range(len(Y)):
        if(Y[i] == 0):
            real_one += 1
            if Y_pre[i] == 0:
                TP += 1
    return TP/real_one

def getCost(Y_pre,Y_test,distance):
    sum=0
    all_sum = 0
    for i in range(len(Y_pre)):
        if(Y_pre[i]!=Y_test[i][0]):
            sum+=distance[i]
            #print(distance[i],i)
        if(Y_test[i][0]==0):
            all_sum+=distance[i]
    print("all sum:"+str("%.2f"%all_sum))
    return sum

def svm_svc(X,Y,X_test,Y_test,weight,distance):
    svc = SVC(kernel='rbf',class_weight='balanced')
    svc.fit(X,Y,sample_weight=weight)
    #svc.fit(X,Y)
    Y_pre=svc.predict(X_test)
    R = recall(Y_pre,Y_test)
    P = recall(Y_pre,Y_test)
    F1 = F1score(R,P)
    T = TNR(Y_pre,Y_test)
    cost = getCost(Y_pre,Y_test,distance)
    print("########################################################svm_svc method######################################################")
    print("accuracy:"+str("%.2f" % svc.score(X_test,Y_test)))  # 计算accuracy并输出
    print("precision:"+str("%.2f" % P))  # 计算precision并输出
    print("recall:"+str("%.2f" % R))  # 计算recall并输出
    print("TNR:"+str("%.2f"%T))
    print("F1 score:"+str("%2f"%F1))
    print("cost:"+str("%2f"%cost))
    print("############################################################################################################################")

def nonliner_svm_svc(X,Y,X_test,Y_test,weight,distance):
    svc = SVC(kernel='rbf',class_weight='balanced')
    poly=PolynomialFeatures(degree=2)
    X=poly.fit_transform(X)
    X_test=poly.fit_transform(X_test)
    svc.fit(X,Y,sample_weight=weight)
    #svc.fit(X,Y)
    Y_pre=svc.predict(X_test)
    R = recall(Y_pre,Y_test)
    P = recall(Y_pre,Y_test)
    F1 = F1score(R,P)
    T = TNR(Y_pre,Y_test)
    cost = getCost(Y_pre,Y_test,distance)
    print("########################################################non liner svm_svc method######################################################")
    print("accuracy:"+str("%.2f" % svc.score(X_test,Y_test)))  # 计算accuracy并输出
    print("precision:"+str("%.2f" % P))  # 计算precision并输出
    print("recall:"+str("%.2f" % R))  # 计算recall并输出
    print("TNR:"+str("%.2f"%T))
    print("F1 score:"+str("%2f"%F1))
    print("cost:"+str("%2f"%cost))
    print("#####################################################################################################################################")

def getWeight(Y_size,Y_all):
    weight = []
    for i in range(Y_all.shape[0]):
        weight.append(abs(math.log(abs(Y_size[i,0]-Y_all[i,0]))))
    return weight

def getDistance(Y_size,Y_all):
    distance = []
    for i in range(Y_all.shape[0]):
        distance.append(abs(Y_size[i,0]-Y_all[i,0]))
    return distance

def getWrongCost(Y,Y_real,Y_size,Y_all):
    cost = 0
    for i in range(Y_all.shape[0]):
        if (Y[i,0]!=Y_real[i,0]):
            cost = cost+abs(Y_size[i,0]-Y_all[i,0])
    return cost

def logistic(X,Y,X_test,Y_test,weight):
    logist = LogisticRegression(penalty='l2',solver='newton-cg')
    #logist_liblinear = LogisticRegression(penalty='l1',solver='liblinear')
    #logist_lbfgs = LogisticRegression(penalty='l2',solver='lbfgs')
    #logist_cv = LogisticRegressionCV(cv=10,penalty='l2',solver='newton-cg')
    #logist.fit(X,Y,sample_weight=weight)
    logist.fit(X,Y)
    Y_pre=logist.predict(X_test)
    R = recall(Y_pre,Y_test)
    P = recall(Y_pre,Y_test)
    F1 = F1score(R,P)
    T = TNR(Y_pre,Y_test)
    print("######################################################logistic regression###################################################")
    print("accuracy:"+str("%.2f" % logist.score(X_test,Y_test)))  # 计算accuracy并输出
    print("precision:"+str("%.2f" % P))  # 计算precision并输出
    print("recall:"+str("%.2f" % R))  # 计算recall并输出
    print("TNR:"+str("%.2f"%T))
    print("F1 score:"+str("%2f"%F1))
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
        R = recall(Y_pre,Y_test)
        P = recall(Y_pre,Y_test)
        F1 = F1score(R,P)
        T = TNR(Y_pre,Y_test)
        if accuracy>max:
            max=accuracy
            max_poly=i
        print("###############################################non liner logistc regression###############################################")
        print("poly:"+str("%d"%i))
        print("accuracy:"+str("%.2f" % accuracy))  # 计算accuracy并输出
        print("precision:"+str("%.2f" % P))  # 计算precision并输出
        print("recall:"+str("%.2f" % R))  # 计算recall并输出
        print("TNR:"+str("%.2f"%T))
        print("F1 score:"+str("%2f"%F1))
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
    
    R = recall(Y,Y_test)
    P = recall(Y,Y_test)
    F1 = F1score(R,P)
    T = TNR(Y,Y_test)
    print("########################################fitting method##############################################")
    print("accuracy:"+str("%.2f" % accuracy(Y,Y_test)))  # 计算accuracy并输出
    print("precision:"+str("%.2f" % P))  # 计算precision并输出
    print("recall:"+str("%.2f" % R))  # 计算recall并输出
    print("TNR:"+str("%.2f"%T))
    print("F1 score:"+str("%2f"%F1))
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
    
    R = recall(Y,Y_test)
    P = recall(Y,Y_test)
    F1 = F1score(R,P)
    T = TNR(Y,Y_test)
    print("########################################non liner fitting method##############################################")
    print("accuracy:"+str("%.2f" % accuracy(Y,Y_test)))  # 计算accuracy并输出
    print("precision:"+str("%.2f" % P))  # 计算precision并输出
    print("recall:"+str("%.2f" % R))  # 计算recall并输出
    print("TNR:"+str("%.2f"%T))
    print("F1 score:"+str("%2f"%F1))
    print("####################################################################################################")
    

def tree(X,Y,X_test,Y_test,weight):
    decision_tree=DecisionTreeClassifier()
    decision_tree.fit(X,Y)
    accuracy=decision_tree.score(X_test,Y_test)
    Y_pre = decision_tree.predict(X_test)
    R = recall(Y_pre,Y_test)
    P = recall(Y_pre,Y_test)
    F1 = F1score(R,P)
    T = TNR(Y_pre,Y_test)
    print("########################################decision tree##############################################")
    print("accuracy:"+str("%.2f" % accuracy))  # 计算accuracy并输出
    print("precision:"+str("%.2f" % P))  # 计算precision并输出
    print("recall:"+str("%.2f" % R))  # 计算recall并输出
    print("TNR:"+str("%.2f"%T))
    print("F1 score:"+str("%2f"%F1))
    print("####################################################################################################")
    feature_name=['size','avgFre','devFre','bias','peak','avgLowFre','devLowFre','lFrePos','minMiddleFre','maxMiddleFre','avgMiddleFre','devMiddleFre','hFrePos','minHighFre','maxHighFre','avgHighFre','devHighFre','setSize','avgLength','devLength','C']
    dot_data = export_graphviz(decision_tree, out_file=None,
                        filled=True,rounded=True,feature_names=feature_name,
                        impurity=False)
    graph = pydotplus.graph_from_dot_data(dot_data)
    graph.write_pdf("iris.pdf")

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
    return a[:,:col-5],a[:,col-5].astype(int)

def loadDataReal(argv,sheet_name):
    filePath = argv[1]
    file = xlrd.open_workbook(filePath)
    sheet = file.sheet_by_name(sheet_name)
    rows = sheet.row_values(0)
    row = sheet.nrows
    col = sheet.ncols
    for i in range(1, row):
        rows = np.vstack((rows, sheet.row_values(i)))
    a = matrix(np.array(rows).astype(float))
    return a[:,:col-5],a[:,col-1].astype(int)


if __name__== '__main__':
    Y_train_size=loadSizeAwareData(sys.argv,"train")
    Y_train_all=loadAllPairsData(sys.argv,"train")
    (X_train_mul,Y_train_mul)=loadMulData(sys.argv,"train")
    (X_test_size,Y_test_size)=loadSizeAwareData(sys.argv,"test")
    (X_test_all,Y_test_all)=loadAllPairsData(sys.argv,"test")
    (X_test_mul,Y_test_mul)=loadMulData(sys.argv,"test")
    (X_train,Y_train)=loadData(sys.argv,"train")
    (X_test,Y_test)=loadData(sys.argv,"test")
    #加载经过处理后的结果（修改了一些极端情况）
    (X_train_real,Y_train_real)=loadDataReal(sys.argv,"train")
    (X_test_real,Y_test_real)=loadDataReal(sys.argv,"test")
    wrong_cost = getWrongCost(Y_test,Y_test_real,Y_test_size,Y_test_all)

    weight = getWeight(Y_train_size,Y_train_all)
    distance = getDistance(Y_test_size,Y_test_all)
    #svm_svc(X_train,Y_train,X_test,Y_test,weight,distance)
    print("\n")
    svm_svc(X_train_real,Y_train_real,X_test_real,Y_test_real,weight,distance)
    print("Wrong Cost:"+str("%.2f"%wrong_cost))
    nonliner_svm_svc(X_train_real,Y_train_real,X_test_real,Y_test_real,weight,distance)
    #logistic(X_train,Y_train,X_test,Y_test,weight)
    #nonliner_logistic(X_train,Y_train,X_test,Y_test,weight)
    logistic(X_train_real,Y_train_real,X_test_real,Y_test_real,weight)
    # nonliner_logistic(X_train_real,Y_train_real,X_test_real,Y_test_real,weight)
    # fitting(X_train_size,Y_train_size,X_train_all,Y_train_all,X_test_size,X_test_all,Y_test)
    # nonliner_fitting(X_train_size,Y_train_size,X_train_all,Y_train_all,X_test_size,X_test_all,Y_test)
    tree(X_train_real,Y_train_real,X_test_real,Y_test_real,weight)
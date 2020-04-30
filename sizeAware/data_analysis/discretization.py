import matplotlib.pyplot as plt
import numpy as np
import math
import xlrd
import sys
from numpy import matrix,zeros
from sklearn.svm import SVC
from sklearn.linear_model import LogisticRegression
from sklearn.linear_model import LogisticRegressionCV
from sklearn.linear_model import RidgeCV
from sklearn.preprocessing import PolynomialFeatures
from sklearn.tree import DecisionTreeClassifier
from sklearn.tree import export_graphviz
from scipy.stats import pearsonr
from sklearn.feature_selection import SelectKBest,chi2
import ML

all_type = 4
types = 4

def discretization(Y):
    newY = zeros(Y.shape)
    for i in range(Y.shape[0]):
        for j in range(Y.shape[1]):
            newY[i,j] = int(Y[i,j]/5)
    return newY

def fitting(X_train,Y_train,X_test):
    ridge_cv = RidgeCV(alphas=[1e-1,1e-2,1e-3,1e-4,1e-5,1e-6])

    ridge_cv.fit(X_train,Y_train)
    Y_pre = ridge_cv.predict(X_test)
    return Y_pre

def fittingCosts(X_train,X_test,cost_train):
    pre_cost = np.zeros((X_test.shape[0],types))
    Y_pre = []
    max = -100
    Max=-1
    second = -100
    for i in range(types):
        pre_cost[:,i]=fitting(X_train,cost_train[:,i],X_test)
    for i in range(pre_cost.shape[0]):
        for j in range(types):
            if(pre_cost[i,j]>max):
                max = pre_cost[i,j]
                Max = j
            elif(pre_cost[i,j]>second):
                second = pre_cost[i,j]
        if(max-second>1):
            Y_pre.append(Max+1)
        else:
            Y_pre.append(0)
    return Y_pre

def get_pre(Y_fitting,Y_type):
    print(Y_fitting)
    print(Y_type)
    return Y_fitting

def svm_svc(X,Y,X_test,Y_test,weight,Cost):
    svc = SVC(kernel='rbf',class_weight='balanced')
    svc.fit(X,Y,sample_weight=weight)
    #svc.fit(X,Y)
    Y_pre=svc.predict(X_test)
    return Y_pre

def feature(X,Y):
    kBest = SelectKBest(score_func=chi2,k=10)
    kBest.fit_transform(X,Y)
    select = kBest.get_support(indices=True)
    return select

if __name__=='__main__':
    if(len(sys.argv)>2):
        types = int(sys.argv[2])
    data_train = ML.getData(sys.argv,"train")
    row_train = data_train.shape[0]
    col_train = data_train.shape[1]
    cost_train = data_train[:,col_train-all_type:col_train-all_type+types]
    X_train = data_train[:,:col_train-all_type-1]
    Y_train = ML.getY(cost_train)
    dis_cost_train = discretization(cost_train)
    
    data_test = ML.getData(sys.argv,"test")
    row_test = data_test.shape[0]
    col_test = data_test.shape[1]
    cost_test = data_test[:,col_test-all_type:col_test-all_type+types]
    X_test = data_test[:,:col_test-all_type-1]
    Y_test = ML.getY(cost_test)

    weight = ML.getWeight(cost_train)

    Y_fitting = fittingCosts(X_train,X_test,dis_cost_train)
    #print(X_train)
    feature(X_train,Y_train)
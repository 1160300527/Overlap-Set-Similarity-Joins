import xlrd
import sys
import numpy as np
import random
import openpyxl
import os
from numpy import *
from xlutils.copy import copy
from sklearn.preprocessing import StandardScaler


def getData(filePath):
    file = xlrd.open_workbook(filePath)
    sheet = file.sheet_by_name("data")
    rows = sheet.row_values(1)
    row = sheet.nrows
    for i in range(2, row):
        rows = np.vstack((rows, sheet.row_values(i)))
    a = matrix(np.array(rows).astype(float))
    a = a[:, :sheet.ncols]
    return a


def saveData(output, data):
    if(os.path.exists(output)):
        os.remove(output)
    wb = openpyxl.Workbook()
    data_sheet = wb.create_sheet(title="data")
    wb.remove(wb["Sheet"])
    rows = data.shape[0]
    cols = data.shape[1]
    for i in range(rows):
        for j in range(cols):
            data_sheet.cell(row=i+1, column=j+1).value = data[i, j]
    wb.save(output)


def normalize(data):
    row = data.shape[0]
    col = data.shape[1]
    # avg = data.sum(axis=0)/row
    # avg_matrix = np.tile(avg, (row, 1))
    # dis = data.max(axis=0)-data.min(axis=0)
    # Data = data-avg_matrix
    # mul = (avg_matrix-data).T.dot(avg_matrix-data)
    # stand = np.sqrt(np.diag(mul)/(row-1))
    # for i in range(row):
    #     for j in range(col-4):
    #         Data[i, j] = Data[i, j]/dis[0,j]
    scaler = StandardScaler()
    x_std = scaler.fit_transform(data)
    for i in range(row):
        for j in range(col-5, col):
            x_std[i, j] = data[i, j]
    return x_std


if __name__ == '__main__':
    data = getData(sys.argv[1])
    Data = normalize(data)
    saveData(sys.argv[2], Data)

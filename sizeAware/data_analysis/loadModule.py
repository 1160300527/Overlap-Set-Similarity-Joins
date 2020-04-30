import joblib
import numpy as np
import sys


def predict(data):
    model = joblib.load("decision_tree.m")
    return (model.predict(np.matrix(data))[0])


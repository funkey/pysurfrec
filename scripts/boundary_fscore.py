#!/usr/bin/python

import numpy as np
import scipy.ndimage

def fscore(rec, gt, tolerance):

    if tolerance > 0:
        rec_g = scipy.ndimage.morphology.binary_dilation(rec, iterations=tolerance).astype(rec.dtype)
        gt_g = scipy.ndimage.morphology.binary_dilation(gt, iterations=tolerance).astype(gt.dtype)
    else:
        rec_g = rec
        gt_g = gt

    # every fg voxel in rec that is bg in gt_g is FP
    fps = rec - gt_g
    fps[fps<=0] = 0
    #print "fps: " + str(fps)
    fp = fps.sum()

    # remaining fg voxels are TP
    tp = rec.sum() - fp

    # every fg voxel in gt that is bg in rec_g is FN
    fns = gt - rec_g
    fns[fns<=0] = 0
    fn = fns.sum()

    return (fp, fn, 2*tp/(2*tp + fn + fp))

if __name__ == "__main__":

    # test

    rec = np.zeros((5,5))
    rec[2,2] = 1
    print rec

    gt = np.zeros((5,5))
    gt[2,2] = 1
    print gt

    print fscore(rec, gt, 1)
    print
    print

    rec = np.zeros((5,5))
    rec[2,2] = 1
    print rec

    gt = np.zeros((5,5))
    gt[2,3] = 1
    print gt

    print fscore(rec, gt, 0)
    print
    print

    rec = np.zeros((5,5))
    rec[2,2] = 1
    print rec

    gt = np.zeros((5,5))
    gt[2,3] = 1
    print gt

    print fscore(rec, gt, 2)
    print
    print

    rec = np.zeros((5,5))
    rec[2,2] = 1
    print rec

    gt = np.zeros((5,5))
    gt[0,0] = 1
    gt[4,4] = 1
    print gt

    print fscore(rec, gt, 2)
    print
    print

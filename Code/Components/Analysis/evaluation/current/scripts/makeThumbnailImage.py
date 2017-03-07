#!/usr/bin/env python

import askap.analysis.evaluation

import matplotlib
matplotlib.use('Agg')
matplotlib.rcParams['font.family'] = 'serif'
matplotlib.rcParams['font.serif'] = ['Times', 'Palatino', 'New Century Schoolbook', 'Bookman', 'Computer Modern Roman']
#matplotlib.rcParams['text.usetex'] = True
import aplpy
import pylab as plt
from astropy.io import fits
import os
import numpy as np

from optparse import OptionParser
import askap.parset as parset
import askap.logging as logging

#############

if __name__ == '__main__':

    parser = OptionParser()
    parser.add_option("-c","--config", dest="inputfile", default="", help="Input parameter file [default: %default]")

    (options, args) = parser.parse_args()

    if(options.inputfile==''):
        inputPars = parset.ParameterSet()        
    elif(not os.path.exists(options.inputfile)):
        logging.warning("Config file %s does not exist!  Using default parameter values."%options.inputfile)
        inputPars = parset.ParameterSet()
    else:
        inputPars = parset.ParameterSet(options.inputfile).makeThumbnail

    fitsim=inputPars.get_value('image','')
    zmin=inputPars.get_value('zmin',-10.)
    zmax=inputPars.get_value('zmax',40.)
    suffix=inputPars.get_value('imageSuffix','png')
    figtitle=inputPars.get_value('imageTitle','')
    figsizes=inputPars.get_value('imageSizes',[16])
    figsizenames = inputPars.get_value('imageSizeNames',['large'])

    if len(figsizes) != len(figsizenames):
        raise IOError("figsizes and figsizenames must be the same length")
    
    if fitsim=='':
        raise IOError("No image defined")

    # Get statistics for the image
    #  If there is a matching weights image, use that to
    #  avoid pixels that have zero weight.
    weightsim=fitsim
    weightsim.replace('.restored','')
    prefix=weightsim[:weightsim.find('.')]
    weightsim.replace(prefix,'weights',1)
    image=fits.getdata(fitsim)
    isgood=(np.ones(image.shape)>0)
    if os.access(weightsim,os.F_OK):
        weights=fits.getdata(weightsim)
        isgood=(weights>0)
    median=np.median(image[isgood])
    madfm=np.median(abs(image[isgood]-median))
    stddev=madfm * 0.6744888
    vmin=zmin * stddev
    vmax=zmax * stddev

    thumbim=fitsim.replace('.fits','.%s'%suffix)
        
    for i in range(len(figsizes)):
        filename=thumbim.replace('.%s'%suffix,'_%s.%s'%(figsizenames[i],suffix))
        print("Writing to file %s"%filename)
        gc=aplpy.FITSFigure(fitsim,figsize=(figsizes[i],figsizes[i]))
        gc.show_colorscale(vmin=vmin,vmax=vmax)
        gc.tick_labels.set_xformat('hh:mm')
        gc.tick_labels.set_yformat('dd:mm')
        gc.add_grid()
        gc.grid.set_linewidth(0.5)
        gc.grid.set_alpha(0.5)
        plt.title(figtitle)
        #
        gc.set_theme('publication')
        gc.save(filename)


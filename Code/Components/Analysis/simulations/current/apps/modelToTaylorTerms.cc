///
/// @file : Create Taylor term images from a cube
///
/// Control parameters are passed in from a LOFAR ParameterSet file.
///
/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Matthew Whiting <matthew.whiting@csiro.au>
#include <askap_simulations.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <casa/Logging/LogIO.h>
#include <askap/Log4cxxLogSink.h>
#include <askapparallel/AskapParallel.h>

#include <analysisutilities/CasaImageUtil.h>

#include <Common/ParameterSet.h>
#include <casa/OS/Timer.h>
#include <casa/namespace.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/Unit.h>
#include <coordinates/Coordinates/Coordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <images/Images/PagedImage.h>
#include <images/Images/ImageInfo.h>
#include <images/Images/ImageOpener.h>
#include <images/Images/FITSImage.h>
#include <images/Images/MIRIADImage.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Array.h>
#include <gsl/gsl_multifit.h>
#include <wcslib/wcs.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <time.h>

using namespace askap;

ASKAP_LOGGER(logger, "modelToTaylorTerms.log");

// Move to Askap Util?
std::string getInputs(const std::string& key, const std::string& def, int argc,
                      const char** argv)
{
  if (argc > 2) {
    for (int arg = 0; arg < (argc - 1); arg++) {
      std::string argument = std::string(argv[arg]);

      if (argument == key) {
	return std::string(argv[arg+1]);
      }
    }
  }

  return def;
}

int main(int argc, const char **argv)
{

  askap::askapparallel::AskapParallel comms(argc, argv);

  try 
    {
      if(comms.isParallel() && comms.isMaster()){
	ASKAPLOG_INFO_STR(logger, "On master, so not doing anything");
      }
      else{

	std::string parsetFile(getInputs("-inputs", "modelToTaylorTerms.in", argc, argv));
	ASKAPLOG_INFO_STR(logger,  "parset file " << parsetFile);
	LOFAR::ParameterSet parset(parsetFile);
	ASKAPLOG_INFO_STR(logger, "Full file follows:\n"<<parset);
	LOFAR::ParameterSet subset(parset.makeSubset("model2TT."));
	ASKAPLOG_INFO_STR(logger, "Subset follows:\n"<<subset);

	std::string modelimage=subset.getString("inputmodel","");
	int nsubx=subset.getInt16("nsubx",1);
	int nsuby=subset.getInt16("nsuby",1);
	const int nterms=3;
	int logevery = subset.getInt16("logevery",10);
	ASKAPLOG_INFO_STR(logger, "Will log every "<<logevery << "% of the time");

	casa::PagedImage<Float> img(modelimage);
	IPosition shape = img.shape();
	int specCoord = img.coordinates().findCoordinate(Coordinate::SPECTRAL);
	int specAxis = img.coordinates().worldAxes(specCoord)[0];
	ASKAPLOG_DEBUG_STR(logger, "Model image " << modelimage << " has shape " << shape << " and the spectral axis is #"<<specAxis );
	
	int nx=0,ny=0,xmin,xmax,ymin,ymax;
	std::stringstream outputnamebase;
	if(comms.isParallel()){
	  nx = (comms.rank()-1) % nsubx;
	  ny = (comms.rank()-1) / nsubx;
	  xmin = int( nx * float(shape[0])/float(nsubx) );
	  xmax = int( (nx+1) * float(shape[0])/float(nsubx) )-1;
	  ymin = int( ny * float(shape[1])/float(nsuby) );
	  ymax = int( (ny+1) * float(shape[1])/float(nsuby) )-1;
	  outputnamebase<<modelimage << "_w"<<comms.rank()-1;
	}
	else{ // if serial mode, use the full range of x & y
	  xmin=ymin=0;
	  xmax=shape[0]-1;
	  ymax=shape[1]-1;
	  outputnamebase << modelimage;
	}

	ASKAPLOG_DEBUG_STR(logger, "isParallel="<<comms.isParallel()<< " rank="<<comms.rank()<<"   x in ["<<xmin<<","<<xmax<<"]   y in ["<<ymin << "," << ymax << "]");


	casa::IPosition outshape(2,shape[0],shape[1]);
	outshape[specAxis] = 1;
	ASKAPLOG_DEBUG_STR(logger, "Shape of output images is " << outshape);
	casa::Array<Float> outputs[nterms];
	for(int i=0;i<nterms;i++){
	  outputs[i] = casa::Array<Float>(outshape,0.);
	}

	casa::IPosition start(shape.size(),0);
	casa::IPosition end(shape-1);      

	const int ndata=shape[specAxis];
	const int degree=nterms+2;
	double chisq;
	gsl_matrix *xdat, *cov;
	gsl_vector *ydat, *w, *c;
	xdat = gsl_matrix_alloc(ndata,degree);
	ydat = gsl_vector_alloc(ndata);
	w = gsl_vector_alloc(ndata);
	c = gsl_vector_alloc(degree);
	cov = gsl_matrix_alloc(degree,degree);

      
	double reffreq = img.coordinates().spectralCoordinate(specCoord).referenceValue()[0];
	ASKAPLOG_DEBUG_STR(logger, "Reference = " << img.coordinates().spectralCoordinate(specCoord).referenceValue());
	for(int i=0;i<ndata;i++){
	  double freq;
	  if(!img.coordinates().spectralCoordinate(specCoord).toWorld(freq,double(i)))
	    ASKAPLOG_ERROR_STR(logger, "Error converting spectral coordinate at channel " << i);
	  //	  float logfreq=log10((wcs->crval[wcs->spec] + (i-wcs->crpix[wcs->spec])*wcs->cdelt[wcs->spec])/wcs->crval[wcs->spec]);
	  float logfreq = log10(freq/reffreq);
	  gsl_matrix_set(xdat,i,0,1.);
	  gsl_matrix_set(xdat,i,1,logfreq);
	  gsl_matrix_set(xdat,i,2,logfreq*logfreq);
// 	  gsl_matrix_set(xdat,i,3,logfreq*logfreq*logfreq);
// 	  gsl_matrix_set(xdat,i,4,logfreq*logfreq*logfreq*logfreq);
	  gsl_vector_set(w,i,1.);
	}

	for(int y=ymin; y<=ymax; y++){
	  for(int x=xmin; x<=xmax; x++){

	    // LOOP OVER Y AND X
	    // EXTRACT SPECTRUM FROM MODEL IMAGE
	    // FIT TO SPECTRUM
	    // STORE FIT RESULTS IN OUTPUT ARRAYS

	    if( (x+y*(xmax-xmin+1)) % int((xmax-xmin+1)*(ymax-ymin+1)*logevery/100.) == 0 )
	      ASKAPLOG_INFO_STR(logger, "Done " << x+y*(xmax-xmin+1) << " spectra out of " << (xmax-xmin+1)*(ymax-ymin+1) <<" with x="<<x<<" and y="<<y);

	    start[0]=end[0]=x;
	    start[1]=end[1]=y;
	    casa::Slicer specslice(start,end,casa::Slicer::endIsLast);
	    casa::Array<Float> spectrum = img.getSlice(specslice,True);
	    if(spectrum(IPosition(shape.size(),0))>1.e-20){
	      casa::Array<Float>::iterator iterSpec=spectrum.begin();
	      for (int i=0;i<ndata;i++){
		gsl_vector_set(ydat,i,log10(double(*iterSpec++)));
	      }
	      gsl_multifit_linear_workspace * work = gsl_multifit_linear_alloc (ndata,degree);
	      gsl_multifit_wlinear (xdat, w, ydat, c, cov, &chisq, work);
	      gsl_multifit_linear_free (work);
	      
	      casa::IPosition outpos(2,x,y);
	      float logflux=gsl_vector_get(c,0);
	      outputs[0](outpos) = pow(10.,logflux);
	      outputs[1](outpos) = gsl_vector_get(c,1);
	      outputs[2](outpos) = gsl_vector_get(c,2);
	    }
	  }
	}

	Unit bunit = img.units();
	casa::Vector<casa::Quantum<Double> > beam = img.imageInfo().restoringBeam();
      
	for(int t=0;t<nterms;t++){
	  std::stringstream name;
	  name << outputnamebase.str() <<".taylor." << t;

	  casa::CoordinateSystem csys = img.coordinates();
	  casa::IPosition tileshape(shape.size(),1);
	  tileshape(0) = std::min(128L,shape(0));
	  tileshape(1) = std::min(128L,shape(1));
	  shape(specAxis)=1;
	
	  ASKAPLOG_INFO_STR(logger, "Creating a new CASA image " << name.str() << " with the shape " << shape << " and tileshape " << tileshape);
	  casa::PagedImage<float> img(casa::TiledShape(shape,tileshape), csys, name.str());
	
	  img.setUnits(bunit);
	  casa::ImageInfo ii = img.imageInfo();
	  ii.setRestoringBeam(beam);
	  img.setImageInfo(ii);

	  casa::IPosition location(shape.size(),0);
	  img.putSlice(outputs[t], location);
	
	}
      }

    } catch (const askap::AskapError& x) {
    ASKAPLOG_FATAL_STR(logger, "Askap error in " << argv[0] << ": " << x.what());
    std::cerr << "Askap error in " << argv[0] << ": " << x.what() << std::endl;
    exit(1);
  } catch (const std::exception& x) {
    ASKAPLOG_FATAL_STR(logger, "Unexpected exception in " << argv[0] << ": " << x.what());
    std::cerr << "Unexpected exception in " << argv[0] << ": " << x.what() << std::endl;
    exit(1);
  }

  return 0;


}

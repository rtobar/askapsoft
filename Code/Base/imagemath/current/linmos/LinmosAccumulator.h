/// @file LinmosAccumulator.h
///
/// @brief combine a number of images as a linear mosaic
/// @details
///
/// @copyright (c) 2012,2014,2015 CSIRO
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>
/// @author Daniel Mitchell <daniel.mitchell@csiro.au>

#ifndef ASKAP_LINMOS_H
#define ASKAP_LINMOS_H

// other 3rd party
#include <casacore/casa/Arrays/Array.h>
#include <casacore/images/Images/ImageRegrid.h>

// ASKAPsoft includes
#include <utils/MultiDimArrayPlaneIter.h>
#include <primarybeam/PrimaryBeam.h>


using namespace casa;

namespace askap {

    namespace imagemath {

        /// @brief Base class supporting linear mosaics (linmos)

        template<typename T>
        class LinmosAccumulator {

            public:

                LinmosAccumulator();

                /// @brief check parset parameters
                /// @details check parset parameters for consistency and set any dependent variables
                ///     weighttype: FromWeightImages or FromPrimaryBeamModel. No default.
                ///     weightstate: Corrected, Inherent or Weighted. Default: Corrected.
                /// @param[in] const LOFAR::ParameterSet &parset: linmos parset
                /// @return bool true=success, false=fail
                bool loadParset(const LOFAR::ParameterSet &parset);

                /// @brief set up a single mosaic
                /// @param[in] const vector<string> &inImgNames : vector of images to mosaic
                /// @param[in] const vector<string> &inWgtNames : vector of weight images, if required
                /// @param[in] const string &outImgName : output mosaic image name
                /// @param[in] const string &outWgtName : output mosaic weight image name
                void setSingleMosaic(const vector<string> &inImgNames,
                                     const vector<string> &inWgtNames,
                                     const string &outImgName, const string &outWgtName);


                /// @brief set up a single mosaic for each taylor term
                /// @param[in] const vector<string> &inImgNames : vector of images to mosaic
                /// @param[in] const vector<string> &inWgtNames : vector of weight images, if required
                /// @param[in] const string &outImgName : output mosaic image name
                /// @param[in] const string &outWgtName : output mosaic weight image name
                void findAndSetTaylorTerms(const vector<string> &inImgNames,
                                           const vector<string> &inWgtNames,
                                           const string &outImgName,
                                           const string &outWgtName);

                /// @brief if the images have not been corrected for the primary
                /// beam they still contain the spectral structure of the primary
                /// beam as well as their intrinsic spectral indices
                /// this method decouples the beam spectral behaviour from the images
                /// Based on a Gaussian beam approximation.
                /// @param[in] const vector<string> &inImgNames : vector of images to mosaic
                /// @param[in] const vector<string> &inWgtNames : vector of weight images, if required
                /// @param[in] const string &outImgName : output mosaic image name
                /// @param[in] const string &outWgtName : output mosaic weight image name


                void removeBeamFromTaylorTerms(Array<T> &taylor0,
                                               Array<T> &taylor1,
                                               Array<T> &taylor2,
                                               const IPosition& curpos,
                                               const CoordinateSystem& inSys);

                /// @brief search the current directory for suitable mosaics
                /// @details based on a vector of image tags, look for sets of images with names
                ///     that contain all tags but are otherwise equal and contain an allowed prefix.
                /// @param[in] const vector<string> &imageTags : vector of image tags
                void findAndSetMosaics(const vector<string> &imageTags);

                /// @brief test whether the output buffers are empty and need initialising
                /// @return bool
                bool outputBufferSetupRequired(void);

                /// @brief set the input coordinate system and shape
                /// @param[in] const IPosition&: shape of input to be merged
                /// @param[in] const CoordinateSystem&: coord system of input
                /// @param[in] const int: input order of the input file
                void setInputParameters(const IPosition& inShape,
                                        const CoordinateSystem& inCoordSys,
                                        const int n=0);

                /// @brief set the output coordinate system and shape
                /// @param[in] const IPosition&: shape of output
                /// @param[in] const CoordinateSystem&: coord system of output
                /// @param[in] const int: input order of the input file
                void setOutputParameters(const IPosition& outShape,
                                         const CoordinateSystem& outCoordSys);

                /// @brief set the output coordinate system and shape, based on the overlap of input images
                /// @details This method is based on the SynthesisParamsHelper::add and
                ///     SynthesisParamsHelper::facetSlicer. It has been reimplemented here
                ///     so that images can be read into memory separately.
                /// @param[in] const vector<IPosition>&: shapes of inputs to be merged
                /// @param[in] const vector<CoordinateSystem>&: coord systems of inputs
                void setOutputParameters(const vector<IPosition>& inShapeVec,
                                         const vector<CoordinateSystem>& inCoordSysVec);

                /// @brief set up any 2D temporary output image buffers required for regridding
                void initialiseOutputBuffers(void);

                /// @brief set up any 2D temporary input image buffers required for regridding
                void initialiseInputBuffers(void);

                /// @brief set up regridder
                void initialiseRegridder(void);

                /// @brief load the temporary image buffers with the current plane of the current input image
                /// @param[in] const scimath::MultiDimArrayPlaneIter& planeIter: current plane id
                /// @param[in] Array<T>& inPix: image buffer
                /// @param[in] Array<T>& inWgtPix: weight image buffer
                void loadInputBuffers(const scimath::MultiDimArrayPlaneIter& planeIter,
                                      Array<T>& inPix,
                                      Array<T>& inWgtPix,
                                      Array<T>& inSenPix);

                /// @brief call the regridder for the buffered plane
                void regrid(void);

                /// @brief add the current plane to the accumulation arrays
                /// @details This method adds from the regridded buffers
                /// @param[out] Array<T>& outPix: accumulated weighted image pixels
                /// @param[out] Array<T>& outWgtPix: accumulated weight pixels
                /// @param[in] const IPosition& curpos: indices of the current plane
                void accumulatePlane(Array<T>& outPix,
                                     Array<T>& outWgtPix,
                                     Array<T>& outSenPix,
                                     const IPosition& curpos);

                /// @brief add the current plane to the accumulation arrays
                /// @details This method adds directly from the input arrays
                /// @param[out] Array<T>& outPix: accumulated weighted image pixels
                /// @param[out] Array<T>& outWgtPix: accumulated weight pixels
                /// @param[in] const Array<T>& inPix: input image pixels
                /// @param[in] const Array<T>& inWgtPix: input weight pixels
                /// @param[in] const IPosition& curpos: indices of the current plane
                void accumulatePlane(Array<T>& outPix,
                                     Array<T>& outWgtPix,
                                     Array<T>& outSenPix,
                                     const Array<T>& inPix,
                                     const Array<T>& inWgtPix,
                                     const Array<T>& inSenPix,
                                     const IPosition& curpos);

                /// @brief divide the weighted pixels by the weights for the current plane
                /// @param[in,out] Array<T>& outPix: accumulated deweighted image pixels
                /// @param[in] const Array<T>& outWgtPix: accumulated weight pixels
                /// @param[in] const IPosition& curpos: indices of the current plane
                void deweightPlane(Array<T>& outPix,
                                   const Array<T>& outWgtPix,
                                   Array<T>& outSenPix,
                                   const IPosition& curpos);

                /// @brief check to see if the input and output coordinate grids are equal
                /// @return bool: true if they are equal
                bool coordinatesAreEqual(void);

                /// @brief check to see if two coordinate grids are equal
                /// @return bool: true if they are equal
                bool coordinatesAreEqual(const CoordinateSystem& coordSys1,
                                         const CoordinateSystem& coordSys2,
                                         const IPosition& shape1,
                                         const IPosition& shape2);

                // return metadata for the current input image
                IPosition inShape(void) {return itsInShape;}
                CoordinateSystem inCoordSys(void) {return itsInCoordSys;}

                // return metadata for the output image
                IPosition outShape(void) {return itsOutShape;}
                CoordinateSystem outCoordSys(void) {return itsOutCoordSys;}

                int weightType(void) {return itsWeightType;}
                void weightType(int type) {itsWeightType = type;}
                int weightState(void) {return itsWeightState;}
                void weightState(int state) {itsWeightState = state;}
                int numTaylorTerms(void) {return itsNumTaylorTerms;}
                bool doSensitivity(void) {return itsDoSensitivity;}
                void doSensitivity(bool value) {itsDoSensitivity = value;}
                string taylorTag(void) {return itsTaylorTag;}

                void beamCentres(Vector<MVDirection> centres) {itsCentres = centres;}

                map<string,string> outWgtNames(void) {return itsOutWgtNames;}
                map<string,string> outSenNames(void) {return itsOutSenNames;}
                map<string,vector<string> > inImgNameVecs(void) {return itsInImgNameVecs;}
                map<string,vector<string> > inWgtNameVecs(void) {return itsInWgtNameVecs;}
                map<string,vector<string> > inSenNameVecs(void) {return itsInSenNameVecs;}
                map<string,bool> outWgtDuplicates(void) {return itsOutWgtDuplicates;}
                map<string,bool> genSensitivityImage(void) {return itsGenSensitivityImage;}

            private:

                /// @brief convert the current input shape and coordinate system to the reference (output) system
                /// @param[in] const DirectionCoordinate& refDC: reference direction coordinate
                /// @return IPosition vector containing BLC and TRC of the current input image,
                ///         relative to another coord. system
                Vector<IPosition> convertImageCornersToRef(const DirectionCoordinate& refDC);

                /// @brief check to see if two coordinate systems are consistent enough to merge
                /// @param[in] const CoordinateSystem& coordSys1
                /// @param[in] const CoordinateSystem& coordSys2
                /// @return bool: true if they are consistent
                bool coordinatesAreConsistent(const CoordinateSystem& coordSys1,
                                              const CoordinateSystem& coordSys2);

                // regridding options
                ImageRegrid<T> itsRegridder;
                IPosition itsAxes;
                String itsMethod;
                Int itsDecimate;
                Bool itsReplicate;
                Bool itsForce;
                Interpolate2D::Method itsEmethod;
                // regridding buffers
                TempImage<T> itsInBuffer, itsInWgtBuffer, itsInSenBuffer, itsInSnrBuffer;
                TempImage<T> itsOutBuffer, itsOutWgtBuffer, itsOutSnrBuffer;
                // metadata objects
                IPosition itsInShape;
                CoordinateSystem itsInCoordSys;
                IPosition itsOutShape;
                CoordinateSystem itsOutCoordSys;
                // options
                int itsWeightType;
                int itsWeightState;
                int itsNumTaylorTerms;
                bool itsDoSensitivity;

                T itsCutoff;

                //
                Vector<MVDirection> itsCentres;
                MVDirection itsInCentre;

                // Set some objects to support multiple mosaics.
                const string itsMosaicTag;
                const string itsTaylorTag;

                map<string,string> itsOutWgtNames;
                map<string,string> itsOutSenNames;
                map<string,vector<string> > itsInImgNameVecs;
                map<string,vector<string> > itsInWgtNameVecs;
                map<string,vector<string> > itsInSenNameVecs;
                map<string,bool> itsOutWgtDuplicates;
                map<string,bool> itsGenSensitivityImage;
                //
                PrimaryBeam::ShPtr itsPB;

        };

    } // namespace imagemath

} // namespace askap

#include <linmos/LinmosAccumulator.tcc>

#endif

/// @file SpectrumElement.cc
///
/// @copyright (c) 2017 CSIRO
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
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>

// Include own header file first
#include "casdaupload/SpectrumElement.h"

// Include package level header file
#include "askap_pipelinetasks.h"

// System includes
#include <string>

// ASKAPsoft includes
#include "casdaupload/ProjectElementBase.h"
#include "casdaupload/ElementBase.h"
#include "casdaupload/CasdaFileUtils.h"
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "xercesc/dom/DOM.hpp" // Includes all DOM
#include "boost/filesystem.hpp"
#include "votable/XercescString.h"
#include "votable/XercescUtils.h"

// Using
using namespace askap::cp::pipelinetasks;
using xercesc::DOMElement;
using askap::accessors::XercescString;
using askap::accessors::XercescUtils;

ASKAP_LOGGER(logger, ".SpectrumElement");


SpectrumElement::SpectrumElement(const LOFAR::ParameterSet &parset)
    : TypeElementBase(parset),
      itsFilenameList(),
      itsThumbnailList(),
      itsNumSpectra(0)
{
    itsName = "spectrum";
    itsFormat = "fits";
    if (itsFilepath.extension() != "." + itsFormat) {
        ASKAPTHROW(AskapError,
                   "Unsupported format image - Expect " << itsFormat << " file extension");
    }
    itsThumbnail = parset.getString("thumbnail", "");

    checkWildcards();
}

void SpectrumElement::checkWildcards()
{

    // glob itsFilepath to get a list of possible names
    //     --> fills itsFilenameList

    // glob itsThumbnail to get a list of possible names
    //     --> fills itsThumbnailList

    itsNumSpectra = itsFilenameList.size();
    ASKAPCHECK(itsThumbnailList.size() != itsNumSpectra, "Thumbnail wildcard produces more files than filename");
    
}

xercesc::DOMElement* SpectrumElement::toXmlElement(xercesc::DOMDocument& doc) const
{
    DOMElement* e = TypeElementBase::toXmlElement(doc);

    if (itsThumbnail != "") {
        XercescUtils::addTextElement(*e, "thumbnail", itsThumbnail.filename().string());
    }

    std::stringstream ss;
    ss << itsNumSpectra;
    XercescUtils::addTextElement(*e, "number", ss.str());

    return e;
}


void SpectrumElement::copyAndChecksum(const boost::filesystem::path& outdir) const
{

    const boost::filesystem::path in(itsFilepath);
    const boost::filesystem::path out(outdir / in.filename());
    ASKAPLOG_INFO_STR(logger, "Copying and calculating checksum for " << in);
    CasdaFileUtils::copyAndChecksum(in, out);

    if (itsThumbnail != "" ) {
        const boost::filesystem::path in(itsThumbnail);
        const boost::filesystem::path out(outdir / in.filename());
        ASKAPLOG_INFO_STR(logger, "Copying and calculating checksum for " << in);
        CasdaFileUtils::copyAndChecksum(in, out);
    }

}


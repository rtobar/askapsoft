/// @file MockVisSource.h
///
/// @copyright (c) 2010 CSIRO
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
/// @author Ben Humphreys <ben.humphreys@csiro.au>

#ifndef ASKAP_CP_MOCKVISSOURCE_H
#define ASKAP_CP_MOCKVISSOURCE_H

// ASKAPsoft includes
#include "boost/shared_ptr.hpp"
#include "cpcommon/VisDatagram.h"

// Local package includes
#include "ingestpipeline/sourcetask/IVisSource.h"
#include "ingestpipeline/sourcetask/test/DequeWrapper.h"

namespace askap {
    namespace cp {

        class MockVisSource : public askap::cp::IVisSource
        {
            public:
                MockVisSource();
                virtual ~MockVisSource();

                void add(boost::shared_ptr< VisDatagram > obj);

                boost::shared_ptr< VisDatagram > next(const long timeout = -1);

                // Shared pointer definition
                typedef boost::shared_ptr<MockVisSource> ShPtr;

            private:
                DequeWrapper< VisDatagram > itsBuffer;
        };

    };
};

#endif

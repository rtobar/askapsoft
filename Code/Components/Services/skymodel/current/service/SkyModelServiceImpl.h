/// @file SkyModelServiceImpl.h
///
/// @copyright (c) 2016 CSIRO
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
/// @author Daniel Collins <daniel.collins@csiro.au>

#ifndef ASKAP_CP_SMS_SKYMODELSERVICEIMPL_H
#define ASKAP_CP_SMS_SKYMODELSERVICEIMPL_H

// System includes
#include <string>

// ASKAPsoft includes
#include <boost/noncopyable.hpp>
#include <Ice/Ice.h>

// Ice interfaces
#include <SkyModelService.h>

// Local package includes

namespace askap {
namespace cp {
namespace sms {

namespace sms_interface = askap::interfaces::skymodelservice;

/// This class implements the "ISkyModelService" Ice interface.
class SkyModelServiceImpl : 
    public sms_interface::ISkyModelService,
    private boost::noncopyable {
    public:
        /// @brief Constructor.
        SkyModelServiceImpl();

        /// @brief Destructor.
        virtual ~SkyModelServiceImpl();

        virtual std::string getServiceVersion(const Ice::Current&);

        virtual sms_interface::ComponentIdSeq coneSearch(
            double rightAscension,
            double declination,
            double searchRadius, 
            double fluxLimit,
            const Ice::Current&);

        virtual sms_interface::ComponentSeq getComponents(
            const sms_interface::ComponentIdSeq& componentIds,
            const Ice::Current&);

        virtual sms_interface::ComponentIdSeq addComponents(
            const sms_interface::ComponentSeq& components,
            const Ice::Current&);

        virtual void removeComponents(
            const sms_interface::ComponentIdSeq& componentIds,
            const Ice::Current&);

    private:
};

}
}
}

#endif

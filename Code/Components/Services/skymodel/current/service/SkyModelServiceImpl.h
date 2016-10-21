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
#include <boost/shared_ptr.hpp>
#include <Common/ParameterSet.h>
#include <Ice/Ice.h>

// Ice interfaces
#include <SkyModelService.h>

// ODB
#include <odb/database.hxx>

// Local package includes

namespace askap {
namespace cp {
namespace sms {

namespace sms_interface = askap::interfaces::skymodelservice;

/// This class implements the "ISkyModelService" Ice interface.
/// In Ice terminology, it is a servant class.
class SkyModelServiceImpl : 
    public sms_interface::ISkyModelService,
    private boost::noncopyable {
    public:

        /// @brief Factory method for constructing the SkyModelService
        /// implementation.
        ///
        /// @return The SkyModelServiceImpl instance.
        /// @throw AskapError   If the implementation cannot be constructed.
        static SkyModelServiceImpl* create(const LOFAR::ParameterSet& parset);

        /// @brief Destructor.
        virtual ~SkyModelServiceImpl();

        /// @brief Initialises an empty database with the schema
        ///
        /// @return true if the schema was created; false if the schema already exists
        bool createSchema();

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
        /// @brief Constructor. 
        /// Private. Use the factory method to create.
        /// @param itsDb The odb::database instance.
        SkyModelServiceImpl(
            boost::shared_ptr<odb::database> database,
            const std::string& tablespace);

        /// @brief SQLite-specific schema creation method
        ///
        /// @param dropTables Should existing tables be dropped or not.
        void createSchemaSqlite(bool dropTables=true);

        /// @brief The odb database
        boost::shared_ptr<odb::database> itsDb;

        const std::string itsTablespace;
};

}
}
}

#endif

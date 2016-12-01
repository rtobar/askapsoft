/// @file GlobalSkyModel.h
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

#ifndef ASKAP_CP_SMS_GLOBALSKYMODEL_H
#define ASKAP_CP_SMS_GLOBALSKYMODEL_H

// System includes
#include <string>

// ASKAPsoft includes
#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <Common/ParameterSet.h>

// ODB
#include <odb/database.hxx>

// Local package includes
#include "datamodel/ContinuumComponent.h"

namespace askap {
namespace cp {
namespace sms {

/// @brief Service facade to the Global Sky Model database.
///
/// Separating this from the Ice interface implementation allows
/// a non-Ice command-line application to use the same database access code.
class GlobalSkyModel :
    private boost::noncopyable {
    public:

        typedef std::vector<datamodel::ContinuumComponent> ComponentList;

        /// @brief Factory method for constructing the GlobalSkyModel implementation.
        ///
        /// @param parset The parameter set
        /// @return The GlobalSkyModel instance.
        /// @throw AskapError   If the implementation cannot be constructed.
        static GlobalSkyModel* create(const LOFAR::ParameterSet& parset);

        /// @brief Destructor.
        virtual ~GlobalSkyModel();

        /// @brief Initialises an empty database with the schema
        /// @param dropTables Should existing tables be dropped or not.
        /// @return true if the schema was created; false if the schema already exists
        bool createSchema(bool dropTables=true);

        /// @brief Ingests a VO table of Continuum Components into the GSM.
        /// @param componentsCatalog The VO table file name for the continuum components.
        /// @param polarisationCatalog The VO table file name for the polarisation data.
        /// @return true on success; otherwise false.
        bool ingestVOTable(const std::string& componentsCatalog, const std::string& polarisationCatalog);

        /// @brief Get the HEALPix NSIDE value.
        ///
        /// @return NSIDE
        inline boost::int64_t getHealpixNside() const {
            return 2l << getHealpixOrder();
        }

        /// @brief Get the HEALPix Order value
        ///
        /// @return Order
        inline boost::int64_t getHealpixOrder() const {
            return 14l;
        }

        /// @brief Get a component by ID. Probably only useful in testing.
        ///
        /// @return The component, or null if not found.
        datamodel::ContinuumComponent* getComponentByID(long id) const;

    private:
        /// @brief Constructor.
        /// Private. Use the factory method to create.
        /// @param itsDb The odb::database instance.
        GlobalSkyModel(boost::shared_ptr<odb::database> database);

        /// @brief SQLite-specific schema creation method
        ///
        /// @param dropTables Should existing tables be dropped or not.
        void createSchemaSqlite(bool dropTables=true);

        /// @brief The odb database
        boost::shared_ptr<odb::database> itsDb;
};

}
}
}

#endif

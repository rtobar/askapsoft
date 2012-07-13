// @file ObsProgramService.ice
//
// @copyright (c) 2010 CSIRO
// Australia Telescope National Facility (ATNF)
// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
// PO Box 76, Epping NSW 1710, Australia
// atnf-enquiries@csiro.au
//
// This file is part of the ASKAP software distribution.
//
// The ASKAP software distribution is free software: you can redistribute it
// and/or modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the License,
// or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

#ifndef ASKAP_OBSPROGRAMSERVICE_ICE
#define ASKAP_OBSPROGRAMSERVICE_ICE

#include <CommonTypes.ice>
#include <DataServiceExceptions.ice>
#include <IService.ice>

module askap
{

module interfaces
{

module schedblock
{
    interface IObsProgramService extends askap::interfaces::services::IService
    {
        long create(string name, string investigator, string opalid)
            throws VersionException;

        long getId(string name) throws NoSuchObsProgramException;

        StringSeq getAll();

    };
};
};
};

#endif

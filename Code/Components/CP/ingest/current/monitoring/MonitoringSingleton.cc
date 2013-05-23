/// @file MonitoringSingleton.cc
///
/// @copyright (c) 2013 CSIRO
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

// Include own header file first
#include "MonitoringSingleton.h"

// Include package level header file
#include "askap_cpingest.h"

// System includes
#include <string>
#include <vector>
#include <ctime>

// ASKAPsoft includes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "boost/shared_ptr.hpp"
#include "Ice/Ice.h"
#include "iceutils/CommunicatorConfig.h"
#include "iceutils/CommunicatorFactory.h"
#include "MoniCA.h" // ICE generated interface

// Local package includes
#include "configuration/Configuration.h" // Includes all configuration attributes too

ASKAP_LOGGER(logger, ".MonitoringSingleton");

using namespace askap;
using namespace askap::cp::icewrapper;
using namespace askap::cp::ingest;
using namespace atnf::atoms::mon::comms;

// Initialise statics
MonitoringSingleton* MonitoringSingleton::itsInstance = 0;

MonitoringSingleton::MonitoringSingleton(const Configuration& config)
        : itsConfig(config)
{
    // Create the prefix for all point names
    itsPrefix = "cp.ingest_" + utility::toString(config.rank());

    // Setup ICE
    const string registryHost = config.monitoringArchiverService().registryHost();
    const string registryPort = config.monitoringArchiverService().registryPort();
    CommunicatorConfig commconfig(registryHost, registryPort);
    CommunicatorFactory commFactory;
    itsComm = commFactory.createCommunicator(commconfig);

    ASKAPDEBUGASSERT(itsComm);

    const string serviceName = config.monitoringArchiverService().serviceIdentity();
    Ice::ObjectPrx base = itsComm->stringToProxy(serviceName);
    itsMonicaProxy = atnf::atoms::mon::comms::MoniCAIcePrx::checkedCast(base);

    if (!itsMonicaProxy) {
        ASKAPLOG_WARN_STR(logger, "Failed to obtain MoniCA proxy");
    } else {
        // Start the thread
        itsThread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&MonitoringSingleton::senderrun, this)));
    }
}

MonitoringSingleton::~MonitoringSingleton()
{
    if (itsThread.get()) {
        itsThread->interrupt();
        itsCondVar.notify_all();
        itsThread->join();
    }

    if (itsComm) {
        itsComm->destroy();
    }
}

MonitoringSingleton* MonitoringSingleton::instance(void)
{
    return itsInstance;
}

void MonitoringSingleton::init(const Configuration& config)
{
    if (!itsInstance) {
        itsInstance = new MonitoringSingleton(config);
    } else {
        ASKAPTHROW(AskapError, "Monitoring Singleton already initialised");
    }
}

void MonitoringSingleton::destroy()
{
    if (itsInstance) {
        delete itsInstance;
        itsInstance = 0;
    }
}

void MonitoringSingleton::sendBool(const std::string& name, bool value)
{
    enqueue(name, new DataValueBoolean(DTBoolean, value));
}

void MonitoringSingleton::sendFloat(const std::string& name, float value)
{
    enqueue(name, new DataValueFloat(DTFloat, value));
}

void MonitoringSingleton::sendDouble(const std::string& name, double value)
{
    enqueue(name, new DataValueDouble(DTDouble, value));
}

void MonitoringSingleton::sendInt32(const std::string& name, int32_t value)
{
    enqueue(name, new DataValueInt(DTInt, value));
}

void MonitoringSingleton::sendInt64(const std::string& name, int64_t value)
{
    enqueue(name, new DataValueLong(DTLong, value));
}

void MonitoringSingleton::sendString(const std::string& name, const std::string& value)
{
    enqueue(name, new DataValueString(DTString, value));
}

void MonitoringSingleton::enqueue(const std::string& name, atnf::atoms::mon::comms::DataValuePtr value)
{
    PointDataIce pd;
    pd.name = itsPrefix + name;
    pd.timestamp = getTime();
    pd.alarm = false;
    pd.value = value;
    boost::mutex::scoped_lock lock(itsMutex);
    itsBuffer.push_front(pd);

    // Notify any waiters
    lock.unlock();
    itsCondVar.notify_all();
}

long MonitoringSingleton::getTime(void) const
{
    return time(0);
}

void MonitoringSingleton::senderrun(void)
{
    vector<string> names;
    vector<PointDataIce> values;

    try {
        while (!(boost::this_thread::interruption_requested())) {

            // Wait for some data to send
            boost::mutex::scoped_lock lock(itsMutex);
            while (itsBuffer.empty()) {
                itsCondVar.wait(lock);
                boost::this_thread::interruption_point();
            }

            // Extract the data from the buffer
            while (!itsBuffer.empty()) {
                names.push_back(itsBuffer.back().name);
                values.push_back(itsBuffer.back());
                itsBuffer.pop_back();
                boost::this_thread::interruption_point();
            }

            // Send the batch
            itsMonicaProxy->setData(names, values, "notused", "notused");
            names.clear();
            values.clear();
        }
    } catch (boost::thread_interrupted& e) {
        // Nothing to do, just return
    }
}
/// @file 
///
/// @brief parallel thread which runs the correlator
/// @details This class is analogous to the main method of the stand alone correlator 
/// application. It can run the correlator, get monitoring data and stop when necessary.
/// The primary goal for this interface is to run the software correlator from the epics
/// CA server. We use parallel thread to get the asynchronous behavior. This class represents
/// the child thread and CorrRunner the main thread.
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#include <corrinterfaces/CorrRunnerThread.h>
#include <corrinterfaces/CorrRunner.h>
#include <askap/AskapError.h>

namespace askap {

namespace swcorrelator {

/// @brief constructor
/// @details
/// @param[in] parent shared pointer to an instance of the main thread class
/// @param[in] parset shared pointer to the parset
CorrRunnerThread::CorrRunnerThread(const boost::shared_ptr<CorrRunner> &parent, 
             const boost::shared_ptr<LOFAR::ParameterSet> &parset) :
             itsParent(parent), itsParset(parset) 
{
  ASKAPDEBUGASSERT(parent);
  itsParent->setStatus(false,"OK");
}             
   
/// @brief the entry point for the parallel thread
void CorrRunnerThread::operator()()
{
  ASKAPDEBUGASSERT(itsParent);
  if (itsParset) {
      std::string status = "ERROR: ";
      try {
        status = "OK";
      }
      catch (const AskapError &ae) {
         status += ae.what();          
      }
      catch (const std::exception &ex) {
         status += ex.what();          
      }
      catch (...) {
         status += "unexpected";
      }
      itsParent->setStatus(false,status);          
  } else {
     itsParent->setStatus(false,"ERROR: Parset is not defined");
  }
}


} // namespace swcorrelator

} // namespace askap
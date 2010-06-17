/// @file
/// @brief Base class for monitor of Deconvolver
/// @details All the monitoring is delegated to this class so that
/// more control is possible.
/// @ingroup Deconvolver
///  
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
///

#ifndef I_DECONVOLVERMONITOR_H
#define I_DECONVOLVERMONITOR_H
#include <casa/aips.h>
#include <boost/shared_ptr.hpp>

#include <casa/Arrays/Array.h>

#include <string>

#include <deconvolution/DeconvolverState.h>

namespace askap {

namespace synthesis {

/// @brief Base class for monitor of Deconvolver
/// @details All the monitoring is delegated to this class so that
/// more control is possible.
/// @ingroup Deconvolver

template<class T> class DeconvolverMonitor {

public:
  typedef boost::shared_ptr<DeconvolverMonitor<T> > ShPtr;
  
  virtual ~DeconvolverMonitor() {};
  
  /// Monitor the current state
  virtual void monitor(const DeconvolverState<T>& ds);

private:

};

} // namespace synthesis

} // namespace askap

#include <deconvolution/DeconvolverMonitor.tcc>

#endif


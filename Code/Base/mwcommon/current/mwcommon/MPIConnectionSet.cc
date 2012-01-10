/// @file MPIConnectionSet.cc
/// @brief Set of MPI connections
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
/// @author Ger van Diepen <diepen@astron.nl>

// System includes
#include <vector>

// ASKAPsoft includes
#include "askap/AskapError.h"
#include "Blob/BlobString.h"

// Local package includes
#include "mwcommon/MPIConnectionSet.h"

namespace askap { namespace mwcommon {

  MPIConnectionSet::MPIConnectionSet()
  {}

  MPIConnectionSet::~MPIConnectionSet()
  {}

  MWConnectionSet::ShPtr
  MPIConnectionSet::clone (const std::vector<int>& inx) const
  {
    const int nrconn = size();
    MPIConnectionSet* set = new MPIConnectionSet();
    MWConnectionSet::ShPtr mwset(set);
    for (std::vector<int>::const_iterator it=inx.begin();
         it!=inx.end();
         ++it) {
      const int i = *it;
      ASKAPASSERT (i>=0 && i<nrconn);
      set->itsConns.push_back (itsConns[i]);
    }
    return mwset;
  }

  int MPIConnectionSet::addConnection (int rank, int tag)
  {
    const int seqnr = itsConns.size();
    MPIConnection::ShPtr ptr(new MPIConnection (rank, tag));
    itsConns.push_back (ptr);
    return seqnr;
  }

  int MPIConnectionSet::size() const
  {
    return itsConns.size();
  }

  int MPIConnectionSet::getReadyConnection()
  {
    return -1;
  }

  void MPIConnectionSet::read(int seqnr, void* buf, size_t size)
  {
    itsConns[seqnr]->receive(buf, size);
  }

  void MPIConnectionSet::write(int seqnr, void* buf, size_t size)
  {
    itsConns[seqnr]->send(buf, size);
  }

  void MPIConnectionSet::read (int seqnr, LOFAR::BlobString& buf)
  {
    itsConns[seqnr]->read(buf);
  }

  void MPIConnectionSet::write (int seqnr, const LOFAR::BlobString& buf)
  {
    itsConns[seqnr]->write(buf);
  }

  void MPIConnectionSet::writeAll (const LOFAR::BlobString& buf)
  {
    for (unsigned int i=0; i<itsConns.size(); ++i) {
      itsConns[i]->write (buf);
    }
  }
  
  /// @brief broadcast blob to all ranks by the connected MWConnection
  /// @details this method waits until all data has arrived into \a buf.
  /// The buffer is resized as needed.
  /// @param[in] buf blob string
  /// @param[in] root root rank which has the data
  /// @note This method requires at least one connection to be defined
  void MPIConnectionSet::broadcast(LOFAR::BlobString& buf, int root)
  {
    ASKAPCHECK(itsConns.size()>0, "MPIConnectionSet::broadcast - no connections defined!");
    ASKAPDEBUGASSERT(itsConns[0]);
    itsConns[0]->broadcast(buf,root);
  }
  

}} // end namespaces


// Package level header file
#include <askap_synthesis.h>

// ASKAPsoft includes
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".primarybeam.primarybeamfactory");
#include <askap/AskapError.h>
#include <casacore/casa/OS/DynLib.h>        // for dynamic library loading
#include <casacore/casa/BasicSL/String.h>   // for downcase


// Local package includes

#include <gridding/PrimaryBeam.h>
#include <gridding/PrimaryBeamFactory.h>

// Primary Beam Types
#include <gridding/GaussianPB.h>

namespace askap {
namespace synthesis {

  // Define the static registry.
  std::map<std::string, PrimaryBeamFactory::PrimaryBeamCreator*>
  PrimaryBeamFactory::theirRegistry;


  PrimaryBeamFactory::PrimaryBeamFactory() {
  }

  void PrimaryBeamFactory::registerPrimaryBeam (const std::string& name,
                                           PrimaryBeamFactory::PrimaryBeamCreator* creatorFunc)
  {
    ASKAPLOG_INFO_STR(logger, "     - Adding "<<name<<" Primary Beam to the registry");
    theirRegistry[name] = creatorFunc;
  }

  PrimaryBeam::ShPtr PrimaryBeamFactory::createPrimaryBeam (const std::string& name,
                                    const LOFAR::ParameterSet& parset)
  {
    std::map<std::string,PrimaryBeamCreator*>::const_iterator it = theirRegistry.find (name);
    if (it == theirRegistry.end()) {
      // Unknown Primary Beam. Try to load the data manager from a dynamic library
      // with that lowercase name (without possible template extension).
      std::string libname(toLower(name));
      const std::string::size_type pos = libname.find_first_of (".<");
      if (pos != std::string::npos) {
        libname = libname.substr (0, pos);      // only take before . or <
      }
      // Try to load the dynamic library and execute its register function.
      // Do not dlclose the library.
      ASKAPLOG_INFO_STR(logger, "Primary Beam "<<name<<
                 " is not in the registry, attempting to load it dynamically");
      casa::DynLib dl(libname, string("libaskap_"), "register_"+libname, false);
      if (dl.getHandle()) {
        // Successfully loaded. Get the creator function.
        ASKAPLOG_INFO_STR(logger, "Dynamically loaded Primary Beam " << name);
        // the first thing the Primary Beam in the shared library is supposed to do is to
        // register itself. Therefore, its name will appear in the registry.
        it = theirRegistry.find (name);
      }
    }
    if (it == theirRegistry.end()) {
      ASKAPTHROW(AskapError, "Unknown Primary Beam " << name);
    }
    // Execute the registered function.
    return it->second(parset);
  }

  // Make the Primary Beam object for the Primary Beam given in the parset file.
  // Currently the standard Beams are still handled by this function.
  // In the (near) future it should be done by putting creator functions
  // for these Beams in the registry and use that.

PrimaryBeam::ShPtr PrimaryBeamFactory::make(const LOFAR::ParameterSet &parset) {

    if (theirRegistry.size() == 0) {
        // this is the first call of the method, we need to fill the registry with
        // all pre-defined primary beams
        ASKAPLOG_INFO_STR(logger, "Filling the Primary Beam registry with pre-defined Beams");
        addPreDefinedPrimaryBeam<GaussianPB>();
        /// addPreDefinedPrimaryBeam<ExamplePB>();


    }

    // buffer for the result
    PrimaryBeam::ShPtr PB;
    /// @todo Better handling of string case
    std::string prefix("primarybeam");
    const string primaryBeamName = parset.getString(prefix);
    prefix += "." + primaryBeamName + ".";
    ASKAPLOG_INFO_STR(logger, "Attempting to greate primary beam "<<primaryBeamName);
    PB = createPrimaryBeam (primaryBeamName, parset.makeSubset(prefix));

    ASKAPASSERT(PB); // if a PB of that name is in the registry it will be here

    return PB;
}



} // namespace synthesis

} // namespace askap

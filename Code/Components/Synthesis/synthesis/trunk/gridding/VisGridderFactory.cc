#include <gridding/VisGridderFactory.h>
#include <gridding/BoxVisGridder.h>
#include <gridding/SphFuncVisGridder.h>
#include <gridding/AntennaIllumVisGridder.h>
#include <gridding/WProjectVisGridder.h>

using namespace LOFAR::ACC::APS;
namespace conrad
{
	namespace synthesis
	{

		VisGridderFactory::VisGridderFactory()
		{
		}

		VisGridderFactory::~VisGridderFactory()
		{
		}

		IVisGridder::ShPtr VisGridderFactory::make(
		    const LOFAR::ACC::APS::ParameterSet &parset)
		{
			IVisGridder::ShPtr gridder;
			/// @todo Better handling of string case
			if (parset.getString("gridder")=="WProject")
			{
				double wmax=parset.getDouble("gridder.WProject.wmax", 35000.0);
				int nwplanes=parset.getInt32("gridder.WProject.nwplanes", 64);
				double cutoff=parset.getDouble("gridder.WProject.cutoff", 1e-3);
				int oversample=parset.getInt32("gridder.WProject.oversample", 8);
				int maxSupport=parset.getInt32("gridder.WProject.maxsupport", 256);
				string tablename=parset.getString("gridder.WProject.tablename", "");
				std::cout << "Using W projection gridding "<< std::endl;
				gridder=IVisGridder::ShPtr(new WProjectVisGridder(wmax, nwplanes, cutoff, oversample,
						maxSupport, tablename));
			}
			else if (parset.getString("gridder")=="AntennaIllum")
			{
				double diameter=parset.getDouble("gridder.AntennaIllum.diameter");
				double blockage=parset.getDouble("gridder.AntennaIllum.blockage");
				double wmax=parset.getDouble("gridder.AntennaIllum.wmax", 10000.0);
				int nwplanes=parset.getInt32("gridder.AntennaIllum.nwplanes", 64);
				double cutoff=parset.getDouble("gridder.AntennaIllum.cutoff", 1e-3);
				int oversample=parset.getInt32("gridder.AntennaIllum.oversample", 8);
				int maxSupport=parset.getInt32("gridder.AntennaIllum.maxsupport", 128);
				int maxFeeds=parset.getInt32("gridder.AntennaIllum.maxfeeds", 1);
				string tablename=parset.getString("gridder.AntennaIllum.tablename", "");
				std::cout << "Using Antenna Illumination for gridding function"
				    << std::endl;
				gridder=IVisGridder::ShPtr(new AntennaIllumVisGridder(diameter, blockage, wmax, nwplanes, cutoff, oversample,
						maxSupport, maxFeeds, tablename));
			}
			else if (parset.getString("gridder")=="Box")
			{
				std::cout << "Using Box function for gridding"<< std::endl;
				gridder=IVisGridder::ShPtr(new BoxVisGridder());
			}
			else
			{
				std::cout << "Using spheriodal function for gridding"<< std::endl;
				gridder=IVisGridder::ShPtr(new SphFuncVisGridder());
			}
			return gridder;
		}
	}
}

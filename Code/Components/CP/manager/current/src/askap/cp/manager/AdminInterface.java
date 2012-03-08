/**
 *  Copyright (c) 2011 CSIRO - Australia Telescope National Facility (ATNF)
 *  
 *  Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 *  PO Box 76, Epping NSW 1710, Australia
 *  atnf-enquiries@csiro.au
 * 
 *  This file is part of the ASKAP software distribution.
 * 
 *  The ASKAP software distribution is free software: you can redistribute it
 *  and/or modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the License,
 *  or (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */
package askap.cp.manager;

// Java imports
import java.util.Map;

// ASKAPsoft imports
import Ice.Current;
import org.apache.log4j.Logger;
import askap.interfaces.component.*;

/**
 * This class implements the askap.interfaces.component.IComponent
 * interface, allowing the central processor manager to be administered
 * (i.e. started, shutdown) programmatically.
 */
public class AdminInterface extends askap.interfaces.component._IComponentDisp {
	
	/**
	 * Id for ISeralizable
	 */
	private static final long serialVersionUID = 1L;

	/**
	 * Ice Communicator
	 */
	private Ice.Communicator itsComm;

	/**
	 * Ice Adapter
	 */
	private Ice.ObjectAdapter itsAdapter;

	/**
	 * Component state
	 */
	private ComponentState itsState;

	/**
	 * Reference to the Obs Service object, which provides
	 * the Central Processor observation service.
	 */
	private ObsService itsObsService;
	
	/**
	 * Ice identity of the observation service.
	 */
	private String itsObsServiceName = "CentralProcessorService";

	/** Logger. */
	private static Logger logger = Logger.getLogger(AdminInterface.class.getName());

	/**
	 * Constructor
	 * @param ic	An already initialised Ice communicator for the object
	 * 				to use.
	 */
	public AdminInterface(Ice.Communicator ic) {
		super();
		logger.debug("Creating AdminInterface");
		itsComm = ic;
		itsAdapter = null;
		itsObsService = null;
		itsState = ComponentState.LOADED;
	}

    public void finalize() {
		logger.debug("Destroying AdminInterface");
    }

	/**
	 * 
	 */
    @Override
	public synchronized void activate(Current curr) throws TransitionException {
		if (itsState != ComponentState.STANDBY) {
			throw new TransitionException("Not in STANDBY state");
		}

		Ice.Object object = itsObsService;
		itsAdapter.add(object,
				itsComm.stringToIdentity(itsObsServiceName));

		// Block until service is actually registered
		while (itsAdapter.find(itsComm.stringToIdentity(itsObsServiceName)) == null) {
			try {
				Thread.sleep(100);
			} catch (InterruptedException e) {
				// No consequence
			}
		}

		// Must transition to ONLINE only once all services are activated
		itsState = ComponentState.ONLINE;
	}

	/**
	 * 
	 */
    @Override
	public synchronized void deactivate(Current curr) throws TransitionException {
		if (itsState != ComponentState.ONLINE) {
			throw new TransitionException("Not in ONLINE state");
		}

		// Must transition to STANDBY before deactivating any services
		itsState = ComponentState.STANDBY;
		
        // Stop the server
        itsAdapter.remove(itsComm.stringToIdentity(itsObsServiceName));

        // Block until service is actually unregistered
        while (itsAdapter.find(itsAdapter.getCommunicator().stringToIdentity(itsObsServiceName)) != null) {
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
            	// No consequence
            }
        }
	}

	/**
	 * 
	 */
    @Override
	public ComponentState getState(Current curr) {
		return itsState;
	}

	/**
	 * Returns a string containing the version of the component.
	 */
    @Override
	public String getVersion(Current curr) {
	        Package p = this.getClass().getPackage();
                return p.getImplementationVersion();

	}

	/**
	 * 
	 */
	public void run() {
		logger.debug("Running AdminInterface");
		if (itsComm == null) {
			throw new RuntimeException("ICE Communicator is null");
		}

		itsAdapter = itsComm.createObjectAdapter("AdminAdapter");
		if (itsAdapter == null) {
			throw new RuntimeException("ICE adapter initialisation failed");
		}

		Ice.Object object = this;
		itsAdapter.add(object,
				itsComm.stringToIdentity("CentralProcessorAdmin"));
		
		boolean activated = false;
		while(!activated) {
			final int interval = 5; // seconds
			final String baseWarn = "  - will retry in " + interval + " seconds";
			try {
				itsAdapter.activate();
				activated = true;
			} catch (Ice.ConnectionRefusedException e) {
				logger.warn("Connection refused" + baseWarn); 
			} catch (Ice.NoEndpointException e) {
				logger.warn("No endpoint exception" + baseWarn);
			} catch (Ice.NotRegisteredException e) {
				logger.warn("Not registered exception" + baseWarn);
			}
			try {
				Thread.sleep(interval * 1000);
			} catch (InterruptedException e) {
				// In this rare case this might happen, faster polling is ok
			}
		}

		// Block here so main() can block on this
		itsComm.waitForShutdown();
		logger.info("Stopping AdminInterface");

        itsAdapter.deactivate();
        itsAdapter.destroy();
        logger.info("AdminInterface stopped");
    }

	/**
	 * 
	 */
	@Override
	public synchronized ComponentTestResult[] selfTest(Current curr)
			throws CannotTestException {
		if (itsState != ComponentState.STANDBY) {
			throw new CannotTestException("Not in STANDBY state");
		}

		return new ComponentTestResult[0];
	}

	/**
	 * 
	 */
	@Override
	public synchronized void shutdown(Current curr) throws TransitionException {
		if (itsState != ComponentState.STANDBY) {
			throw new TransitionException("Not in STANDBY state");
		}

		// Must transition to LOADED before destroying any objects
		itsState = ComponentState.LOADED;

		itsObsService = null;
	}

	/**
	 * 
	 */
	@Override
	public synchronized void startup(Map<String, String> config, Current curr)
			throws TransitionException {
		if (itsState != ComponentState.LOADED) {
			throw new TransitionException("Not in UNLOADED state");
		}

		itsObsService = new ObsService(itsComm);

		// Must transition to standby only once all objects are created
		itsState = ComponentState.STANDBY;

	}
}

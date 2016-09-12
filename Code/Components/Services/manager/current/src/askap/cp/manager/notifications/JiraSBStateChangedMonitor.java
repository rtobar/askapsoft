/*
 * Copyright (c) 2016 CSIRO - Australia Telescope National Facility (ATNF)
 * 
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * PO Box 76, Epping NSW 1710, Australia
 * atnf-enquiries@csiro.au
 * 
 * This file is part of the ASKAP software distribution.
 * 
 * The ASKAP software distribution is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 * 
 * @author Daniel Collins <daniel.collins@csiro.au>
 */
package askap.cp.manager.notifications;

import askap.interfaces.schedblock.ObsState;
import askap.util.ParameterSet;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.logging.Level;
import org.apache.log4j.Logger;

/**
 * Implements Scheduling block state change notifications via JIRA.
 * 
 * @author Daniel Collins <daniel.collins@csiro.au>
 */
public final class JiraSBStateChangedMonitor extends SBStateMonitor {

    /**
     * Logger
     */
    private static final Logger logger = Logger.getLogger(JiraSBStateChangedMonitor.class.getName());
	private final ParameterSet config;

	/**
	 * 
	 * @param config
	 */
	public JiraSBStateChangedMonitor(ParameterSet config) {
		this.config = config;
	}

	/**
	 * Issues a scheduling block state changed notification.
	 * 
	 * @param sbid: The scheduling block ID.
	 * @param newState: The new scheduling block state.
	 * @param updateTime: The update timestamp string. 
	 */
	@Override
	public void notify(long sbid, ObsState newState, String updateTime) {
		// TODO: Do I need to load the module? 
        try {
			logger.debug("creating ProcessBuilder");
			// TODO: the following command line is a guess, I need to test this on Galaxy!
            ProcessBuilder pb = new ProcessBuilder(
				"schedblock",
				"annotate",
				Long.toString(sbid),
				"\"Ready for data processing\"");

			// ensure that JIRA authentication environment variables are set
			if (!(pb.environment().containsKey("JIRA_USER") ||
				  pb.environment().containsKey("JIRA_PASSWORD"))) {
				logger.error("JIRA credentials not set");
				// TODO: Appropriate Ice exception for failed notification
				//throw new PipelineStartException(e.getMessage());
			}

			// TODO: do I need to grab stdout and stderr? EG for error parsing?

            Process p = pb.start();
			int exitCode = p.waitFor();
			// TODO: check for success/failure
        } catch (IOException e) {
            logger.error("Failed to issue JIRA notification: " + e.getMessage());
			// TODO: Appropriate Ice exception for failed notification
            //throw new PipelineStartException(e.getMessage());
        } catch (InterruptedException e) {
			logger.error(e);
			// TODO: Appropriate Ice exception for failed notification
            //throw new PipelineStartException(e.getMessage());
		}
	}
	
}

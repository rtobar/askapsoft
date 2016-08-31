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

import Ice.Communicator;
import IceStorm.NoSuchTopic;
import askap.interfaces.schedblock.ISBStateMonitorPrx;
import askap.interfaces.schedblock.ISBStateMonitorPrxHelper;
import askap.interfaces.schedblock.ObsState;
import askap.util.ParameterSet;
import org.apache.log4j.Logger;

/**
 *
 * @author Daniel Collins <daniel.collins@csiro.au>
 */
public final class TestSBStateChangedMonitor extends SBStateMonitor {

	private static final Logger logger = Logger.getLogger(TestSBStateChangedMonitor.class.getName());
	private final ParameterSet config;
    private Communicator communicator;
	private ISBStateMonitorPrx monitor;

	/**
	 * 
	 * @param config
	 * @param communicator
	 * @throws IceStorm.NoSuchTopic
	 */
	public TestSBStateChangedMonitor(
		ParameterSet config,
		Communicator communicator) throws NoSuchTopic {
		this.config = config;
		this.communicator = communicator;

		// Get the names of the topic manager and topic from the parset
		String topicManagerName = config.getString("sbstatemonitor.topicmanager");
		String topicName = config.getString("sbstatemonitor.topic");

		// get a proxy to the IceStorm topic manager
		logger.debug("Getting topic manager proxy: " + topicManagerName);
		Ice.ObjectPrx obj = communicator.stringToProxy(topicManagerName);
		IceStorm.TopicManagerPrx topicManager = IceStorm.TopicManagerPrxHelper.checkedCast(obj);

		// get the topic
		IceStorm.TopicPrx topic = null;
		while (topic == null) {
			try {
				topic = topicManager.retrieve(topicName);
			} catch (IceStorm.NoSuchTopic nst) {
				try {
					topic = topicManager.create(topicName);
				} catch (IceStorm.TopicExists te) {
					// Another client created the topic.
					topic = topicManager.retrieve(topicName);
				}
			}
		}

		// Get the publisher
		Ice.ObjectPrx publisher = topic.getPublisher().ice_oneway();
		monitor = ISBStateMonitorPrxHelper.uncheckedCast(publisher);
	}

	@Override
	public void notify(long sbid, ObsState newState, String updateTime) {
		// publish the notification back 
		// TODO: this is probably a bad idea. Should create a new test interface.
		monitor.changed(sbid, newState, updateTime);
	}
	
}

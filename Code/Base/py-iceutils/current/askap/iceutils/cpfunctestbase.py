# Copyright (c) 2016 CSIRO
# Australia Telescope National Facility (ATNF)
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# PO Box 76, Epping NSW 1710, Australia
# atnf-enquiries@csiro.au
#
# This file is part of the ASKAP software distribution.
#
# The ASKAP software distribution is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the License
# or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA.
#
"""
SDP Functional Test Base Class
------------------------------
"""

import os

# import Ice, IceStorm
from askap.slice import CP
from askap.interfaces.cp import ICPFuncTestReporter
#shouldn't need this
# from askap.interfaces.schedblock import ObsState

from .icesession import IceSession
from .server import Server

__all__ = ["CPFuncTestBase"]

# logger = logging.getLogger(__name__)


class FeedbackService(ICPFuncTestReporter):
    """ Service implementation of the askap.interfaces.cp.ICPFuncTestReporter
    Ice interface.

    Used for feedback from processes under test to the test driver.
    """
    def __init__(self):
        self.history = []
        """The method call history. Each entry is a (str, dict) tuple. The str
        gives the method name, and the dict gives the method args as name:value
        pairs.
        """

    def clear_history(self):
        """ Clears the history list. """
        self.history = []

    def sbStateChangedNotification(self, sbid, obsState, current=None):
        """Scheduling block state changed notification feedback."""
        self.history.append(
            ('sbStateChangedNotification', {
                'sbid': sbid,
                'obsState': obsState,
            }))


class FuncTestServer(Server):
    """ Ice Server for hosting the FeedbackService. """
    def __init__(self, feedback_service, comm):
        self.feedback_service = feedback_service
        super(FuncTestServer, self).__init__(
            comm,
            configurable=False,
            retries=10,
            monitoring=False)

    def initialize_services(self):
        """Template method for initialising specific services."""
        self.add_service('FuncTestReporter', self.feedback_service)


class CPFuncTestBase(object):
    """Base class for ASKAP CP/SDP functional test classes."""
    def __init__(self):
        """Initialise the CPFuncTestBase instance"""
        self.ice_session = None
        self.server = None
        self.server_thread = None
        self.feedback_service = None

    def setUp(self):
        """Test fixture set up.

        Note that the ICE_CONFIG and TEST_DIR environment variables should be
        set prior to calling this method from the subclass::

            os.environ["ICE_CONFIG"] = "config-files/ice.cfg"
            os.environ['TEST_DIR'] = 'test_sbstate_changed'
            super(TestSBStateChanged, self).setUp()

        """
        self.ice_session = IceSession(
            os.path.expandvars("$TEST_DIR/applications.txt"),
            cleanup=True)
        try:
            self.ice_session.start()
            self.__setup_feedback_service()
        except Exception as ex:
            self.ice_session.communicator.destroy()
            self.ice_session.terminate()
            raise ex

    def tearDown(self):
        """ Test fixture tear down """
        self.shutdown()

    def shutdown(self):
        """ Shut down the Ice session and servers """
        self.ice_session.communicator.destroy()
        self.ice_session.terminate()
        self.ice_session = None

    def __setup_feedback_service(self):
        """ Initialises the functional test feedback service """
        self.feedback_service = FeedbackService()
        self.server = FuncTestServer(
            self.feedback_service,
            self.ice_session.communicator)
        self.server.setup_services()

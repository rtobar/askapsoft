//# tWorkersDesc.cc: Test program for class WorkersDesc
//#
//# Copyright (C) 2007
//#
//# $Id$

#include <mwcommon/WorkersDesc.h>
#include <mwcommon/ConradError.h>
#include <ostream>

using namespace conrad::cp;
using namespace std;

void doIt1()
{
  // First define the cluster.
  // File systems can be accessed from multiple nodes.
  ClusterDesc cl;
  cl.setName ("cl");
  NodeDesc node0;
  node0.setName ("node0");
  node0.addFileSys ("fs0");
  node0.addFileSys ("fs1");
  cl.addNode (node0);
  NodeDesc node1;
  node1.setName ("node1");
  node1.addFileSys ("fs1");
  node1.addFileSys ("fs2");
  cl.addNode (node1);
  NodeDesc node2;
  node2.setName ("node2");
  node2.addFileSys ("fs0");
  node2.addFileSys ("fs1");
  node2.addFileSys ("fs2");
  cl.addNode (node2);
  WorkersDesc wdesc(cl);
  // Now define all workers which can perform 2 work types.
  vector<int> wtypes(2);
  wtypes[0] = 0;
  wtypes[1] = 1;
  wdesc.addWorker (0, "node0", wtypes);
  wdesc.addWorker (1, "node1", wtypes);
  wdesc.addWorker (2, "node2", wtypes);
  // Now find a worker for a specific task on a file system.
  int worker;
  worker = wdesc.findWorker (0, "fs0");
  CONRADASSERT (worker == 0);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (0, "fs2");
  CONRADASSERT (worker == 1);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (0, "fs1");
  CONRADASSERT (worker == 2);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (0, "fs2");
  CONRADASSERT (worker == 1);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (0, "fs1");
  CONRADASSERT (worker == 0);
  worker = wdesc.findWorker (0, "fs0");
  CONRADASSERT (worker == 0);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (0, "fs0");
  CONRADASSERT (worker == 2);
  wdesc.incrLoad (worker);
  wdesc.incrLoad (0);
  wdesc.incrLoad (1);
  worker = wdesc.findWorker (1, "");
  CONRADASSERT (worker == 2);
  wdesc.incrLoad (worker);
  CONRADASSERT (wdesc.findWorker (2, "") == -1);
  CONRADASSERT (wdesc.findWorker (0, "fs3") == -1);
}

void doIt2()
{
  // First define the cluster.
  // FIle systems can be accessed from a single node.
  ClusterDesc cl;
  cl.setName ("cl");
  NodeDesc node0;
  node0.setName ("node0");
  node0.addFileSys ("fs0");
  cl.addNode (node0);
  NodeDesc node1;
  node1.setName ("node1");
  node1.addFileSys ("fs1");
  cl.addNode (node1);
  NodeDesc node2;
  node2.setName ("node2");
  node2.addFileSys ("fs2");
  cl.addNode (node2);
  WorkersDesc wdesc(cl);
  // Now define all workers which can perform 2 work types.
  vector<int> wtypes(2);
  wtypes[0] = 0;
  wtypes[1] = 1;
  wdesc.addWorker (0, "node0", wtypes);
  wdesc.addWorker (1, "node1", wtypes);
  wdesc.addWorker (2, "node2", wtypes);
  // Now find a worker for a specific task on a file system.
  int worker;
  worker = wdesc.findWorker (0, "fs0");
  CONRADASSERT (worker == 0);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (0, "fs0");
  CONRADASSERT (worker == 0);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (0, "fs2");
  CONRADASSERT (worker == 2);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (0, "fs1");
  CONRADASSERT (worker == 1);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (1, "");
  CONRADASSERT (worker == 1);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (1, "");
  CONRADASSERT (worker == 2);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (1, "");
  CONRADASSERT (worker == 0);
  wdesc.incrLoad (worker);
  CONRADASSERT (wdesc.findWorker (2, "") == -1);
  CONRADASSERT (wdesc.findWorker (0, "fs4") == -1);
}

int main()
{
  try {
    doIt1();
    doIt2();
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  cout << "OK" << endl;
  return 0;
}

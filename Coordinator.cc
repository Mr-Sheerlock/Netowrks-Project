
#include "Coordinator.h"

Define_Module(Coordinator);

void Coordinator::initialize()
{
  // TODO - Generated method body
  const char *DataPath = "coordinator.txt";
  fstream DataFile(DataPath, std::ios_base::in);
  int NodeNumber, StartTime;
  DataFile >> NodeNumber >> StartTime;
  cMessage *msg = new cMessage();
  msg->setName("Start");
  if (NodeNumber == 0)
  {
    sendDelayed(msg,StartTime, "out0"); return;
  }
  
  sendDelayed(msg,StartTime, "out1");
  
}

void Coordinator::handleMessage(cMessage *msg)
{
  // TODO - Generated method body
}

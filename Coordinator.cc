//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

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

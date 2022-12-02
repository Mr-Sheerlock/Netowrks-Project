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

#include "Node.h"

Define_Module(Node);


void Node::ReadLine(){
    MsgContent.clear();
    string temp;
    getline(DataFile,temp);
    Errorbits = temp.substr(0,4);
    MsgContent= temp.substr(4);
    cout<< "Node I read " << Errorbits << " " << MsgContent<<endl;
}


void Node::initialize()
{
    cout << "Initializing " << this->getName() << endl;
    id=this->getName()[4]; // id is if it's node 0 or 1 
    DataPath="input.txt";
    DataPath.insert(5,1,id);
    DataFile.open(DataPath.c_str(), std::ios_base::in);
}

void Node::handleMessage(cMessage *msg)
{

    CustomMsg *packet = dynamic_cast<CustomMsg *>(msg);
    if (packet == nullptr)
    {
        // coordinator's first move
        // cout << "recieved @ node " << this->getName()<<endl;

        ReadLine();
        ReadLine();
        return; 
    }
    // other node
    
}


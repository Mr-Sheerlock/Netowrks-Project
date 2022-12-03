
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
        cout <<"MY WS IS "<< par("WS").intValue()<<endl;
        return; 
    }
    // other node
    
}


#include "Node.h"

Define_Module(Node);

//for debugging purposes later
bitset<4> reverseBitset(bitset<4> mybits)
{
    bitset<4> res;
    for (int i = 0; i < 4; i++)
    {
        res[i] = mybits[3 - i];
    }
    return res;
}

void Node::ReadFile()
{
    string temp, err, msg ,out;
    DataFile.open(DataPath.c_str(), std::ios_base::in);
    OutFile.open("output.txt",std::ios_base::out);
    while (getline(DataFile, temp))
    {
        err = temp.substr(0, 4);
        bitset<4> errorbits = bitset<4>(err);
        msg = temp.substr(4);

        double now= simTime().dbl();
        out= "At time ";
        OutFile.precision(3);
        OutFile<<out <<fixed<<now;
        out=" Node";out+=id;out+=" , Introducing error with code= " +errorbits.to_string();
        OutFile<<out<<endl;
        
        Messages.push_back(msg);
        Errorbits.push_back(errorbits);
    }
    DataFile.close();
}

// IMPORTANT NOTE: If bits are 1010, then bits[0]=0, bits[1]=1.... etc ie. bit[x] x starts from the least significant bit
void Node::ErrSend(string Message, bitset<4> ErrBits, bool dupdelaytime = 0)
{
    float delay = PT + TD;
    if (dupdelaytime)
        delay += DD;
    if (ErrBits[2])
        return; // loss, do nothing // You might also want to modify the function to reutrn false
    if (ErrBits[1])
    {                   // duplication
        ErrBits[1] = 0; // how to print duplicate ?
        ErrSend(Message, ErrBits, 1);
    }
    if (ErrBits[3])
    { // call modify
    }
    if (ErrBits[0])
    { // change delay variable
        delay += ED;
    }
    // TODO: Send and Print the message
}

void Node::initialize()
{
    WS = par("WS").intValue();
    TO = par("TO").doubleValue();
    PT = par("PT").doubleValue();
    TD = par("TD").doubleValue();
    ED = par("ED").doubleValue();
    DD = par("DD").doubleValue();
    LP = par("LP").doubleValue();
    cout << "Initializing " << this->getName() << endl;
    id = this->getName()[4]; // id is if it's node 0 or 1
    DataPath = "input.txt";
    DataPath.insert(5, 1, id);
    
}

void Node::handleMessage(cMessage *msg)
{

    CustomMsg *packet = dynamic_cast<CustomMsg *>(msg);
    if (packet == nullptr)
    {
        // coordinator's first move
        ReadFile();
        return;
    }
    // other node
    string payload = packet->getM_Payload();
    if (!payload.length())
    {
        // control message
        if (packet->getM_Ack())
        {
            // ack
        }
        else
        {
            //nack

        }
    }
}

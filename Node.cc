#include "Node.h"

Define_Module(Node);

char GetParityByte(string Payload)
{
    int PayloadLength = Payload.size();
    vector<std::bitset<8>> CharVec(PayloadLength);
    // Add character bytes in the vector
    for (int i = 0; i < PayloadLength; i++)
    {
        bitset<8> CurrentChar(Payload[i]);
        CharVec[i] = CurrentChar;
    }
    // First set the parity as the first character in the vector
    bitset<8> Parity = CharVec[0];

    // loop to calculate the parity byte
    for (int i = 1; i < PayloadLength; i++)
    {
        Parity = Parity ^ CharVec[i];
    }
    return (char)Parity.to_ulong();
}

bool CheckError(string Payload, char ParityByte)
{
    bitset<8> ReceivedParity(ParityByte);
    int PayloadLength = Payload.size();
    vector<std::bitset<8>> CharVec(PayloadLength);
    // Add character bytes in the vector
    for (int i = 0; i < PayloadLength; i++)
    {
        bitset<8> CurrentChar(Payload[i]);
        CharVec[i] = CurrentChar;
    }
    // First set the parity as the first character in the vector
    bitset<8> CalculatedParity = CharVec[0];
    // loop to calculate the parity byte
    for (int i = 1; i < PayloadLength; i++)
    {
        CalculatedParity = CalculatedParity ^ CharVec[i];
    }
    if (CalculatedParity == ReceivedParity)
    {
        return true;
    }
    return false;
}

// for debugging purposes later
bitset<4> reverseBitset(bitset<4> mybits)
{
    bitset<4> res;
    for (int i = 0; i < 4; i++)
    {
        res[i] = mybits[3 - i];
    }
    return res;
}

void Node::LogRead(bitset<4> const &errorbits)
{
    string out;
    OutFile.open("output.txt", std::ios_base::app);
    double now = simTime().dbl();
    out = "At time ";
    OutFile.precision(3);
    OutFile << out << fixed << now;
    out = " Node";
    out += id;
    out += " , Introducing error with code= " + errorbits.to_string();
    OutFile << out << endl;
    OutFile.close();
}

void Node::LogTransmissionOrRecieval(bool Transmitting, int seq_num, string payload, char Trailer, int Modified = -1, bool Lost = 0, int Duplicate = 0, int delay = 0)
{

    string out, temp;
    bitset<8> trailer(Trailer);
    OutFile.open("output.txt", std::ios_base::app);
    double now = simTime().dbl();
    out = "At time ";
    OutFile.precision(3);
    OutFile << out << fixed << now;
    temp = Transmitting ? "sent" : "received";
    out = " Node";
    out += id;
    out += " , " + temp + " frame with seq_num= " + to_string(seq_num);
    out += " and payload= " + payload;
    out += " and trailer= " + trailer.to_string();
    out += " , Modified " + to_string(Modified);
    temp = Lost ? "Yes" : "No";
    out += " , Lost " + temp;
    out += ", Duplicate " + to_string(Duplicate);
    out += " , Delay " + to_string(delay);
    OutFile << out << endl;
    OutFile.close();
}

void Node::LogTimeout(int seq_num)
{

    string out;
    OutFile.open("output.txt", std::ios_base::app);
    double now = simTime().dbl();
    out = "Timeout event at time ";
    OutFile.precision(3);
    OutFile << out << fixed << now;
    out = " at Node";
    out += id;
    out += " for frame with seq_num= " + to_string(seq_num);
    OutFile << out << endl;
    OutFile.close();
}

void Node::LogControl(int seq_num, bool Ack = 1, bool Lost = 0)
{

    string out, temp;
    OutFile.open("output.txt", std::ios_base::app);
    double now = simTime().dbl();
    out = "At time ";
    OutFile.precision(3);
    OutFile << out << fixed << now;
    temp = Ack ? "ACK" : "NACK";
    out = " Node";
    out += id;
    out += ", Sending " + temp;
    out += " with number " + to_string(seq_num);
    temp = Lost ? "Yes" : "No";
    out += " , loss " + temp;
    OutFile << out << endl;
    OutFile.close();
}

void Node::ReadFile()
{
    string temp, err, msg;
    DataFile.open(DataPath.c_str(), std::ios_base::in);

    while (getline(DataFile, temp))
    {
        err = temp.substr(0, 4);
        bitset<4> errorbits = bitset<4>(err);
        msg = temp.substr(4);

        // we can put this part in a function and call them whenever we like
        LogRead(errorbits);
        Messages.push_back(msg);
        Errorbits.push_back(errorbits);
    }
    DataFile.close();
}

void Node::SendData(float delay, int modify, bool lost, int duplicate)
{
    CustomMsg *msg = new CustomMsg();
    msg->setM_Header(next_frame_to_Send);
    msg->setM_Payload(Messages[nFramesAcked + next_frame_to_Send]);
    msg->setM_Trailer(GetParityByte(Messages[nFramesAcked + next_frame_to_Send]));
    msg->setM_FrameType(0);
    LogTransmissionOrRecieval(1, next_frame_to_Send, Messages[nFramesAcked + next_frame_to_Send], msg->getM_Trailer(),
                              modify, lost, duplicate, delay);
    if (lost)
        return;

    scheduleAt(simTime() + delay, msg);
}

// IMPORTANT NOTE: If bits are 1010, then bits[0]=0, bits[1]=1.... etc ie. bit[x] x starts from the least significant bit

void Node::ErrSend(string Message, bitset<4> ErrBits, bool dupdelaytime = 0)
{
    float delay = PT + TD;
    int modify = -1;
    int duplicate = 0;
    if (dupdelaytime)
    {
        delay += DD;
        duplicate = 2;
    }

    // if (ErrBits[2])
    //     return; // loss, do nothing // You might also want to modify the function to reutrn false
    if (ErrBits[1])
    {                   // duplication
        ErrBits[1] = 0; // how to print duplicate ?
        duplicate;
        ErrSend(Message, ErrBits, 1);
    }
    if (ErrBits[3])
    { // call modify IMPORTANT: modify must return the
      // modify = ;
    }
    if (ErrBits[0])
    { // change delay variable
        delay += ED;
    }
    // TODO: Send and Print the message
    SendData(delay, modify, ErrBits[2], duplicate);
}

void Node::initialize()
{
    // clear the output file
    OutFile.open("output.txt", std::ios_base::out);
    OutFile.close();
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
    Timeouts = vector<CustomMsg *>(WS + 1, NULL);
}

void inc(int &seq_num)
{
    seq_num = (seq_num + 1) % (WS + 1);
}

void Node::handleMessage(cMessage *msg)
{

    CustomMsg *packet = dynamic_cast<CustomMsg *>(msg);
    if (packet == nullptr)
    {
        // coordinator's first move
        ReadFile();
        // IMPORTANT TODO:send something to the other node
        //  timeout= new CustomMsg("Timeout");
        //  timeout->setM_Header(4);
        //  scheduleAt(simTime()+3, timeout);
        //  // cancelEvent(timeout);
        return;
    }
    else
    {
        // other node
        string payload = packet->getM_Payload();
        if (!payload.length())
        {
            // control message: either ack or timeout
            if (packet->getM_Ack() == 2)
            {
                // Timeout
                next_frame_to_Send = packet->getM_Header();
                nBuffered = 0;
                delete Timeouts[packet->getM_Header()];
                Timeouts[packet->getM_Header()] = NULL;
                for (int i = 0; i < Timeouts.size(); i++)
                {
                    if (Timeouts[i])
                    {
                        cancelEvent(Timeouts[i]);
                        Timeouts[i] = NULL;
                    }
                }
            }
            if (packet->getM_Ack())
            {
                // ack
            }
            else
            {
                // nack
            }
        }
    }
    // Message
}

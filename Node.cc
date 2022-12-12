#include "Node.h"
Define_Module(Node);

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
    PreviousPT = 0;
    //Sender
    Timeouts = vector<cMessage*> (WS + 1);
    for (int i = 0; i < WS+1; i++)
    {
        Timeouts[i] = new cMessage();
        Timeouts[i]->setName(to_string(i).c_str());
    }
    next_frame_to_Send = 0;
    Ack_Expected = 0;
    nBuffered = 0;
    nFramesAcked = 0;
    Msgsread=0;
    //Receiver
    frame_expected = 0;
}

char Node:: GetParityByte(string Payload)
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

bool Node:: CheckError(string Payload, char ParityByte)
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

void Node::LogRead(bitset<4> const &errorbits)
{
    string out;
    OutFile.open("output.txt", std::ios_base::app);
    double now = simTime().dbl() >=PreviousPT ? simTime().dbl() :PreviousPT ;
    out = "At time [";
    OutFile.precision(3);
    OutFile << out << fixed << now;
    out = "] Node";
    out += id;
    out += " , Introducing channel error with code= [" + errorbits.to_string();
    OutFile << out << "]" << endl;
    OutFile.close();
}

void Node::LogTransmissionOrRecieval(bool Transmitting, int seq_num, string payload, char Trailer, int Modified = -1, bool Lost = 0, int Duplicate = 0, int delay = 0)
{

    string out, temp;
    bitset<8> trailer(Trailer);
    OutFile.open("output.txt", std::ios_base::app);
    // double now = simTime().dbl() +PreviousPT;
    double now = PreviousPT;
    out = "At time [";
    OutFile.precision(3);
    OutFile << out << fixed << now << "]";
    temp = Transmitting ? "sent" : "received";
    out = " Node";
    out += id;
    out += " , " + temp + " frame with seq_num= [" + to_string(seq_num);
    out += "] and payload= [" + payload;
    out += "] and trailer= [" + trailer.to_string();
    out += "] , Modified [" + to_string(Modified);
    temp = Lost ? "Yes" : "No";
    out += "] , Lost [" + temp;
    out += "], Duplicate [" + to_string(Duplicate);
    out += "] , Delay [" + to_string(delay);
    out+="]";
    OutFile << out << endl;
    OutFile.close();
}

void Node::LogTimeout(int seq_num)
{

    string out;
    OutFile.open("output.txt", std::ios_base::app);
    double now = simTime().dbl();
    out = "Timeout event at time [";
    OutFile.precision(3);
    OutFile << out << fixed << now<< "]";
    out = " at Node [";
    out += id;
    out += "] for frame with seq_num= [" + to_string(seq_num);
    OutFile << out << "]" << endl ;
    OutFile.close();
}

void Node::LogControl(int seq_num, bool Ack = 1, bool Lost = 0)
{

    string out, temp;
    OutFile.open("output.txt", std::ios_base::app);
    double now = simTime().dbl();
    out = "At time [";
    OutFile.precision(3);
    OutFile << out << fixed << now<<"]";
    temp = Ack ? "ACK" : "NACK";
    out = " Node [";
    out += id;
    out += "], Sending [" + temp;
    out += "] with number [" + to_string(seq_num);
    temp = Lost ? "Yes" : "No";
    out += "] , loss [" + temp;
    OutFile << out <<"]"<< endl ;
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
        Messages.push_back(msg);
        Errorbits.push_back(errorbits);
    }
    DataFile.close();
}

int Node::ModifyMsg(string &Payload)
{
    // We now need to convert the string into bitset
    vector<bitset<8>> Vmsg(Payload.size()); // Vector containing the bitset of each char in the payload
    // Filling the vector
    for (int i = 0; i < Payload.size(); ++i)
    {
        bitset<8> CharPayload(Payload[i]); // bitset of one char from the payload
        Vmsg[i] = CharPayload;             // The MSB has index 0, LSB has index size-1
    }

    //    cout << endl;

    // Now, we want to modify the payload by inverting any 1 bit from it
    int ModifiedByte = uniform(0, 1) * Payload.size(); // generates a random integer between 0 and size-1 inclusive
                                                       // This is the char that will be modified
    int ModifiedBit = uniform(0, 1) * 7;               // This is the bit that will be modified in that char (0 indexing is used)
    bitset<8> error(128 / pow(2, ModifiedBit));
    // 2^0 = 0000 0001
    // 2^1 = 0000 0010
    // 2^2 = 0000 0100
    //
    // 2^7 = 1000 0000
    // 128 divided pow will reverse the indexing order, such that if the ModifiedBit = 0
    // the first element (index 0, or MSB) will be modified

    Vmsg[ModifiedByte] = Vmsg[ModifiedByte] ^ error; // modifying that bit

    // Now, Vmsg contains the modified payload, we need to update the
    // CustomMessage's payload
    for (int i = 0; i < Payload.size(); ++i)
    {
        Payload[i] = (char)Vmsg[i].to_ulong();
    }
    return ModifiedByte * 8 + ModifiedBit;
}

void Node::FramingMsg(string &Payload)
{
    string ModifiedPayload = "";

    for (int i = 0; i < Payload.size(); ++i)
    {
        if (Payload[i] == '$' || Payload[i] == '/') // We need to add a '/' before the character
        {
            ModifiedPayload.push_back('/'); // appending a '/' to the end of the string
        }
        ModifiedPayload.push_back(Payload[i]); // appending the next character
    }

    Payload = ModifiedPayload;
}

void Node::inc(int& seq_num)
{
    seq_num = (seq_num + 1) % (WS + 1);
}

int Node::dec(int seq_num)
{
    return ((seq_num + WS) % (WS+1));
}

void Node:: StartTimer(int SeqNum, float delay)
{
    EV<< "Setting Timer @ SeqNum " << SeqNum <<endl;
    scheduleAt(simTime() + delay + TO, Timeouts[SeqNum]);
}

int Node::ErrorHandling(string &Message, bitset<4> ErrorBits, float &TotalDelay, float &PTDelay)
{
    //TODO
    //check for errors:
    PTDelay = CalculatePT();
    TotalDelay = PTDelay + TD;
    if(ErrorBits[2] == 1)   //Lost
    {
        return 0;
    }
    if(ErrorBits[0] == 1)   //delay error
    {
        TotalDelay += ED;
    }
    if(ErrorBits[3] == 1)    //Modify
    {
        int BitModified = ModifyMsg(Message);
    }
    if(ErrorBits[1] == 1)   //Duplication
    {
        return 1;
    }
    return -1;
}

void Node::SendData(string Message, bitset<4> ErrorBits)
{
    EV << "SimTime is " <<simTime()<<endl;
    CustomMsg *msg = new CustomMsg();
    msg->setM_FrameType(0);
    msg->setM_Header(next_frame_to_Send);
    FramingMsg(Message);
    char trailer=GetParityByte(Message);
    msg->setM_Trailer(trailer);
    float TotalDelay, PTDelay; 
    int Error = ErrorHandling(Message, ErrorBits, TotalDelay, PTDelay);
    msg->setM_Payload(Message.c_str());
    float delay= ErrorBits[0]? ED: 0;
    StartTimer(next_frame_to_Send, PTDelay);

    LogTransmissionOrRecieval(1,next_frame_to_Send,Message,trailer,ErrorBits[3],ErrorBits[2],ErrorBits[1],delay );
    if(ErrorBits[1]) LogTransmissionOrRecieval(1,next_frame_to_Send,Message,trailer,ErrorBits[3],ErrorBits[2],2,delay );
    
    if(Error == 0) //lost
    {
        return;
    }
    else if(Error == 1) //duplicates
    {
        sendDelayed(msg->dup(), TotalDelay + DD, "out");
    }

    
    sendDelayed(msg, TotalDelay, "out");
    //start timer
}

float Node::CalculatePT()
{
    if(simTime() >= PreviousPT)
    {
        PreviousPT = simTime().dbl() + PT;
        return PT;
    }
    else
    {
        float TimeLeft = PreviousPT - simTime().dbl();
        PreviousPT = simTime().dbl() + PT + TimeLeft;
        // EV<<"returnd val is " <<PT+TimeLeft<<endl;
        return TimeLeft + PT;
    }
}

void Node::SendControlMsg(int Frame_Type, int AckNum)
{
    CustomMsg *ControlMsg = new CustomMsg();
    ControlMsg->setM_Ack(AckNum);
    ControlMsg->setM_FrameType(Frame_Type);
    float temp =uniform(0, 1) * 100;
    LogControl(AckNum,1,temp>=LP);
    if ( temp>= LP)
    {
        float delay = CalculatePT() + TD;
        sendDelayed(ControlMsg, delay, "out");
    }
    else
    {
        return;
    }
}

void Node::Protocol(Events CurrentEvent, int SeqNumber)
{
    string CurrentMessage;
    bitset<4> CurrentErrorBits;
    while(true)
    {
        switch(CurrentEvent)
        {
            case Read:
                //read from message buffer string and error bits
                CurrentMessage = Messages[nBuffered + nFramesAcked];
                CurrentErrorBits = Errorbits[nBuffered + nFramesAcked];
                if(nBuffered + nFramesAcked>=Msgsread){
                    //if first time to read
                    Msgsread++;
                    LogRead(CurrentErrorBits);
                }
                EV<< "Sending ";
                EV<<"Message = "<<CurrentMessage<<", bits = "<<CurrentErrorBits<<endl;
                SendData(CurrentMessage, CurrentErrorBits);
                //increase nbuffer
                nBuffered ++;
                //increment next frame to send
                inc(next_frame_to_Send);
                break;
            case Ack:
                //if seq == ack expected
                if(SeqNumber == Ack_Expected)
                {
                    //stop timer
                    EV<< " AckExpected Received,  " <<endl;
                    EV<< "Stopping timer @ Seqnum " <<SeqNumber<<endl;
                    cancelEvent(Timeouts[SeqNumber]);
                    nBuffered--;
                    // inc(next_frame_to_Send);
                    inc(Ack_Expected);
                    EV<< "AckExpected Now is" <<Ack_Expected<<endl;
                    nFramesAcked++;
                    
                }
                break;
            case Timeout_Nack:
                //Retransmission
                EV << " Timeout yaba  @ SeqNum" << SeqNumber <<endl;
                 for (int i = 0; i < Timeouts.size(); i++)
                {
                    if (Timeouts[i])
                    {
                        cancelEvent(Timeouts[i]);
                    }
                }
                next_frame_to_Send = Ack_Expected;
                CurrentMessage = Messages[nFramesAcked];
                CurrentErrorBits.reset();     //0000 no error
                SendData(CurrentMessage, CurrentErrorBits);
                nBuffered = 1;
                inc(next_frame_to_Send);
                break;
        }
        //check if window buffer is full
        if(nBuffered < WS)
        {
            CurrentEvent = Read;
        }
        else
        {
            return;
        }
        //check if there is more messages to send
        if( nBuffered+nFramesAcked == Messages.size())
        {
            return;
        }
    }
}


void Node::handleMessage(cMessage *msg)
{

    CustomMsg *packet = dynamic_cast<CustomMsg *>(msg);
    if(packet == nullptr)
    {
        // cout<<"Node " <<id<<" received CMessage";
        // EV<<"Node " <<id<<" received CMessage";
        string msg_content = msg->getName();
        if(msg_content == "Start")
        {
            // coordinator's first move
            ReadFile();
            Protocol(Read, 0);
        }
        else
        {
            //timeout
            int Timeout_seq_num = atoi(msg_content.c_str());
            Protocol(Timeout_Nack, Timeout_seq_num);
        }
    }
    else
    {
        //  cout<<"Node" <<id<<"received CusMessage";
        // EV<<"Node" <<id<<"received CusMessage";
        // EV<<"Frame type is " << packet->getM_FrameType();
        // cout<<"Frame type is " << packet->getM_FrameType();
        if(packet->getM_FrameType() == 0)   //data
        {
            //Receiver
            int Type = 1;
            int Msg_seq_number = packet->getM_Header();
            //check if frame expected
            if(Msg_seq_number == frame_expected)
            {
                char Parity = packet->getM_Trailer();
                string Payload = packet->getM_Payload();
                if(CheckError(Payload, Parity))
                {
                    //Send ACK
                    Type = 1;
                    inc(frame_expected);
                }
                else
                {
                    //Send NACK
                    Type = 2;
                }
            }

            int AckNum = dec(frame_expected);
            EV << "Sending Ack on AckNum " << AckNum<<endl;
            SendControlMsg(Type, AckNum);
        }
        else if(packet->getM_FrameType() == 1)  //ack
        {
            //Sender
            EV<<"Received an Ack"<<endl;
            int Ack_seq_number = packet->getM_Ack();
            Protocol(Ack, Ack_seq_number);
        }
        else if(packet->getM_FrameType() == 2)  //nack
        {
            //Sender
            int Nack_seq_number = packet->getM_Ack();
            Protocol(Timeout_Nack, Nack_seq_number);
        }
    }
}

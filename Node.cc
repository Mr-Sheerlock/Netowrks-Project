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
        Messages.push_back(msg);
        Errorbits.push_back(errorbits);
    }
    DataFile.close();
}

// IMPORTANT NOTE: If bits are 1010, then bits[0]=0, bits[1]=1.... etc ie. bit[x] x starts from the least significant bit
// Another important note: inc(frameToSend) expected must be outside this function)
//potential #TODO: you might want to add a bool Buffered that gets multiplied by the PT and also propagated to SendData
void Node::ErrSend(bool duplicate = 0)
{
    float delay = PT + TD;
    int modify = -1;                                     // variable used for printing purposes
    int duplicateVersion = 0;                            // variable used for printing purposes
    
    string Message = Messages[nFramesAcked + nBuffered]; // Fetches message from vector
    bitset<4> ErrBits = Errorbits[nFramesAcked + nBuffered];

    
    FramingMsg(Message); // Applies byte stuffing

    if (ErrBits[1])
    { // duplication
        // ErrBits[1] = 0;
        duplicateVersion = 1;
        if (duplicate)
        {
            delay += DD;
            duplicateVersion = 2;
        }
        if (!duplicate) // to prevent inf recursion
            ErrSend(1);
    }
    char parity = GetParityByte(Message);
    if (ErrBits[3])
    { // call modify IMPORTANT: modify must return the index of modified bit
        modify = ModifyMsg(Message);
    }
    if (ErrBits[0])
    { // change delay variable
        delay += ED;
    }
    SendData(Message, parity,delay, modify, ErrBits[2], duplicateVersion);
}

void Node::SendData(string Msgg, char Parity, float delay, int modify, bool lost, int duplicate)
{
    CustomMsg *msg = new CustomMsg();

    // a message to signal the end of the processing
    CustomMsg *processingFin = new CustomMsg("Processing Finished");
    processingFin->setM_Header(1);
    scheduleAt(simTime() + PT, processingFin);

    msg->setM_Header(next_frame_to_Send);
    //    string Msgg=Messages[nFramesAcked + next_frame_to_Send];
    msg->setM_Payload(Msgg.c_str());
    msg->setM_Trailer(Parity);
    msg->setM_FrameType(0);
    LogTransmissionOrRecieval(1, next_frame_to_Send, Msgg, Parity,
                              modify, lost, duplicate, delay);
    if (!duplicate || duplicate == 1)
    {
        //sending timeout
        Timeouts[next_frame_to_Send] = new CustomMsg("Timeout");
        CustomMsg *timeout = Timeouts[next_frame_to_Send];
        timeout->setM_Header(0);
        timeout->setM_Ack(next_frame_to_Send);
        scheduleAt(simTime() + PT + TO, timeout);
    }
    if (lost)
        return;

    sendDelayed(msg, simTime() + delay, "out");
}

int Node::ModifyMsg(string &Payload)
{
    //    cout << "ModifyMSG Function Called" << endl;
    //    cout << "Msg payload content: " << endl << msg->getM_Payload() << endl;

    //    string Payload = msg->getM_Payload();   // returns payload from the custom message

    //    cout << "String Payload: " << endl << Payload << endl;

    // We now need to convert the string into bitset

    vector<bitset<8>> Vmsg(Payload.size()); // Vector containing the bitset of each char in the payload

    //    cout << "Bitset vector: " << endl;

    // Filling the vector
    for (int i = 0; i < Payload.size(); ++i)
    {
        bitset<8> CharPayload(Payload[i]); // bitset of one char from the payload
        Vmsg[i] = CharPayload;             // The MSB has index 0, LSB has index size-1

        //        cout << Vmsg[i];
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

    //    cout << "Modified Bitset vector: " << endl;

    // Now, Vmsg contains the modified payload, we need to update the
    // CustomMessage's payload
    for (int i = 0; i < Payload.size(); ++i)
    {
        Payload[i] = (char)Vmsg[i].to_ulong();

        //        cout << Vmsg[i];
    }

    //    cout << endl;

    //    cout << "String Payload after modification: " << endl << Payload << endl;
    //    cout << "Modified Byte: " << ModifiedByte << ", Modified Bit: " << ModifiedBit << endl;
    //    cout << "Returned value: " << ModifiedByte * 8 + ModifiedBit << endl;

    //    msg->setM_Payload(Payload.c_str());     // updating message payload

    //    cout << "Msg payload: " << endl << msg->getM_Payload() << endl;

    return ModifiedByte * 8 + ModifiedBit;
}

void Node::FramingMsg(string &Payload)
{
    //    string Payload = msg->getM_Payload();
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
    //    msg->setM_Payload(ModifiedPayload.c_str());     // updating the message payload
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
    next_frame_to_Send = 0;
    nBuffered = 0;
    nFramesAcked = 0;
    frame_expected = 0;
}

// you might wanna add a check in the case of the end of the Messages Vector
void Node::inc(int &seq_num)
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
        if(Messages.size()==0){
            return; //mat3ml4 7ag
        }
        //  timeout= new CustomMsg("Timeout");
        //  timeout->setM_Header(4);
        //  scheduleAt(simTime()+3, timeout);
        //  // cancelEvent(timeout);

        ErrSend();
        nBuffered++;
        if (!(nFramesAcked + nBuffered < Msgsread))
        {
            Msgsread++;
            LogRead(Errorbits[nFramesAcked + nBuffered]);
        }

        // Msgsread++;
        // ErrSend();
        // nBuffered++;
        // Msgsread++;

        return;
    }
    else
    {
        // other node
        string payload = packet->getM_Payload();
        int FrameType = packet->getM_FrameType();

    switch (FrameType) {
        case 0:
        // data
        if (payload.length()==0)
        { // control message on Sender side:  either timeout or Processing finished (Network ready)
            if ( packet->getM_Header())
            {
                //Processing finished
                // cout <<"ana 5alast processing w habda2 felly ba3do ya mosahel"<<endl;
                // EV <<"ana 5alast processing w habda2 felly ba3do ya mosahel"<<endl;
                ErrSend();
                inc(next_frame_to_Send);
            }
            else{
                // Timeout
                next_frame_to_Send = packet->getM_Header();
                nBuffered = 0;
                // delete Timeouts[packet->getM_Header()];
                Timeouts[packet->getM_Header()] = NULL;
                for (int i = 0; i < Timeouts.size(); i++)
                {
                    if (Timeouts[i])
                    {
                        cancelEvent(Timeouts[i]);
                        delete Timeouts[i];
                        Timeouts[i] = NULL;
                    }
                }
            }
        }
        else
        {
            cout<<"Receiver received a received message";
            // reciever
            if (packet->getM_Header() == frame_expected)
            {
                inc(frame_expected);
                char parity = packet->getM_Trailer();
                CustomMsg *ack = new CustomMsg();
                if (CheckError(payload, parity))
                {
                    // no error
                    ack->setM_FrameType(1);
                    ack->setM_Ack(frame_expected);
                }
                else
                {
                    ack->setM_FrameType(2);
                    ack->setM_Ack(frame_expected);
                }
                if (uniform(0, 1) * 100 >= 10)
                    sendDelayed(ack, simTime() + PT + TD, "out");
            }
            // law 7aga lessa hateegy b3deen discard
            //  law 7aga gatly 2abl keda handle the counterpart of acc-ack
            cout << " I received the message " << endl;
            EV << " I received your message " << endl;
            cout << "The load is " << payload;
            EV << "The load is " << payload;
        }
        break;
    case 1:
        // ack
        //check if it's on the leading frame in the window
        //if yes, stop the timer and reduce nbuffered and increase nFramesAcked
        //if no: TENATIVE SOLUTION: eb3at el frame w 5las, wl reciever should send all of the acks from that frame up to 
        // the frame that it expects (exclusive)

        break;
    case 2:
        // nack
        //1 2 3 4 5 6   //rec 1 2 3 4 5 
        break;
    }

    // if (!payload.length())
    // { // control message:  either ack or timeout or Processing finished (Network ready)

    //     // case(packet->getM_Ack())
    // }
    // else // reciever
    // {
    //     if (packet->getM_Header() == frame_expected)
    //     {
    //         inc(frame_expected);
    //         char parity = packet->getM_Trailer();
    //         CustomMsg *ack = new CustomMsg();
    //         if (CheckError(payload, parity))
    //         {
    //             // no error
    //             ack->setM_FrameType(1);
    //             ack->setM_Ack(frame_expected);
    //         }
    //         else
    //         {
    //             ack->setM_FrameType(2);
    //             ack->setM_Ack(frame_expected);
    //         }
    //         if (uniform(0, 1) * 100 >= 10)
    //             sendDelayed(ack, simTime() + PT + TD, "out");
    //     }
    //     // law 7aga lessa hateegy b3deen discard
    //     //  law 7aga gatly 2abl keda handle the counterpart of acc-ack
    //     cout << " I received the message " << endl;
    //     EV << " I received your message " << endl;
    //     cout << "The load is " << payload;
    //     EV << "The load is " << payload;
    // }
}
// Message
}

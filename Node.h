
#ifndef __NETWORKPROJECT_NODE_H_
#define __NETWORKPROJECT_NODE_H_

#include <omnetpp.h>
#include <iostream>
#include <vector>
#include <bitset>
#include <stdio.h>
#include <string>
#include <fstream>
#include <cmath>        // included to use pow function
#include <unistd.h>
using std::bitset;
using std::cin;
using std::cout;
using std::fstream;
using std::string;
using std::to_string;
using std::vector;
using std::fixed;
#include "CustomMsg_m.h"
using namespace omnetpp;

/**
 * TODO - Generated class
 */
class Node : public cSimpleModule
{
  char id;
  fstream DataFile;
  string DataPath;
  fstream OutFile;
  vector<string> Messages;
  // string MsgContent; // consider changing this  to vector?
  vector<bitset<4>> Errorbits;
  // bitset<4> ErrorBits; //consider changing this to vecotr ?
  int WS;
  float TO; 
  float PT;
  float TD;
  float ED;
  float DD;
  float LP;


  vector<CustomMsg*> Timeouts;
  // CustomMsg * timeout;
  int next_frame_to_Send; //seqnum
  int nBuffered; // no. of frames in the buffer
  int nFramesAcked; // number of frames acked in the right order so far 
  int frame_expected; //seqnum
  int Msgsread;
protected:
  virtual void initialize();
  virtual void handleMessage(cMessage *msg);

  void ReadFile();

  void ErrSend(bool dupdelaytime);
  int ModifyMsg(string &Payload);               // Modifies the message by adding an error to the payload. It also return the index of modified bit

  void LogRead(bitset<4> const &errorbits); // Logs Reading Action
  void LogTransmissionOrRecieval(bool Transmitting,int seq_num, string payload, char Trailer,int Modified, bool Lost,int Duplicate, int delay);
  void LogTimeout(int seq_num);
  void LogControl(int seq_num,bool Ack, bool Lost);

  void SendData(string Msgg,char Parity, float delay, int modify,bool lost,int duplicate);
  void FramingMsg(string &Payload);         // Applies byte stuffing to payload of msg

  void inc(int &num);
};

#endif

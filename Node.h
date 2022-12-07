
#ifndef __NETWORKPROJECT_NODE_H_
#define __NETWORKPROJECT_NODE_H_

#include <omnetpp.h>
#include <iostream>
#include <vector>
#include <bitset>
#include <stdio.h>
#include <string>
#include <fstream>
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
protected:
  virtual void initialize();
  virtual void handleMessage(cMessage *msg);
  void ReadFile();
  void ErrSend(string Message,bitset<4> ErrBits,bool dupdelaytime);
  
};

#endif

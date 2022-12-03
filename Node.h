
#ifndef __NETWORKPROJECT_NODE_H_
#define __NETWORKPROJECT_NODE_H_

#include <omnetpp.h>
#include <iostream>
#include <vector>
#include <bitset>
#include <stdio.h>
#include <string>
#include <fstream>
using std::bitset;
using std::cin;
using std::cout;
using std::fstream;
using std::string;
using std::to_string;
using std::vector;
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
  string Errorbits;
  string MsgContent;

protected:
  virtual void initialize();
  virtual void handleMessage(cMessage *msg);
  void ReadLine();
};

#endif

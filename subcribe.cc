#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/aodv-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/spectrum-module.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/aodv-helper.h"
#include "ns3/gnuplot.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <math.h>
#include <list>
#include <stdlib.h>
#define packetsize 100
#define numSensors 1
using namespace ns3;

int numconnection=0;
int isconnect[numSensors];
NodeContainer sensorNodes,
              broker,
              subcriber;
 Ptr<Socket> sourcebroker;
     TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ipv4InterfaceContainer wifiInterfaces ;


void SubscribeRequest (Ptr<Socket> socket, uint32_t packetSize){


int clientId=socket->GetNode()->GetId();
if(isconnect[clientId]==0){ 
std::ostringstream message;
   
        if(clientId < 10)
      message << "00" << clientId;
    else if(clientId < 100)
      message << "0" << clientId;
    else 
      message << clientId;
  int packetId=999;
     if(packetId < 10)
      message << "00" << packetId;
    else if(packetId < 100)
      message << "0" << packetId;
    else 
      message << packetId;
    int qos=rand()%5;
    message<<qos;
    int topicId=777;
        if(topicId < 10)
      message << "00" << topicId;
    else if(topicId < 100)
      message << "0" << topicId;
    else 
      message << topicId;

    Ptr<Packet> packet = Create<Packet>((uint8_t*) message.str().c_str(), packetSize);
    socket->Send (packet);
   std::cout <<"Client send subscribe message: "<<message.str()<<" at "<<Simulator::Now().GetSeconds()<<std::endl;
   }   
}
void ClientReceive(Ptr<Socket> socket){
 Ptr<Packet> packet = socket->Recv(); 
    uint8_t *buffer = new uint8_t[packet->GetSize ()];
  packet->CopyData(buffer, packet->GetSize ());
  std::string str = std::string((char*)buffer);
  int packetId=atoi(str.substr(0,3).c_str());
   int returncode = atoi(str.substr(3,3).c_str());
   int clientId=socket->GetNode()->GetId();
   if(returncode!=128){
    std::cout<<"Client receive message: "<<str<<"( packet "<<packetId<<" subscribe successfully ) at "<< Simulator::Now().GetSeconds()<<std::endl;
isconnect[clientId]=1;
  Simulator::Stop();

}
else
      std::cout<<"Client receive message: "<<str<<"( packet "<<packetId<<" subscribe fail at ) "<< Simulator::Now().GetSeconds()<<std::endl;

}
void SubBack(Ptr<Socket> socket,Ptr<Packet> packet){
socket->Send(packet);
}
void BrokerReceive (Ptr<Socket> socket){

  Ptr<Packet> packet = socket->Recv(); 
    uint8_t *buffer = new uint8_t[packet->GetSize ()];
  packet->CopyData(buffer, packet->GetSize ());
  std::string str = std::string((char*)buffer);
    int clientId=atoi(str.substr(0,3).c_str());

  int packetId = atoi(str.substr(3,3).c_str());
  int qos = atoi(str.substr(6,1).c_str());
  std::cout<<"Broker received subscribe message: "<<str<<" from client "<<clientId<<" at "<< Simulator::Now().GetSeconds()<<std::endl;
std::ostringstream msg;
msg<<packetId;
int returncode;
if(0<=qos&&qos<=2)
  returncode=qos;
else
  returncode=128;
msg<<returncode;
 Ptr<Packet> packet1 = Create<Packet>((uint8_t*) msg.str().c_str(), packetsize);
  //source broker
  sourcebroker=Socket::CreateSocket(broker.Get(0),tid);
      InetSocketAddress subAddress = InetSocketAddress(wifiInterfaces.GetAddress(clientId,0), 1883);
sourcebroker->Connect(subAddress);
SubBack(sourcebroker,packet1);
  std::cout<<"Broker send subback to client "<<clientId<<" at "<< Simulator::Now().GetSeconds()<<std::endl;

}

int main(int argc, char *argv[]){
  for(int i=0;i<numSensors;i++){
    isconnect[i]=0;
  }
    sensorNodes.Create(numSensors);
    broker.Create(1);
    subcriber.Create(1);

  std::string phyMode ("DsssRate1Mbps");
// disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", 
                      StringValue (phyMode));
  WifiHelper wifi;

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
// set it to zero; otherwise, gain will be added
  //wifiPhy.Set ("RxGain", DoubleValue (0) ); 
 //   wifiPhy.Set ("TxGain", DoubleValue (0) ); 

  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  //wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO); 
//wifiPhy.Set("TxPowerEnd",DoubleValue(30.0));
//wifiPhy.Set("TxPowerLevels",UintegerValue(1));
//wifiPhy.Set("EnergyDetectionThreshold",DoubleValue(-110));
  // config wifi channel
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());

  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));


  // Add an upper mac and disable rate control
  WifiMacHelper wifiMac;  
  wifiMac.SetType ("ns3::AdhocWifiMac");
    NodeContainer wifiNodes;
    wifiNodes.Add(sensorNodes);
    wifiNodes.Add(broker);
    wifiNodes.Add(subcriber);
  NetDeviceContainer wifiDevices;
  wifiDevices = wifi.Install (wifiPhy, wifiMac, wifiNodes);
  AodvHelper aodv;
  Ipv4StaticRoutingHelper staticRouting;

  Ipv4ListRoutingHelper listRouting;
  listRouting.Add (staticRouting, 0);
  listRouting.Add (aodv, 10);

  InternetStackHelper internet;
  internet.SetRoutingHelper (listRouting);   
  internet.Install (wifiNodes);

  Ipv4AddressHelper ipv4Address;
 // NS_LOG_INFO ("Assign IP Addresses.");
  ipv4Address.SetBase ("10.1.1.0", "255.255.255.0");
  wifiInterfaces = ipv4Address.Assign (wifiDevices);


  MobilityHelper mobility,mob1,mob2;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0),
                                 "MinY", DoubleValue (0),
                                 "DeltaX", DoubleValue (200),
                                 "DeltaY", DoubleValue (200),
                                 "GridWidth", UintegerValue (10),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (sensorNodes);

  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add(Vector(0.0, 00.0, 00.0));
  mob1.SetPositionAllocator (positionAlloc);
   mob1.Install (subcriber);
     Ptr<ListPositionAllocator> positionAlloc1= CreateObject<ListPositionAllocator> ();
  positionAlloc1->Add(Vector(1000.0, 1000.0,0.0));
  mob2.SetPositionAllocator (positionAlloc1);
   mob2.Install (broker);

//source client
Ptr<Socket> *pubSocket;
pubSocket=new Ptr<Socket>[numSensors];
  for(int i = 0; i < numSensors; i++){
    pubSocket[i] = Socket::CreateSocket(sensorNodes.Get(i), tid); 
    InetSocketAddress brokerAddress = InetSocketAddress(wifiInterfaces.GetAddress(numSensors,0), 1883);
    pubSocket[i]->Connect(brokerAddress);
}
//sink broker
  Ptr<Socket> brokerSocket = Socket::CreateSocket (broker.Get(0), tid);
  InetSocketAddress anySourceSocketAddress = InetSocketAddress (Ipv4Address::GetAny (), 1883);
  brokerSocket->Bind (anySourceSocketAddress);
  brokerSocket->SetRecvCallback (MakeCallback (&BrokerReceive));

//sink client
  Ptr<Socket> *sinkSocket;
sinkSocket=new Ptr<Socket>[numSensors];
  for(int i=0;i<numSensors;i++){
sinkSocket[i]=Socket::CreateSocket(sensorNodes.Get(i),tid);
InetSocketAddress anyAddress = InetSocketAddress (Ipv4Address::GetAny (), 1883);
  sinkSocket[i]->Bind (anyAddress);
  sinkSocket[i]->SetRecvCallback (MakeCallback (&ClientReceive));
 }
 for(int k=0;k<10;k++){
   for(int i = 0; i < numSensors; i++){
      Simulator::Schedule (Seconds ((double)10*k), &SubscribeRequest, 
                       pubSocket[i], packetsize);
    }    
  }

    Simulator::Run ();
  Simulator::Destroy ();


  return 0;
}
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
#include "ns3/sixlowpan-module.h"
#include "ns3/aodv-helper.h"
#include "ns3/gnuplot.h"
#include "ns3/netanim-module.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <math.h>
#include <list>
#define fireX 1000.0
#define fireY 1000.0
#define numSensors 100
#define T 30
using namespace ns3;
/*class UAV:public NodeContainer{
public:
  std::list<double> x;
  std::list<double> y;
  std::list<double> temp;

};

UAV uav;*/
int t=0;
int send[1],receive[1];
NodeContainer sensorNodes,
              broker,
              subcriber;
 Ptr<Socket> sourcebroker;
 Gnuplot2dDataset pubdataset,brokerdataset,subdataset,PDRdataset;
 double posX(Ptr<Node> node){
  Ptr<MobilityModel> mobility = node->GetObject<MobilityModel>();
  Vector positon = mobility->GetPosition ();
  return positon.x;

}
double posY(Ptr<Node> node){
  Ptr<MobilityModel> mobility = node->GetObject<MobilityModel>();
  Vector positon = mobility->GetPosition ();
  return positon.y;

}
void updatet(){
  t++;
}

 void Create2DPlotUAV()

{
  std::cout<<"Creating 2DPlot file"<<std::endl;
  std::string fileNameWithNoExtension = "MQTT-2d";
  std::string graphicsFileName        = fileNameWithNoExtension + ".png";
  std::string plotFileName            = fileNameWithNoExtension + ".plt";

  // Instantiate the plot and set its title.
  Gnuplot plot (graphicsFileName);
  plot.SetTitle ("plotTitle");

  // Make the graphics file, which the plot file will create when it
  // is used with Gnuplot, be a PNG file.
  plot.SetTerminal ("png");

  // Set the labels for each axis.
  plot.SetLegend ("x", "y");

  // Set the range for the x axis.
  plot.AppendExtra ("set xrange [0:2500]");
plot.AppendExtra ("set yrange [0:+2500]");
  // Instantiate the dataset, set its title, and make the points be
  // plotted with no connecting lines.
 
  
   pubdataset.SetStyle (Gnuplot2dDataset::POINTS);
pubdataset.SetTitle("Publisher");
   pubdataset.SetStyle (Gnuplot2dDataset::POINTS);
subdataset.SetTitle("Subcriber");
   pubdataset.SetStyle (Gnuplot2dDataset::POINTS);
brokerdataset.SetTitle("Broker");
for(int i=0;i<numSensors;i++){
  
    pubdataset.Add(posX(sensorNodes.Get(i)),posY(sensorNodes.Get(i)));
}
brokerdataset.Add(posX(broker.Get(0)),posY(broker.Get(0)));
subdataset.Add(posX(subcriber.Get(0)),posY(subcriber.Get(0)));
  // Add the dataset to the plot.
  plot.AddDataset (pubdataset);
plot.AddDataset(subdataset);
plot.AddDataset(brokerdataset);
  // Open the plot file.
  std::ofstream plotFile (plotFileName.c_str());

  // Write the plot file.
  plot.GenerateOutput (plotFile);

  // Close the plot file.
  plotFile.close ();
}

void PublisherSend (Ptr<Socket> socket, uint32_t packetSize){


send[t]++;
    int nodeId = socket->GetNode()->GetId();
    double temp = 150*exp(-0.000005*(posX(socket->GetNode())-fireX)*(posX(socket->GetNode())-fireX)-0.000005*(posY(socket->GetNode())-fireY)*(posY(socket->GetNode())-fireY))+20;

    std::ostringstream message;
    int topicId=1;
     if(topicId < 10)
      message << "00" << topicId;
    else if(topicId < 100)
      message << "0" << topicId;
    else 
      message << topicId;
    if(nodeId < 10)
      message << "00" << nodeId;
    else if(nodeId < 100)
      message << "0" << nodeId;
    else 
      message << nodeId;

    if(temp < 100) 
      message << "0" << (int)(temp);
    else 
      message << (int)(temp);

    Ptr<Packet> packet = Create<Packet>((uint8_t*) message.str().c_str(), packetSize);
    socket->Send (packet);
   std::cout <<"Node "<<nodeId<<" send "<<message.str()<<" at "<<Simulator::Now().GetSeconds()<<std::endl;
  
     
}
void BrokerSend(Ptr<Socket> socket,Ptr<Packet> packet){
  socket->Send(packet);
}
void BrokerReceive (Ptr<Socket> socket){

  Ptr<Packet> packet = socket->Recv(); 
    uint8_t *buffer = new uint8_t[packet->GetSize ()];
  packet->CopyData(buffer, packet->GetSize ());
  std::string str = std::string((char*)buffer);

  int topicId = atoi(str.substr(0,3).c_str());
   switch(topicId){
    case 1:
     BrokerSend(sourcebroker,packet);
       std::cout<<"Broker received message and send it to subcriber "<<" at "<<Simulator::Now().GetSeconds()<<std::endl;
     break;
     default: break;
   }

}
void SubcriberReceive (Ptr<Socket> socket){
receive[t]++;
  Ptr<Packet> packet = socket->Recv();      
 std::cout<<"Subcriber received a packet "<<" at "<<Simulator::Now().GetSeconds()<<std::endl;

}
void PDR(){
for(int i=0;i<1;i++){
  std::cout<<"PDR["<<i<<"]="<<(double)receive[i]/send[i]<<std::endl;
}
}
void stopSimulation(){
Simulator::Stop();
}
int main(int argc, char *argv[]){
    sensorNodes.Create(numSensors);
    broker.Create(1);
    subcriber.Create(1);

  std::string phyMode ("DsssRate1Mbps");
  uint32_t packetSize = 100; // bytes
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
  Ipv4InterfaceContainer wifiInterfaces ;
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

     TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
//source publisher
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
  //source broker
  sourcebroker=Socket::CreateSocket(broker.Get(0),tid);
      InetSocketAddress subAddress = InetSocketAddress(wifiInterfaces.GetAddress(numSensors+1,0), 1883);
sourcebroker->Connect(subAddress);
//sink subcriber
Ptr<Socket> subSocket=Socket::CreateSocket(subcriber.Get(0),tid);
InetSocketAddress anyAddress = InetSocketAddress (Ipv4Address::GetAny (), 1883);
  subSocket->Bind (anyAddress);
  subSocket->SetRecvCallback (MakeCallback (&SubcriberReceive));
 for(int k=0;k<1;k++){
   for(int i = 0; i < numSensors; i++){
      Simulator::Schedule (Seconds ((double)(0.2*i+k*T)), &PublisherSend, 
                       pubSocket[i], packetSize);
    }    
  }
  Simulator::Schedule(Seconds(25),&updatet);
Simulator::Schedule(Seconds(29),&PDR);
Simulator::Schedule(Seconds(29.5),&Create2DPlotUAV);
Simulator::Schedule(Seconds(30),&stopSimulation);
    Simulator::Run ();

  Simulator::Destroy ();


  return 0;
}

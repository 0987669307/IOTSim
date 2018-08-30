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
#define numUav 2
#define vuav 5.0
using namespace ns3;
/*class UAV:public NodeContainer{
public:
  std::list<double> x;
  std::list<double> y;
  std::list<double> temp;
};
UAV uav;*/
  MobilityHelper mob1;
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      Ptr<Socket> *source,*sink;
        Ipv4InterfaceContainer wifiInterfaces ;
int packetSize=100;
AnimationInterface *animation=0;
NodeContainer uav, task[10], extra;
void time(){
  std::cout<<Simulator::Now().GetSeconds()<<" s "<<std::endl;
}
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

void stopmove(Ptr<Node> cvNode){
   Ptr<ConstantVelocityMobilityModel> cvMobuav = cvNode->GetObject<ConstantVelocityMobilityModel>();
   cvMobuav-> SetVelocity(Vector(0.0, 0.0, 0.0));  
}
void gotoxy(Ptr<Node> node,double x, double y){
//  std::cout<<"UAV "<<idUAV<<": vi tri hien tai: x="<<posX(node)<<" y="<<posY(node)<<std::endl;
 // std::cout<<"Vi tri dich: x="<<x<<" y="<<y<<" at "<<Simulator::Now().GetSeconds()<<std::endl;
   double xx=x-posX(node);
  double yy=y-posY(node);
  double distance=sqrt(xx*xx+yy*yy);
  double vx=xx/distance;
  double vy=yy/distance;
    Ptr<ConstantVelocityMobilityModel> cvMobuav = node->GetObject<ConstantVelocityMobilityModel>();
  cvMobuav-> SetVelocity(Vector(vx*vuav,vy*vuav, 0.0));  
  Simulator::Schedule(Seconds(distance/vuav),&stopmove,node);

}
void stopSimulation(){
  Simulator::Stop();
}
void newtask(AnimationInterface* anim, MobilityHelper mblt, NodeContainer container, double x, double y, int priority){
  Ptr<ListPositionAllocator> positionAll = CreateObject<ListPositionAllocator> ();
  positionAll->Add(Vector(x, y, 00.0));
  mblt.SetPositionAllocator (positionAll);
  mblt.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  mblt.Install(container);
  anim->UpdateNodeSize(container.Get(0)->GetId(),20,20);
  switch (priority){
    case 1:{
      anim->UpdateNodeColor(container.Get(0),255,255,0);
      break;
    }
    case 2:{
      anim->UpdateNodeColor(container.Get(0),0,255,0);
      break;      
    }
    case 3:{
      anim->UpdateNodeColor(container.Get(0),0,0,255);
      break;
    }
    case 4:{
      anim->UpdateNodeColor(container.Get(0),0,255,255);
      break;      
    }
    case 5:{
      anim->UpdateNodeColor(container.Get(0),128,128,00);
      break;      
    }
    case 6:{
      anim->UpdateNodeColor(container.Get(0),255,00,255);
      break;      
    }
  }  
}
void taskComplete(AnimationInterface* anim, NodeContainer container){
  anim->UpdateNodeSize(container.Get(0)->GetId(),0,0);
}
void generateTask1(){
  Simulator::Schedule(Seconds(0.0),&newtask,animation,mob1,task[3],150,150,3);
  Simulator::Schedule(Seconds(0.0),&newtask,animation,mob1,task[2],50,150,2);
  Simulator::Schedule(Seconds(0.0),&newtask,animation,mob1,task[1],150,50,6);
}
void generateTask2(){
  Simulator::Schedule(Seconds(0.0),&newtask,animation,mob1,task[4],220,130,1);
  Simulator::Schedule(Seconds(0.0),&newtask,animation,mob1,task[5],280,50,6);
  Simulator::Schedule(Seconds(0.0),&newtask,animation,mob1,task[6],330,90,5);
  Simulator::Schedule(Seconds(0.0),&newtask,animation,mob1,task[7],380,190,3);
}
void extraTask(){
  Simulator::Schedule(Seconds(300),&newtask,animation,mob1,task[0],260,100,1);
  Simulator::Schedule(Seconds(250),&newtask,animation,mob1,task[8],220,20,2);
  Simulator::Schedule(Seconds(300),&newtask,animation,mob1,task[9],350,150,4);
}
void handle1(){
  Simulator::Schedule(Seconds(1),&gotoxy,uav.Get(0),50,150);
  Simulator::Schedule(Seconds(200),&taskComplete,animation,task[2]);
  Simulator::Schedule(Seconds(201),&gotoxy,uav.Get(0),150,150);
  Simulator::Schedule(Seconds(350),&taskComplete,animation,task[3]);
  Simulator::Schedule(Seconds(351),&gotoxy,uav.Get(0),150,50);
  Simulator::Schedule(Seconds(500),&taskComplete,animation,task[1]);    
}
void handle2(){
  Simulator::Schedule(Seconds(1), &gotoxy,uav.Get(1),220,130);
  Simulator::Schedule(Seconds(180),&taskComplete,animation,task[4]); 
  Simulator::Schedule(Seconds(181),&gotoxy,uav.Get(1),380,190);
  Simulator::Schedule(Seconds(251),&gotoxy,uav.Get(1),220,20);
  Simulator::Schedule(Seconds(380),&taskComplete,animation,task[8]);
  Simulator::Schedule(Seconds(381),&gotoxy,uav.Get(1),260,100);
  Simulator::Schedule(Seconds(570),&taskComplete,animation,task[0]);
  Simulator::Schedule(Seconds(502),&gotoxy,uav.Get(0),380,190);
  Simulator::Schedule(Seconds(650),&taskComplete,animation,task[7]);
  Simulator::Schedule(Seconds(571),&gotoxy,uav.Get(1),350,150);
  Simulator::Schedule(Seconds(700),&taskComplete,animation,task[9]);
  Simulator::Schedule(Seconds(651),&gotoxy,uav.Get(0),330,90);
  Simulator::Schedule(Seconds(850),&taskComplete,animation,task[6]);
  Simulator::Schedule(Seconds(701),&gotoxy,uav.Get(1),280,50);
  Simulator::Schedule(Seconds(1000),&taskComplete,animation,task[5]);
  Simulator::Schedule(Seconds(851),&gotoxy,uav.Get(0),280,50);
 
  Simulator::Schedule(Seconds(1005),&stopSimulation);

}
void uavSend(Ptr<Socket> socket, int packetSize){
    int idUav = socket->GetNode()->GetId();

    std::ostringstream message;
    if(idUav<10)
      message<<"0"<<idUav;
    else
      message << idUav;
    message<<"DoYouNeedHelp";
    Ptr<Packet> packet = Create<Packet>((uint8_t*) message.str().c_str(), packetSize);
    socket->Send (packet);
}
void uavReceive(Ptr<Socket> socket){
    int id=socket->GetNode()->GetId();

  Ptr<Packet> packet = socket->Recv();      
  uint8_t *buffer = new uint8_t[packet->GetSize ()];
  packet->CopyData(buffer, packet->GetSize ());
  std::string str = std::string((char*)buffer);
  int idUav = atoi(str.substr(0,2).c_str());
  std::string msg = str.substr(2,13);
  if(msg=="DoYouNeedHelp"){
    std::ostringstream message;
    if(idUav<10)
      message<<"0"<<idUav;
    else
      message << idUav;
  message<<"Yes";
  source[id] = Socket::CreateSocket(uav.Get(id), tid);
  InetSocketAddress uavSocketAddress = InetSocketAddress(wifiInterfaces.GetAddress(idUav,0), 80);
  source[id]->Connect(uavSocketAddress);
  Ptr<Packet> packet1 = Create<Packet>((uint8_t*) message.str().c_str(), packetSize);
    source[id]->Send (packet1);
    }
    std::cout<<"UAV"<<id<<" received: "<<msg<<" at "<<Simulator::Now().GetSeconds()<< std::endl;
}
int main(int argc, char *argv[]){

  uav.Create(numUav);
  for(int i=0;i<10;i++){
    task[i].Create(1);
  }
  extra.Create(1);

  //wifi
   std::string phyMode ("DsssRate1Mbps");
     // Convert to time object

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
  wifiPhy.Set ("RxGain", DoubleValue (-7) ); 
 //   wifiPhy.Set ("TxGain", DoubleValue (0) ); 

  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO); 
//wifiPhy.Set("TxPowerStart",DoubleValue(30.0));
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
  NetDeviceContainer wifiDevices;
    wifiDevices = wifi.Install (wifiPhy, wifiMac, uav);

  AodvHelper aodv;
  Ipv4StaticRoutingHelper staticRouting;

  Ipv4ListRoutingHelper listRouting;
  listRouting.Add (staticRouting, 0);
  listRouting.Add (aodv, 10);

  InternetStackHelper internet;
  internet.SetRoutingHelper (listRouting);   
  internet.Install (uav);

  Ipv4AddressHelper ipv4Address;
 // NS_LOG_INFO ("Assign IP Addresses.");
  ipv4Address.SetBase ("10.1.1.0", "255.255.255.0");
  wifiInterfaces = ipv4Address.Assign (wifiDevices);
  //source
  source=new Ptr<Socket>[numUav];
  source[0] = Socket::CreateSocket(uav.Get(0), tid);
  InetSocketAddress uavSocketAddress = InetSocketAddress(wifiInterfaces.GetAddress(1,0), 80);
  source[0]->Connect(uavSocketAddress);

    //sink
  sink=new Ptr<Socket>[numUav];
  for(int i=0;i<numUav;i++){
  sink[i]= Socket::CreateSocket(uav.Get(i),tid);
  InetSocketAddress anySourceSocketAddress = InetSocketAddress (Ipv4Address::GetAny (), 80);
  sink[i]->Bind (anySourceSocketAddress);
  sink[i]->SetRecvCallback (MakeCallback (&uavReceive));
  }
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add(Vector(100.0, 100.0, 00.0));
  positionAlloc->Add(Vector(300,100,0));
  mob1.SetPositionAllocator (positionAlloc);
  mob1.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  mob1.Install(uav);
  Ptr<ListPositionAllocator> position = CreateObject<ListPositionAllocator> ();
  position->Add(Vector(400.0, 200.0, 00.0));
  mob1.SetPositionAllocator (position);
  mob1.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  mob1.Install(extra);

  animation=new  AnimationInterface("task.xml") ;
  generateTask1();
  generateTask2();
  extraTask();
  handle1();
  handle2();

  Simulator::Schedule(Seconds(501),&uavSend,source[0],packetSize);
  //int resourceId=animation->AddResource("/home/nguyen/Downloads/ns-allinone-3.28/netanim-3.108/ns3-logo1.png");
  //animation->UpdateNodeImage(0,resourceId);
 // animation->UpdateNodeImage(1,resourceId);
  animation->UpdateNodeSize(uav.Get(0)->GetId(),10,10);
  animation->UpdateNodeSize(uav.Get(1)->GetId(),10,10);
  animation->UpdateNodeSize(extra.Get(0)->GetId(),0,0);
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

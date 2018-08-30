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
#define numUav 4
#define vuav 5.0
using namespace ns3;
/*class UAV:public NodeContainer{
public:
  std::list<double> x;
  std::list<double> y;
  std::list<double> temp;
};
UAV uav;*/
int idEvent[10];
int x[10],y[10];
int minx=200,maxx=400,miny=0,maxy=200;
int currentContainer=0;
int numEvent=0;
  MobilityHelper mob1;
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      Ptr<Socket> *source,*sink;
        Ipv4InterfaceContainer wifiInterfaces ;
int packetSize=500;
AnimationInterface *animation=0;
NodeContainer uav, task[10];
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
int timmin(int a[],int m,int n){
  int id;
  int min=a[m];
  for(int i=m+1;i<n;i++){
    if(a[i]<min&&a[i]>0) id=i;
  }
  return id;
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
void taskComplete(AnimationInterface* anim, NodeContainer container){
  anim->UpdateNodeSize(container.Get(0)->GetId(),0,0);
  numEvent--;
  if(numEvent==0)
    stopSimulation();
}
void newtask(AnimationInterface* anim, MobilityHelper mblt, NodeContainer container, double x, double y, int typeTask,int id){
  Ptr<ListPositionAllocator> positionAll = CreateObject<ListPositionAllocator> ();
  positionAll->Add(Vector(x, y, 00.0));
  mblt.SetPositionAllocator (positionAll);
  mblt.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  mblt.Install(container);
  anim->UpdateNodeSize(container.Get(0)->GetId(),20,20);

  switch (typeTask){
    case 1:{
      anim->UpdateNodeColor(container.Get(0),255,0,0);
      break;
    }
    case 2:{
      anim->UpdateNodeColor(container.Get(0),0,255,0);
      break;      
    }

  } 

  Simulator::Schedule(Seconds(rand()%(360-120+1)+120),&taskComplete,animation,task[id]);

}

void generateTask(){
  for(int i=0;i<10;i++){
    pri[i]=0;
    if(i<50){
      x[i]=rand()%(maxx1-minx1+1)+minx1;
      y[i]=rand()%(maxy1-miny1+1)+miny1;
    }
    else{
      x[i]=rand()%(maxx2-minx2+1)+minx2;
      y[i]=rand()%(maxy2-miny2+1)+miny2;
    }
  }
numEvent1=rand()%5+1;
numEvent2=rand()%5+1;
for(int i=0;i<numEvent1;i++){
newtask(animation,mob1,task[currentContainer],x[currentContainer],y[currentContainer],rand()%5+1,currentContainer);
currentContainer++;
}
for(int i=0;i<numEvent2;i++){
newtask(animation,mob1,task[currentContainer],x[currentContainer+50],y[currentContainer+50],rand()%5+1,currentContainer);
currentContainer++;
}
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

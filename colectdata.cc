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
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <math.h>
#include <list>
#define fireX 1200.0
#define fireY 800.0
#define numSensors 100
#define numUav 10
#define vuav 7.0
#define r 100.0
#define a 0.05
#define a1 0.5
#define a2 0.6
#define a3 0.7
#define T 25
using namespace ns3;
/*class UAV:public NodeContainer{
public:
  std::list<double> x;
  std::list<double> y;
  std::list<double> temp;

};

UAV uav;*/
class Sigate:public NodeContainer{
public:
  std::list<double> x;
  std::list<double> y;
  std::list<double> temp;

};
double totalEnergy=0.0;
double e[numUav];
int state[numUav];
int numUavNeed;
double t1=0.0,t2=0.0;
Sigate subcriber;
double minx,miny,maxx,maxy;
NodeContainer sensorNodes,
              broker,
              uav;
 Ptr<Socket> sourcebroker;
 Gnuplot2dDataset pubdataset,brokerdataset,subdataset,PDRdataset,dts[numUav];
 int numdataset=0;
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
double getValue(std::list<double>& l,int n){
  std::list<double>::iterator i = l.begin();
  std::advance(i, n);
  return *i; 
}
void gotoxy(Ptr<Node> node,double x, double y,Gnuplot2dDataset gn);
void stopmove(Ptr<Node> cvNode,Gnuplot2dDataset gn);
 void Create2DPlotUAV()

{
  std::string fileNameWithNoExtension = "UAVmove-2d";
  std::string graphicsFileName        = fileNameWithNoExtension + ".png";
  std::string plotFileName            = fileNameWithNoExtension + ".plt";
  std::string plotTitle               = " ";
  std::string dataTitle               = " ";

  // Instantiate the plot and set its title.
  Gnuplot plot (graphicsFileName);
  plot.SetTitle (plotTitle);

  // Make the graphics file, which the plot file will create when it
  // is used with Gnuplot, be a PNG file.
  plot.SetTerminal ("png");

  // Set the labels for each axis.
  plot.SetLegend ("x", "y");

  // Set the range for the x axis.
  plot.AppendExtra ("set xrange [0:2000]");
plot.AppendExtra ("set yrange [0:+2500]");
  // Instantiate the dataset, set its title, and make the points be
  // plotted with no connecting lines.
 
  
   pubdataset.SetStyle (Gnuplot2dDataset::POINTS);
  pubdataset.SetTitle("Sensor");
for(int i=0;i<2000;i+=200){
  for(int j=0;j<2000;j+=200)
    pubdataset.Add(i,j);
}
  // Add the dataset to the plot.
  plot.AddDataset (pubdataset);
  for(int i=0;i<numdataset;i++){
dts[i].SetStyle(Gnuplot2dDataset::LINES);
 
plot.AddDataset(dts[i]);
}
  // Open the plot file.
  std::ofstream plotFile (plotFileName.c_str());

  // Write the plot file.
  plot.GenerateOutput (plotFile);

  // Close the plot file.
  plotFile.close ();
}
Gnuplot2dDataset energyDataset;
 void Create2DEnergyPlot()

{
  std::string fileNameWithNoExtension = "Energy-2d";
  std::string graphicsFileName        = fileNameWithNoExtension + ".png";
  std::string plotFileName            = fileNameWithNoExtension + ".plt";
  std::string plotTitle               = " ";
  std::string dataTitle               = " ";

  // Instantiate the plot and set its title.
  Gnuplot plot (graphicsFileName);
  plot.SetTitle (plotTitle);

  // Make the graphics file, which the plot file will create when it
  // is used with Gnuplot, be a PNG file.
  plot.SetTerminal ("png");

  // Set the labels for each axis.
  plot.SetLegend ("x", "y");

  // Set the range for the x axis.
  plot.AppendExtra ("set xrange [0:2000]");
plot.AppendExtra ("set yrange [0:+25000]");
  // Instantiate the dataset, set its title, and make the points be
  // plotted with no connecting lines.
 
  
   energyDataset.SetStyle (Gnuplot2dDataset::POINTS);
  energyDataset.SetTitle("Energy");

  // Add the dataset to the plot.
  plot.AddDataset (energyDataset);

  // Open the plot file.
  std::ofstream plotFile (plotFileName.c_str());

  // Write the plot file.
  plot.GenerateOutput (plotFile);

  // Close the plot file.
  plotFile.close ();
}
void stopsimulation(){
  Create2DPlotUAV();
  Create2DEnergyPlot();
  Simulator::Stop();
}
void energy(){
t1=t2;
t2=Simulator::Now().GetSeconds();
for(int i=0;i<numUavNeed;i++){
  switch(state[i]){
    case 0:
    totalEnergy+=a*vuav*(t2-t1);
    break;
    case 1:
    totalEnergy+=a1*(t2-t1);
    break;
    case 2:
    totalEnergy+=a2*(t2-t1);
    break;
    case 3:
    totalEnergy+=a3*(t2-t1);
    break;
    case 4:
    totalEnergy+=a*vuav*(t2-t1);
    break;
    default:
    break;
  }
}
energyDataset.Add(t2,totalEnergy);
}


void xulydulieu(){
  double minx=getValue(subcriber.x,0);
  double miny=getValue(subcriber.y,0);
  double maxx=getValue(subcriber.x,0);
  double maxy=getValue(subcriber.y,0);
  for(uint32_t i=1;i<subcriber.x.size();i++){

if(minx>getValue(subcriber.x,i)) minx =getValue(subcriber.x,i);
if(maxx<getValue(subcriber.x,i)) maxx =getValue(subcriber.x,i);
if(miny>getValue(subcriber.y,i)) miny =getValue(subcriber.y,i);
if(maxy<getValue(subcriber.y,i)) maxy =getValue(subcriber.y,i);

  }
  double tamx=(minx+maxx)/2.0;
double tamy=(miny+maxy)/2.0;
int m=(maxx-minx)/(r*sqrt(2));
int n=(maxy-miny)/(r*sqrt(2));
int k=m*n;
numUavNeed=k;
numdataset+=k;
std::cout<<"maxx="<<maxx<<" maxy="<<maxy<<std::endl;
std::cout<<"minx="<<minx<<" miny="<<miny<<std::endl;

std::cout<<"m="<<m<<" n="<<n<<std::endl;
double arrx[k],arry[k];
if(m%2==0&&n%2==0){
  for(int i=-m/2;i<m/2;i++){
    for(int j=-n/2;j<n/2;j++){
arrx[n*(i+m/2)+j+n/2]=tamx+r/sqrt(2)+i*r*sqrt(2);
arry[n*(i+m/2)+j+n/2]=tamy+r/sqrt(2)+j*r*sqrt(2);

    }
  }
}
if(m%2!=0&&n%2==0){
  for(int i=-m/2;i<=m/2;i++){
    for(int j=-n/2;j<n/2;j++){
arrx[n*(i+m/2)+j+n/2]=tamx+i*r*sqrt(2);
arry[n*(i+m/2)+j+n/2]=tamy+r/sqrt(2)+j*r*sqrt(2);

    }
  }
}
if(m%2==0&&n%2!=0){
  for(int i=-m/2;i<m/2;i++){
    for(int j=-n/2;j<=n/2;j++){
arrx[n*(i+m/2)+j+n/2]=tamx+r/sqrt(2)+i*r*sqrt(2);
arry[n*(i+m/2)+j+n/2]=tamy+j*r*sqrt(2);

    }
  }
}if(m%2!=0&&n%2!=0){
  for(int i=-m/2;i<=m/2;i++){
    for(int j=-n/2;j<=n/2;j++){
arrx[n*(i+m/2)+j+n/2]=tamx+r/sqrt(2)+i*r*sqrt(2);
arry[n*(i+m/2)+j+n/2]=tamy+r/sqrt(2)+j*r*sqrt(2);

    }
  }
}
std::cout<<"Toa do tam la: "<<tamx<<" y="<<tamy<<std::endl;
std::cout<<"Cac vi tri can den la: "<<std::endl;
for(int i=0;i<k;i++){
  std::cout<<arrx[i]<<" "<<arry[i]<<std::endl;
}

stopsimulation();
}
void PublisherSend (Ptr<Socket> socket, uint32_t packetSize){


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
  // std::cout <<"Node "<<nodeId<<" send "<<message.str()<<" at "<<Simulator::Now().GetSeconds()<<std::endl;
  
     
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
     //  std::cout<<"Broker received message and send it to subcriber "<<" at "<<Simulator::Now().GetSeconds()<<std::endl;
     break;
     default: break;
   }

}
void SubcriberReceive (Ptr<Socket> socket){
  Ptr<Packet> packet = socket->Recv();      
   uint8_t *buffer = new uint8_t[packet->GetSize ()];
  packet->CopyData(buffer, packet->GetSize ());
  std::string str = std::string((char*)buffer);

 // int topicId = atoi(str.substr(0,3).c_str());
    

  int t = atoi(str.substr(6,3).c_str());
  if(t>100){
  int nodeId = atoi(str.substr(3,3).c_str());
subcriber.x.push_back(posX(sensorNodes.Get(nodeId)));
subcriber.y.push_back(posY(sensorNodes.Get(nodeId)));
subcriber.temp.push_back(t);
}
 std::cout<<"Subcriber received a packet: "<<str <<" at "<<Simulator::Now().GetSeconds()<<std::endl;

}

int main(int argc, char *argv[]){
  for(int i=0;i<numUav;i++){
    e[i]=0;
  }
  for(int i=0;i<numUav;i++){
    state[i]=0;
  }
    sensorNodes.Create(numSensors);
        uav.Create(numUav);

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
     mob1.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
   mob1.Install(uav);

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
  Simulator::Schedule(Seconds(25.0),&xulydulieu);
    Simulator::Run ();
  Simulator::Destroy ();


  return 0;
}
void stopmove(Ptr<Node> cvNode,Gnuplot2dDataset gn){
  int idUAV=cvNode->GetId()-numSensors;
  energy();
  state[idUAV]++;
  Ptr<ConstantVelocityMobilityModel> cvMobuav = cvNode->GetObject<ConstantVelocityMobilityModel>();
  cvMobuav-> SetVelocity(Vector(0.0, 0.0, 0.0));  
  if(state[idUAV]==1||state[idUAV]==5){
  std:: cout<<"UAV "<<idUAV<<" dung tai vi tri x="<<posX(cvNode)<<" y="<<posY(cvNode)<<" at"<<Simulator::Now().GetSeconds()<<std::endl;
  gn.Add(posX(cvNode),posY(cvNode));
  }
  switch(state[idUAV]){
    case 1:
    Simulator::Schedule(Seconds(250),&stopmove,uav.Get(idUAV),dts[idUAV]);
    case 2:
    Simulator::Schedule(Seconds(300),&stopmove,uav.Get(idUAV),dts[idUAV]);
    case 3:
    Simulator::Schedule(Seconds(350),&stopmove,uav.Get(idUAV),dts[idUAV]);
    case 4:
    gotoxy(uav.Get(idUAV),0,0,dts[idUAV]);
  }
  if(idUAV==(numUavNeed-1)&&state[idUAV]==5){
    stopsimulation();
  }
}
void gotoxy(Ptr<Node> node,double x, double y,Gnuplot2dDataset gn){
  energy();
  int idUAV=node->GetId()-numSensors;
  std::cout<<"UAV "<<idUAV<<": vi tri hien tai: x="<<posX(node)<<" y="<<posY(node)<<std::endl;
  std::cout<<"Vi tri dich: x="<<x<<" y="<<y<<" at "<<Simulator::Now().GetSeconds()<<std::endl;

  gn.Add(posX(node),posY(node));
  double xx=x-posX(node);
  double yy=y-posY(node);
  double distance=sqrt(xx*xx+yy*yy);
  double vx=xx/distance;
  double vy=yy/distance;
    Ptr<ConstantVelocityMobilityModel> cvMobuav = node->GetObject<ConstantVelocityMobilityModel>();
  cvMobuav-> SetVelocity(Vector(vx*vuav,vy*vuav, 0.0));  
  Simulator::Schedule(Seconds(distance/vuav),&stopmove,node,gn);

}
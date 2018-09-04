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
#include "ns3/propagation-loss-model.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/sixlowpan-module.h"
#include "ns3/aodv-helper.h"
#include "ns3/gnuplot.h"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <math.h>
#include <list>
#define numNode 5
#define numEvent 20
#define vuav 5.0
#define numTask 3
using namespace ns3;
double t1=0,t2=0;
NodeContainer c;
int packetSize=500;
int xmax1=200,ymax1=200;
double coeff[numNode]={0,1,1,1,1};
double energy[numNode]={99990,20000,20000,20000,20000};
int responseTime[numNode][numTask];
int data[numNode][numTask];
int timeToDo[numEvent];
int state[numNode];
TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
Ipv4InterfaceContainer wifiInterfaces ;
int x[numEvent],y[numEvent],isfinish[numEvent],zone[numEvent];
std::list<double> z1,z2,z3,z4;
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
double distance(double x1, double y1, Ptr<Node> node2){
  double x2=posX(node2),y2=posY(node2);
  double xx=(x1-x2)*(x1-x2);
  double yy=(y1-y2)*(y1-y2);
  return sqrt(xx+yy);
}
double getValue(std::list<double>& l,int n){
  std::list<double>::iterator i = l.begin();
  std::advance(i, n);
  return *i; 
}
int timIdMin(double a[],int n){
	int min=a[0];
	int id=0;
	for(int i=1;i<n;i++){
		if(a[i]<min){
			min=a[i];
			id=i;
		}
	}
	return id;
}
void updateEnergy(){
t1=t2;
t2=Simulator::Now().GetSeconds();
for(int i=0;i<numNode;i++){
	energy[i]-=coeff[i]*(t2-t1);
}
}
void IsStop(){
	int s=1;
	for(int i=0;i<numEvent;i++){
		if(isfinish[i]==0){ 
			s=0;
			break;			
	}
}
	if(s==1){
		std::cout<<"Remain energy:"<<std::endl;
		std::cout<<"UAV 1:"<<energy[1]<<std::endl;
		std::cout<<"UAV 2:"<<energy[2]<<std::endl;
		std::cout<<"UAV 3:"<<energy[3]<<std::endl;
		std::cout<<"UAV 4:"<<energy[4]<<std::endl;
  Simulator::Stop();
}
}
void capnhatheso(int id,int sta){
if(sta==0) coeff[id]=3;
if(sta==1) coeff[id]=8;
if(sta==2) coeff[id]=16;
}
void stopmove(Ptr<Node> cvNode);
void gotoxy(Ptr<Node> node,double x, double y);
void taskfinish(int id,int zone){
	std::cout<<"UAV "<<zone<<" finished task "<<id<<" at "<<Simulator::Now().GetSeconds()<<std::endl;
	isfinish[id]=1;
	updateEnergy();
	capnhatheso(zone,0);
	if(zone==1) {
		
		z1.pop_front();
				
	}
	if(zone==2) {
	
		z2.pop_front();
		
				}
	if(zone==3) {
		
		z3.pop_front();
		
			}
		if(zone==4) {
		
		z4.pop_front();
					
	}
	IsStop();
}

void send(Ptr<Socket> socket,int packetSize){
	std::ostringstream msg;
	int id=socket->GetNode()->GetId();
	msg<<id;
	energy[id]=(int)energy[id];
	if(energy[id]<10)
		msg<<"0000"<<energy[id];
	else if(energy[id]<100)
		msg<<"000"<<energy[id];
	else if(energy[id]<1000)
		msg<<"00"<<energy[id];
	else if(energy[id]<10000)
		msg<<"0"<<energy[id];
	else msg<<energy[id];
	for(int i=0;i<numTask;i++){
		msg<<responseTime[id][i];
		msg<<data[id][i];
	}
	Ptr<Packet> packet=Create<Packet>((uint8_t*)msg.str().c_str(),packetSize);
	socket->Send(packet);
}
void advertise(Ptr<Node> node){
	int id=node->GetId();
	Ptr<Socket> *source;
	source=new Ptr<Socket> [numNode];
	for(int i=0;i<numNode;i++){
		if(i!=id){
		    //source
		source[i] = Socket::CreateSocket(c.Get(id), tid);
 		InetSocketAddress address = InetSocketAddress(wifiInterfaces.GetAddress(i,0), 80);
  		source[i]->Connect(address);
  		send(source[i],packetSize);
	}
}
		std::cout<<"Node "<<id<<" send ad at "<<Simulator::Now().GetSeconds()<<std::endl;

}
void Receive(Ptr<Socket> socket){
	int idNode=socket->GetNode()->GetId();
	Ptr<Packet> packet=socket->Recv();
	uint8_t *buffer=new uint8_t[packet->GetSize()];
	packet->CopyData(buffer,packet->GetSize());
	std::string str=std::string((char*)buffer);
	int id=atoi(str.substr(0,1).c_str());
	/*int e=atoi(str.substr(1,4).c_str());
	int t[3],d[3];
	for(int i=0;i<3;i++){
		t[i]=atoi(str.substr(5+i*5,2).c_str());
		d[i]=atoi(str.substr(7+i*5,3).c_str());
	}*/
	std::cout<<"Node "<<idNode<<" received ad from node "<<id<<" at "<<Simulator::Now().GetSeconds()<<std::endl;
}
void findZone(){
	for(int i=0;i<numEvent;i++){
		if(x[i]<xmax1){
			if(y[i]<ymax1)
				{
					zone[i]=1;
					z1.push_back(i);
				}
			else {
					zone[i]=3;
					z3.push_back(i);
				}
		}
		else{
			if(y[i]<ymax1)
				{
					zone[i]=2;
					z2.push_back(i);
				}
			else {
					zone[i]=4;
					z4.push_back(i);
				}
		}
	}
}
void doTask(int zoneoftask);
int main(){
	c.Create(numNode);
	for(int i=0;i<numNode;i++){
		state[i]=0;
	}
  for(int i=0;i<numEvent;i++){
    if(i<14)
    timeToDo[i]=rand()%(300-180+1)+180;
  else
        timeToDo[i]=rand()%(60-20+1)+20;

  }
  for(int i=0;i<numNode;i++){
  	for(int j=0;j<numTask;j++){
  		if(i==0){
  			coeff[i]=0;
  			responseTime[i][j]=rand()%(60-50+1)+50;
  			data[i][j]=rand()%((j+1)*(200-100+1))+100;
  		}
  		else{
  			coeff[i]=(double)0.01*(rand()%(20-10+1)+10);
  			responseTime[i][j]=rand()%(20-15+1)+15;
  			data[i][j]=rand()%((j+1)*(200-100+1))+100;
  		}
  	}
  }
  for(int i=0;i<numEvent;i++){   
  	isfinish[i]=0;
      x[i]=rand()%(400);
      y[i]=rand()%(400);

  }
  findZone();

  std::cout<<"z1"<<std::endl;
  for(uint8_t i=0;i<z1.size();i++){
  	std::cout<<getValue(z1,i)<<std::endl;
  }
    std::cout<<"z2"<<std::endl;
  for(uint8_t i=0;i<z3.size();i++){
  	std::cout<<getValue(z2,i)<<std::endl;
  }
    std::cout<<"z3"<<std::endl;
  for(uint8_t i=0;i<z3.size();i++){
  	std::cout<<getValue(z3,i)<<std::endl;
  }
    std::cout<<"z4"<<std::endl;
  for(uint8_t i=0;i<z4.size();i++){
  	std::cout<<getValue(z4,i)<<std::endl;
  }
  std::string phyMode ("DsssRate1Mbps");

  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));
  WifiHelper wifi;

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
 
  // set it to zero; otherwise, gain will be added
  wifiPhy.Set ("RxGain", DoubleValue (-7) ); 

  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO); 

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
  wifiDevices = wifi.Install (wifiPhy, wifiMac, c);
  //routing
  AodvHelper aodv;
  Ipv4StaticRoutingHelper staticRouting;
  Ipv4ListRoutingHelper listRouting;
  listRouting.Add (staticRouting, 0);
  listRouting.Add (aodv, 10);
  InternetStackHelper internet;
  internet.SetRoutingHelper (listRouting);   
  internet.Install (c);
  //assigne ip address
  Ipv4AddressHelper ipv4Address;
  ipv4Address.SetBase ("10.1.1.0", "255.255.255.0");
  wifiInterfaces = ipv4Address.Assign (wifiDevices);

  //possition allocation
  MobilityHelper mob;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (100.0, 100.0, 0.0));
  positionAlloc->Add (Vector (100.0, 300.0, 0.0));
  positionAlloc->Add (Vector (300.0, 100.0, 0.0));
  positionAlloc->Add (Vector (300.0, 300.0, 0.0));
  mob.SetPositionAllocator (positionAlloc);
  mob.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mob.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  mob.Install (c);
    //sink
  Ptr<Socket> *sink;

  sink=new Ptr<Socket>[numNode];
  for(int i=0;i<numNode;i++){
  sink[i]= Socket::CreateSocket(c.Get(i),tid);
  InetSocketAddress anySourceSocketAddress = InetSocketAddress (Ipv4Address::GetAny (), 80);
  sink[i]->Bind (anySourceSocketAddress);
  sink[i]->SetRecvCallback (MakeCallback (&Receive));
  }

  for(int i=1;i<5;i++){
  	doTask(i);
  }

  for(int n=0;n<5;n++){
  for(int i=0;i<numNode;i++){
  Simulator::Schedule(Seconds(i+n*200),&advertise,c.Get(i));
  }
  }
  Simulator::Run();
  Simulator::Destroy();
  return 0;
}

void stopmove(Ptr<Node> cvNode){
	int id=cvNode->GetId();

	updateEnergy();
	capnhatheso(id,2);
	if(id==1){ 

	Simulator::Schedule(Seconds(timeToDo[(int)getValue(z1,0)]),&taskfinish,(int)getValue(z1,0),id);
					Simulator::Schedule(Seconds(timeToDo[(int)getValue(z1,0)]+1),&doTask,id);

}
if(id==2){
	Simulator::Schedule(Seconds(timeToDo[(int)getValue(z2,0)]),&taskfinish,(int)getValue(z2,0),id);
					Simulator::Schedule(Seconds(timeToDo[(int)getValue(z2,0)]+1),&doTask,id);

}
if(id==3){
	Simulator::Schedule(Seconds(timeToDo[(int)getValue(z3,0)]),&taskfinish,(int)getValue(z3,0),id);
					Simulator::Schedule(Seconds(timeToDo[(int)getValue(z3,0)]+1),&doTask,id);

}
if(id==4){
	Simulator::Schedule(Seconds(timeToDo[(int)getValue(z4,0)]),&taskfinish,(int)getValue(z4,0),id);
					Simulator::Schedule(Seconds(timeToDo[(int)getValue(z4,0)]+1),&doTask,id);

}
  Ptr<ConstantVelocityMobilityModel> cvMobuav = cvNode->GetObject<ConstantVelocityMobilityModel>();
  cvMobuav-> SetVelocity(Vector(0.0, 0.0, 0.0));  
//  std:: cout<<"UAV "<<cvNode->GetId()-numUav<<" dung tai vi tri x="<<posX(cvNode)<<" y="<<posY(cvNode)<<" at"<<Simulator::Now().GetSeconds()<<std::endl;
}
void gotoxy(Ptr<Node> node,double x, double y){
	updateEnergy();
	capnhatheso(node->GetId(),1);
  double xx=x-posX(node);
  double yy=y-posY(node);
  double distance=sqrt(xx*xx+yy*yy);
  double vx=xx/distance;
  double vy=yy/distance;
  Ptr<ConstantVelocityMobilityModel> cvMobuav = node->GetObject<ConstantVelocityMobilityModel>();
  cvMobuav-> SetVelocity(Vector(vx*vuav,vy*vuav, 0.0));  
  Simulator::Schedule(Seconds(distance/vuav),&stopmove,node);
 // std::cout<<"UAV "<<node->GetId()-numUav<<": vi tri hien tai: x="<<posX(node)<<" y="<<posY(node)<<std::endl;
  //std::cout<<"Vi tri dich: x="<<x<<" y="<<y<<" at "<<Simulator::Now().GetSeconds()<<std::endl;
}

void doTask(int zoneoftask){
	switch(zoneoftask){
		case 1:{
		if(z1.size()>0){
		gotoxy(c.Get(1),x[(int)getValue(z1,0)],y[(int)getValue(z1,0)]);
		std::cout<<"UAV "<<zoneoftask<<" start task: "<<(int)getValue(z1,0)<<" at "<<Simulator::Now().GetSeconds()<< std::endl;
		
				
	}
	 break;
}
		case 2:{
		if(z2.size()>0){
		std::cout<<"UAV "<<zoneoftask<<" start task: "<<(int)getValue(z2,0)<<" at "<<Simulator::Now().GetSeconds()<< std::endl;
		gotoxy(c.Get(2),x[(int)getValue(z2,0)],y[(int)getValue(z2,0)]);
				
	}
	 break;
}
		case 3:{
		if(z3.size()>0){
					std::cout<<"UAV "<<zoneoftask<<" start task: "<<(int)getValue(z3,0)<<" at "<<Simulator::Now().GetSeconds()<< std::endl;		gotoxy(c.Get(3),x[(int)getValue(z3,0)],y[(int)getValue(z3,0)]);
	}
	 break;
}
		case 4:{
		if(z4.size()>0){std::cout<<"UAV "<<zoneoftask<<" start task: "<<(int)getValue(z4,0)<<" at "<<Simulator::Now().GetSeconds()<< std::endl;
		gotoxy(c.Get(4),x[(int)getValue(z4,0)],y[(int)getValue(z4,0)]);
	}
	 break;
}
	}
}
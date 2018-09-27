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
#define numUav 36
#define numTask 360
#define vuav 7.0
#define centerX 600
#define centerY 600

using namespace ns3;
/*class UAV:public NodeContainer{
public:
  std::list<double> x;
  std::list<double> y;
  std::list<double> temp;
};
UAV uav;*/
double r=100*sqrt(2);
  MobilityHelper mob1;
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
        Ipv4InterfaceContainer wifiInterfaces ;
NodeContainer uav,task, e;
int x[numTask],y[numTask];
int pri[numTask];
double X[numUav],Y[numUav];
double t[numUav];
double d[numUav];
int done[numUav];
int timeToFinish[numTask];
int state[numUav];
int stateoftask[numTask];
int first[4]={14,15,20,21};
int second[12]={7,8,9,10,13,16,19,22,25,26,27,28};
int third[20]= {0,1,2,3,4,5,6,11,12,17,18,23,24,29,30,31,32,33,34,35};
double eng[numUav];
int coeff[numUav];
double t1=0,t2=0;
double time1[numUav];
double energyInit=800000.0;
int q=0;
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

void position(Ptr<Node> node, double x, double y){
  MobilityHelper m;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add(Vector(x, y, 00.0));
  m.SetPositionAllocator (positionAlloc);
  m.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  m.Install(node);
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
int timIdMax(double a[],int n){
  int max=a[0];
  int id=0;
  for(int i=1;i<n;i++){
    if(a[i]>max){
      max=a[i];
      id=i;
    }
  }
  return id;
}
void updateEnergy(){
  t1=t2;
  t2=Simulator::Now().GetSeconds();
  for(int i=0;i<numUav;i++){
    eng[i]-=coeff[i]*(t2-t1);
  }
}
void changeCoeff(int idUav, int coEff){
  updateEnergy();
  coeff[idUav]=coEff;
}
void taskComplete( Ptr<Node> node1,int z,int idTask);
void doTask(Ptr<Node> node1, int z);
void help1(Ptr<Node> node, int z);
void help2(Ptr<Node> node, int z);
void spreadTask(int zone,int n);
void isStop(){
q++;
std::cout<<"goi ham isstop "<<q<<" : "<<Simulator::Now().GetSeconds()<<std::endl;
int s1=1,s2=1;
for(int i=0;i<numTask;i++){
  if (pri[i]!=0){
    s1=0;
    break;
  }
}
for(int i=0;i<numUav;i++){
  if(done[i]==0){
    s2=0;
    break;
  }
}
  if(s1==1&&s2==1){
    double totalEnergy=energyInit*numUav;
    double totalTime=0;
        std::cout<<"Thoi gian lam viec cua cac UAV: "<<std::endl;

    for(int i=0;i<numUav;i++){
      totalEnergy-=eng[i];
      std::cout<<i<<": "<<time1[i]<<std::endl;
      totalTime+=time1[i];
    }

    std::cout<<"Tong thoi gian: "<<totalTime<<" (s)"<<std::endl;
    std::cout<<"Tong nang luong tieu thu: "<<totalEnergy/1000000.0<<" (MJ)"<<std::endl;
    int emin=timIdMin(eng, numUav-1);
    std::cout<<" UAV "<<emin<<" tieu thu nhieu nang luong nhat: "<<(energyInit- eng[emin])/1000<<" (KJ)"<<std::endl;
    int emax=timIdMax(eng, numUav-1);
    std::cout<<" UAV "<<emax<<" tieu thu it nang luong nhat: "<<(energyInit- eng[emax])/1000<<" (KJ)"<<std::endl;
    std::cout<<"Thoi gian ket thuc: "<<Simulator::Now().GetSeconds()<< std::endl;
    Simulator::Stop();
  }
}
void stopmove(Ptr<Node> cvNode,int idTask){
  int iduav=cvNode->GetId();
   Ptr<ConstantVelocityMobilityModel> cvMobuav = cvNode->GetObject<ConstantVelocityMobilityModel>();
   cvMobuav-> SetVelocity(Vector(0.0, 0.0, 0.0)); 
  // std::cout<<"UAV "<<cvNode->GetId()<<" dung tai vi tri "<<posX(cvNode)<<" "<<posY(cvNode)<<" : "<< Simulator::Now().GetSeconds()<<std::endl;
   if(idTask==1000){
    changeCoeff(iduav,0);
    done[iduav]=1;
    time1[iduav]=Simulator::Now().GetSeconds()-time1[iduav];
    isStop();
   }
   else{
    changeCoeff(iduav, 250);
   }
}
void gotoxy(Ptr<Node> node,double x, double y,int idTask){

  int idUAV=node->GetId();
  changeCoeff(idUAV,150);
  if(idTask==999){
    time1[idUAV]=Simulator::Now().GetSeconds();
  }
 // std::cout<<"UAV "<<idUAV<<": vi tri hien tai: x="<<posX(node)<<" y="<<posY(node)<<std::endl;
 // std::cout<<"Vi tri dich: x="<<x<<" y="<<y<<" : "<<Simulator::Now().GetSeconds()<<std::endl;
   double xx=x-posX(node);
  double yy=y-posY(node);
  double distance=sqrt(xx*xx+yy*yy);
  double vx=xx/distance;
  double vy=yy/distance;
    Ptr<ConstantVelocityMobilityModel> cvMobuav = node->GetObject<ConstantVelocityMobilityModel>();
  cvMobuav-> SetVelocity(Vector(vx*vuav,vy*vuav, 0.0));  
  Simulator::Schedule(Seconds(distance/vuav),&stopmove,node,idTask);

}
void newtask( Ptr<Node> node,  int id){
     position(node,x[id],y[id]);

}

void capnhatkhoangcach(){
     for(int i=0;i<numUav;i++){
      d[i]=sqrt(X[i]*X[i]+Y[i]*Y[i]);
      t[i]=d[i]/vuav;
    }
}
int main(int argc, char *argv[]){
   uav.Create(numUav);
   task.Create(numTask);
   e.Create(1);
       int m=6,n=6;
  for(int i=-m/2;i<m/2;i++){
    for(int j=-n/2;j<n/2;j++){
      X[n*(i+m/2)+j+n/2]=centerX+r/sqrt(2)+i*r*sqrt(2);
      Y[n*(i+m/2)+j+n/2]=centerY+r/sqrt(2)+j*r*sqrt(2);
    }
  }
   for(int i=0;i<numTask;i++){
    stateoftask[i]=0;
    pri[i]=0;
    int z=i/10;
    int k=i%10;
    if(k==2||k==5||k==8)
      timeToFinish[i]=rand()%(150-75)+75;
    else 
      timeToFinish[i]=rand()%(300-150)+150; 
    
    if(z>17)
    {
      if(i%10<5)
      {
        x[i]=rand()%(int)(r*sqrt(2)/2)+X[z]-r/sqrt(2);
        y[i]=rand()%(int)(r*sqrt(2))+Y[z]-r/sqrt(2);
      }
      else
      {
        x[i]=rand()%(int)(r*sqrt(2)/2)+X[z];
        y[i]=rand()%(int)(r*sqrt(2))+Y[z]-r/sqrt(2);     
      }
    }
    else
    {
      if(i%10<5)
      {
        x[i]=rand()%(int)(r*sqrt(2)/2)+X[z];
        y[i]=rand()%(int)(r*sqrt(2))+Y[z]-r/sqrt(2);
      }
      else
      {
        x[i]=rand()%(int)(r*sqrt(2)/2)+X[z]-r/sqrt(2);
        y[i]=rand()%(int)(r*sqrt(2))+Y[z]-r/sqrt(2);   
      }

    }
   }
   for(int i=0;i<numUav;i++){
    eng[i]=energyInit;
    coeff[i]=0;
    t[i]=0;
    d[i]=0;
    done[i]=0;
   }
/*
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
  */
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add(Vector(0.0, 0.0, 00.0));
  mob1.SetPositionAllocator (positionAlloc);
  mob1.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  mob1.Install(uav);
  position(e.Get(0),1200,1200);

  /*
  0   4  8  12
  1   5  9  13
  2   6  10 14
  3   7  11 15
  */  
 // for(int i=0;i<4;i++){
 //   animation->UpdateNodeSize(uav.Get(i)->GetId(),40,40);
 // }

  for(int i=0;i<4;i++){
    spreadTask(first[i],1);
  }
    for(int i=0;i<12;i++){
    Simulator::Schedule(Seconds(700),&spreadTask,second[i],2);
  }
    for(int i=0;i<20;i++){
    Simulator::Schedule(Seconds(1500),&spreadTask,third[i],3);
  }

 // animation->UpdateNodeSize(uav.Get(0)->GetId(),10,10);

  capnhatkhoangcach();
  for(int i=0;i<4;i++){
  gotoxy(uav.Get(first[i]),X[first[i]], Y[first[i]],999);
  Simulator::Schedule(Seconds(t[first[i]]+1),&doTask,uav.Get(first[i]),first[i]);
}
  for(int i=0;i<12;i++){
  Simulator::Schedule(Seconds(720),&gotoxy,uav.Get(second[i]),X[second[i]], Y[second[i]],999);
  Simulator::Schedule(Seconds(t[second[i]]+720+1),&doTask,uav.Get(second[i]), second[i]);
}
  for(int i=0;i<20;i++){
  Simulator::Schedule(Seconds(1520),&gotoxy,uav.Get(third[i]),X[third[i]], Y[third[i]],999);
  Simulator::Schedule(Seconds(t[third[i]]+1+1520),&doTask,uav.Get(third[i]), third[i]);
}
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
void taskComplete( Ptr<Node> node1, int z, int idTask){
//  int id=node1->GetId();
//  std::cout<<" xu ly xong tac vu "<<idTask<<" : "<< Simulator::Now().GetSeconds()<<std::endl;
  pri[idTask]=0;
  
  doTask(node1,z);  
}
void doTask(Ptr<Node> node1, int z){
  int id1=node1->GetId();
  state[id1]=0;
  int begin=z*10;
  for(int i=begin;i<(begin+10);i++){
  if(stateoftask[i]==0){ 
    stateoftask[i]=1;
    state[id1]=1;
        double xx=posX(uav.Get(id1));
    double yy=posY(uav.Get(id1));
    double x1=xx-x[i];
    double y1=yy-y[i];
    gotoxy(uav.Get(id1),x[i],y[i],i);
    double dist=sqrt(x1*x1+y1*y1);
     Simulator::Schedule(Seconds(timeToFinish[i]+dist/vuav),&taskComplete,node1,z,i);
      break;
    
  }
}
  if(state[id1]==0) {    
   gotoxy(uav.Get(id1),0,0,1000);
  }
}

void spreadTask(int zone,int n){
  int begin=zone*10;
  for(int i=begin;i<(begin+10);i++){
    pri[i]=i%10+1;
    if(n==1)
      Simulator::Schedule(Seconds(100*(i-begin)),&newtask,task.Get(i),i);
    else if(n==2)
      Simulator::Schedule(Seconds(110*(i-begin)),&newtask,task.Get(i),i);
    else
      Simulator::Schedule(Seconds(120*(i-begin)),&newtask,task.Get(i),i);
  }
}


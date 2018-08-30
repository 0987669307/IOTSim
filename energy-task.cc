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
#define fireX 1200.0
#define fireY 800.0
#define numSensors 100
#define numUav 4
#define vuav 7.0
#define r 100.0
#define a 0.05
#define a1 1.2
#define a2 0.4
#define a3 0.8
#define eo 0.2
using namespace ns3;
/*class UAV:public NodeContainer{
public:
  std::list<double> x;
  std::list<double> y;
  std::list<double> temp;

};
UAV uav;*/
double totalEnergy=0.0;
double e[numUav];
int state[numUav];
int numUavNeed=numUav;
double t1=0.0,t2=0.0;
double minx,miny,maxx,maxy;
NodeContainer 
              uav;
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
  plot.SetLegend ("t(s)", "E(J)");

  // Set the range for the x axis.
  plot.AppendExtra ("set xrange [0:2000]");
plot.AppendExtra ("set yrange [0:+5000]");
  // Instantiate the dataset, set its title, and make the points be
  // plotted with no connecting lines.
 
  
   energyDataset.SetStyle (Gnuplot2dDataset::LINES);
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
    totalEnergy+=(a1+eo)*(t2-t1);
    break;
    case 2:
    totalEnergy+=(a2+eo)*(t2-t1);
    break;
    case 3:
    totalEnergy+=(a3+eo)*(t2-t1);
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
void stopmove(Ptr<Node> cvNode){
  int idUAV=cvNode->GetId();
  energy();
  state[idUAV]++;
  Ptr<ConstantVelocityMobilityModel> cvMobuav = cvNode->GetObject<ConstantVelocityMobilityModel>();
  cvMobuav-> SetVelocity(Vector(0.0, 0.0, 0.0));  
  if(state[idUAV]==1||state[idUAV]==5){
  std:: cout<<"UAV "<<idUAV<<" dung tai vi tri x="<<posX(cvNode)<<" y="<<posY(cvNode)<<" at"<<Simulator::Now().GetSeconds()<<std::endl;
  }
  switch(state[idUAV]){
    case 1:{std::cout<<"UAV "<<idUAV<<" bat dau xu ly tac vu 1"<<std::endl;
    Simulator::Schedule(Seconds(350),&stopmove,uav.Get(idUAV));break;
    }
    case 2:{std::cout<<"UAV "<<idUAV<<" da xu ly xong tac vu 1 "<<" at "<<Simulator::Now().GetSeconds()<<std::endl;
      std::cout<<"UAV "<<idUAV<<" bat dau xu ly tac vu 2"<<std::endl;
    Simulator::Schedule(Seconds(250),&stopmove,uav.Get(idUAV));break;
    }
    case 3:{std::cout<<"UAV "<<idUAV<<" da xu ly xong tac vu 2 "<<" at "<<Simulator::Now().GetSeconds()<<std::endl;
      std::cout<<"UAV "<<idUAV<<" bat dau xu ly tac vu 3"<<std::endl;
    Simulator::Schedule(Seconds(300),&stopmove,uav.Get(idUAV));break;
    }
    case 4:std::cout<<"UAV "<<idUAV<<" da xu ly xong tac vu 3 "<<" at "<<Simulator::Now().GetSeconds()<<std::endl;
    {  energy();
      double x=0,y=0;
  std::cout<<"UAV "<<idUAV<<": vi tri hien tai: x="<<posX(uav.Get(idUAV))<<" y="<<posY(uav.Get(idUAV))<<std::endl;
  std::cout<<"Vi tri dich: x="<<x<<" y="<<y<<" at "<<Simulator::Now().GetSeconds()<<std::endl;

  double xx=x-posX(uav.Get(idUAV));
  double yy=y-posY(uav.Get(idUAV));
  double distance=sqrt(xx*xx+yy*yy);
  double vx=xx/distance;
  double vy=yy/distance;
    Ptr<ConstantVelocityMobilityModel> cvMobuav = uav.Get(idUAV)->GetObject<ConstantVelocityMobilityModel>();
  cvMobuav-> SetVelocity(Vector(vx*vuav,vy*vuav, 0.0));  
  Simulator::Schedule(Seconds(distance/vuav),&stopmove,uav.Get(idUAV));
  break;
}
    default: break;
  }
  if(idUAV==(numUavNeed-1)&&state[idUAV]==5){
    stopsimulation();
  }
}
void gotoxy(Ptr<Node> node,double x, double y){
  energy();
  int idUAV=node->GetId();
  std::cout<<"UAV "<<idUAV<<": vi tri hien tai: x="<<posX(node)<<" y="<<posY(node)<<std::endl;
  std::cout<<"Vi tri dich: x="<<x<<" y="<<y<<" at "<<Simulator::Now().GetSeconds()<<std::endl;

  double xx=x-posX(node);
  double yy=y-posY(node);
  double distance=sqrt(xx*xx+yy*yy);
  double vx=xx/distance;
  double vy=yy/distance;
    Ptr<ConstantVelocityMobilityModel> cvMobuav = node->GetObject<ConstantVelocityMobilityModel>();
  cvMobuav-> SetVelocity(Vector(vx*vuav,vy*vuav, 0.0));  
  Simulator::Schedule(Seconds(distance/vuav),&stopmove,node);

}
int main(int argc, char *argv[]){
  for(int i=0;i<numUav;i++){
    e[i]=0;
  }
  for(int i=0;i<numUav;i++){
    state[i]=0;
  }
     uav.Create(numUav);
  MobilityHelper mob1;
 Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add(Vector(0.0, 00.0, 00.0));
  mob1.SetPositionAllocator (positionAlloc);
    mob1.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
   mob1.Install(uav);
gotoxy(uav.Get(0),1129.29,729.289);
gotoxy(uav.Get(1),1129.29,870.711);
gotoxy(uav.Get(3),1270.71,870.711);
gotoxy(uav.Get(2),1270.71,729.289);
     AnimationInterface anim ("energy-task.xml");
    Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

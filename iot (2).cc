// iot.cc - NS-3.39
// Multi-cenário: variação automática de TxPower e PacketInterval

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include <fstream>
#include <iomanip>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("IoT_MultiScenario");

struct Metrics {
  double throughput;
  double delay;
  double pdr;
  uint32_t flows;
};

Metrics RunScenario(uint32_t nNodes, double simTime, double txPower,
                    double packetInterval, uint32_t packetSize)
{
  NodeContainer sensorNodes, sinkNode;
  sensorNodes.Create(nNodes);
  sinkNode.Create(1);

  // WiFi 802.11n
  WifiHelper wifi;
  wifi.SetRemoteStationManager("ns3::MinstrelHtWifiManager");

  YansWifiPhyHelper wifiPhy;
  YansWifiChannelHelper wifiChannel;

  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::LogDistancePropagationLossModel",
                                 "Exponent", DoubleValue(3.0),
                                 "ReferenceDistance", DoubleValue(1.0),
                                 "ReferenceLoss", DoubleValue(46.6777));

  wifiPhy.SetChannel(wifiChannel.Create());
  wifiPhy.Set("TxPowerStart", DoubleValue(txPower));
  wifiPhy.Set("TxPowerEnd", DoubleValue(txPower));

  WifiMacHelper wifiMac;
  wifiMac.SetType("ns3::AdhocWifiMac");

  NetDeviceContainer sensorDevices = wifi.Install(wifiPhy, wifiMac, sensorNodes);
  NetDeviceContainer sinkDevice = wifi.Install(wifiPhy, wifiMac, sinkNode);

  // Mobilidade
  MobilityHelper mobility;
  mobility.SetPositionAllocator("ns3::RandomRectanglePositionAllocator",
                                "X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"),
                                "Y", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(sensorNodes);
  mobility.Install(sinkNode);

  // Internet
  InternetStackHelper internet;
  internet.Install(sensorNodes);
  internet.Install(sinkNode);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  ipv4.Assign(sensorDevices);
  Ipv4InterfaceContainer sinkIf = ipv4.Assign(sinkDevice);

  // Aplicações UDP
  uint16_t port = 4000;
  UdpServerHelper server(port);
  ApplicationContainer serverApp = server.Install(sinkNode.Get(0));
  serverApp.Start(Seconds(0.0));
  serverApp.Stop(Seconds(simTime));

  for (uint32_t i = 0; i < nNodes; i++)
  {
    UdpClientHelper client(sinkIf.GetAddress(0), port);
    client.SetAttribute("MaxPackets", UintegerValue(1000000000u));
    client.SetAttribute("Interval", TimeValue(Seconds(packetInterval)));
    client.SetAttribute("PacketSize", UintegerValue(packetSize));
    ApplicationContainer app = client.Install(sensorNodes.Get(i));
    app.Start(Seconds(1.0));
    app.Stop(Seconds(simTime));
  }

  // FlowMonitor
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  Simulator::Stop(Seconds(simTime));
  Simulator::Run();

  monitor->CheckForLostPackets();
  Ptr<Ipv4FlowClassifier> classifier =
      DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
  auto stats = monitor->GetFlowStats();

  double totalThr = 0.0, totalDelay = 0.0, totalPdr = 0.0;
  uint32_t flows = 0;

  for (auto &f : stats)
  {
    auto st = f.second;
    if (st.txPackets > 0)
    {
      double thr = (double)st.rxBytes * 8.0 / simTime / 1e6;
      double avgDelay = st.rxPackets > 0
                        ? st.delaySum.GetSeconds() / st.rxPackets
                        : 0.0;
      double pdr = (double)st.rxPackets / (double)st.txPackets;

      totalThr += thr;
      totalDelay += avgDelay;
      totalPdr += pdr;
      flows++;
    }
  }

  Simulator::Destroy();

  Metrics m;
  m.throughput = flows ? totalThr / flows : 0.0;
  m.delay = flows ? totalDelay / flows : 0.0;
  m.pdr = flows ? totalPdr / flows : 0.0;
  m.flows = flows;
  return m;
}

int main(int argc, char *argv[])
{
  uint32_t nNodes = 30;
  double simTime = 30.0;
  uint32_t packetSize = 100;
  std::string outCsv = "metrics.csv";

  // PARÂMETROS DOS CENÁRIOS
  std::vector<double> txPowerList = {10.0, 12.0, 14.0, 16.0};
  std::vector<double> intervalList = {0.2, 0.5, 1.0};

  // CSV
  std::ofstream csv(outCsv);
  csv << "TxPower,PacketInterval,Throughput_Mbps,Delay_s,PDR,Flows\n";

  for (double tx : txPowerList)
  {
    for (double interval : intervalList)
    {
      std::cout << "Running scenario: TxPower=" << tx
                << " dBm, Interval=" << interval << " s\n";

      Metrics m = RunScenario(nNodes, simTime, tx, interval, packetSize);

      csv << tx << "," << interval << ","
          << m.throughput << "," << m.delay << ","
          << m.pdr << "," << m.flows << "\n";
    }
  }

  csv.close();
  std::cout << "Finished. Results saved in metrics.csv\n";
  return 0;
}


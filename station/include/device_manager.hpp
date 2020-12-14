#pragma once

#include <filesystem>
#include <map>
#include <regex>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>

#include "include/manager.hpp"
#include "include/packet.hpp"

namespace fs = std::filesystem;

using namespace boost;
using boost::asio::serial_port;
using boost::asio::serial_port_base;

struct device_t
{
  std::string board_id;
  int TTL;
};


using DeviceTree = std::unordered_map<std::string, std::vector<device_t>>;
using PortTree = std::unordered_map<std::string, asio::serial_port *>;

/// Manager to control the devices
class DeviceManager : public IManager
{
private:
        PortTree ports;
        DeviceTree devices;
  asio::io_service io;

public:
  DeviceManager (){};
  ~DeviceManager (){};
  void init (){};
  void stop (){};

  // Ports
  void register_ports ();
  std::vector<std::string> available_ports ();
        DeviceTree device_tree ();

  // Devices

  // Communication
  // void broadcast_header (const header_t &header);
};

// std::vector<std::string>
// DeviceManager::register_ports ()
// {
//   std::vector<std::string> port_list;
//   // TODO: Implementation for Windows
// //
// https://stackoverflow.com/questions/2674048/what-is-proper-way-to-detect-all-available-serial-ports-on-windows

//   // for (const auto &port : this->ports)
//   //   delete port;

//   std::regex valid_port (".*USB.?");

//   /// TODO: Add implementation for Windows
//   for (auto &p : fs::directory_iterator ("/dev/"))
//     {
//       std::string port_path = p.path ();
//       if (std::regex_match (port_path, valid_port))
//         {
//           auto port = new asio::serial_port (this->io);

//           // Configuration of the port
//           // TODO: Read the config values from an external file
//           port->open (port_path);
//           port->set_option (serial_port::baud_rate (115200));
//           port->set_option (serial_port_base::character_size (8));

//           this->ports.push_back (port);
//           port_list.push_back (port_path);
//         }
//     }
//   return port_list;
// }

// // Number of ports registered at the moment
// int
// DeviceManager::available_ports ()
// {
//   return this->ports.size ();
// }

// void
// DeviceManager::broadcast_header (const header_t &header)
// {
//   // for (auto port : this->ports)
//   //   {
//   //     /// TODO: Async write header
//   //   }
// }

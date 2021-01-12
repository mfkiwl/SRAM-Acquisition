#pragma once

#include <filesystem>
#include <map>
#include <regex>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>

#include "include/packet.hpp"

namespace fs = std::filesystem;

using namespace boost;
using boost::asio::serial_port;
using boost::asio::serial_port_base;

#define NUM_DEVS_PER_CHAIN 13

// Metadata for each board in a chain
//
// TODO: Store the memory size for different models
// To use this protocol with different board models, we need to keep track of
// the amount of memory per model so that we don't read outside of the SRAM
// bounds.
struct dev_status_t
{
  std::string board_id;
  uint8_t TTL;
  bool is_on;
};

// Port name and list of devices in the chain
using DeviceMap = std::unordered_map<std::string, std::vector<dev_status_t> >;

// Port name and serial port
using PortMap = std::unordered_map<std::string, asio::serial_port *>;

/// Manager to control the devices
class DeviceManager
{
private:
  PortMap ports;
  DeviceMap devices;
  asio::io_context ctx;

public:
  DeviceManager (){};
  ~DeviceManager ();

  // Ports
  void register_ports ();
  std::vector<std::string> available_ports ();

  // Devices
  DeviceMap device_map ();
  void register_devices ();
  void power_on ();
  void power_off ();

  // Communication
  void broadcast_blocking (const char *buf, const int &len);
  std::vector<std::pair<std::string, header_t> >
  listen_headers_block (const int num_devices);
  body_t listen_body_block (const std::string &port_name);
};

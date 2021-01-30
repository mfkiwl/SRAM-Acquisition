#include <iostream>
#include <thread>

#include "include/device_manager.hpp"

/// Clean the device manager
DeviceManager::~DeviceManager ()
{
  this->devices.clear ();
  this->ports.clear ();
}

// Power on all chains
// Note that to run this command superuser access is required
// TODO: Select which chain to power off
void
DeviceManager::power_on ()
{
  std::system ("ykushcmd -u a");
}

/// Power off all chains
/// TODO: Select which chain to power off
void
DeviceManager::power_off ()
{
  std::system ("ykushcmd -d a");
}

// TODO: Add implementation for Windows
// https://stackoverflow.com/questions/2674048/what-is-proper-way-to-detect-all-available-serial-ports-on-windows
//
// TODO: Read the config values from an external file
void
DeviceManager::register_ports ()
{

  std::regex valid_port (".*USB.?");

  this->devices.clear ();

  for (auto &p : fs::directory_iterator ("/dev/"))
    {
      std::string port_path = p.path ();
      if (std::regex_match (port_path, valid_port))
        {
          auto port_name = p.path ().filename ();
          auto port = new asio::serial_port (this->ctx);

          // Default port configuration
          port->open (port_path);
          port->set_option (serial_port::baud_rate (115200));
          port->set_option (serial_port_base::character_size (8));

          this->ports[port_name] = port;
          this->devices[port_name] = std::vector<dev_status_t> ();
        }
    }
}

std::vector<std::string>
DeviceManager::available_ports ()
{
  std::vector<std::string> ports;

  transform (begin (this->devices), end (this->devices), back_inserter (ports),
             [] (auto const &pair) { return pair.first; });

  return ports;
}

DeviceMap
DeviceManager::device_map ()
{
  return this->devices;
}

/// Send a header to all ports in a blocking way
/// TODO: Add error handling
void
DeviceManager::broadcast_blocking (const uint8_t *buf, const int &len)
{
  for (auto [port_name, port] : this->ports)
    {
      port->write_some (asio::buffer (buf, len));
    }
}

/// Listen for headers in every port in a blocking way
///
/// The number of devices in each port has to be supplied and has to be the
/// same In future implementations this communication will be async and the
/// number of devices will be dynamic
/// @param num_devices number of headers to read
std::vector<std::pair<std::string, header_t> >
DeviceManager::listen_headers_block (const int num_devices)
{
  // Port name from where header came is needed later to send bodies
  std::vector<std::pair<std::string, header_t> > headers;
  auto msg_data = new char[sizeof (header_t)];

  for (auto [port_name, port] : this->ports)
    {

      for (int dev = 0; dev < num_devices; ++dev)
        {
          header_t header;
          port->read_some (asio::buffer (msg_data, sizeof (header_t)));
          memcpy (&header, msg_data, sizeof (header_t));
          headers.push_back (std::make_pair (port_name, header));
        }
    }
  delete msg_data;

  return headers;
}

/// Listen for headers in every port in a blocking way
///
/// The number of devices in each port has to be supplied and has to be the
/// same In future implementations this communication will be async and the
/// number of devices will be dynamic
body_t
DeviceManager::listen_body_block (const std::string &port_name)
{
  body_t body;
  auto msg_data = new char[sizeof (body_t)];
  auto port = this->ports[port_name];
  port->read_some (asio::buffer (msg_data, sizeof (body_t)));

  memcpy (&body, msg_data, sizeof (body_t));

  delete msg_data;
  return body;
}

/// Send a ping to each port to discover devices
void
DeviceManager::register_devices ()
{
  // Register ports if there are no registered ports
  if (this->ports.empty ())
    {
      this->register_ports ();
    }

  // Overwrite the values each time
  this->devices.clear ();

  header_t ping_header = {
    .type = (uint8_t)header_type::PING,
    .TTL = 0,
    .CRC = 0x45,
    .bid_high = 0,
    .bid_medium = 0,
    .bid_low = 0,
  };

  uint8_t *header_ser = new uint8_t[sizeof (header_t)];
  memcpy (header_ser, &ping_header, sizeof (header_t));
  this->broadcast_blocking (header_ser, sizeof (header_t));
  delete header_ser;

  auto acks = this->listen_headers_block (NUM_DEVS_PER_CHAIN);

  for (const auto &[port_name, ack] : acks)
    {
      dev_status_t dev;
      dev.board_id = fmt::format ("0x{0:08X}{1:08X}{2:08X}", ack.bid_high,
                                  ack.bid_medium, ack.bid_low);
      dev.TTL = ack.TTL;
      dev.is_on = true;
      this->devices[port_name].push_back (dev);
    };
}

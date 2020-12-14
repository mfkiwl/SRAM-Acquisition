#include "include/device_manager.hpp"

void
DeviceManager::register_ports ()
{
  // TODO: Implementation for Windows
  // https://stackoverflow.com/questions/2674048/what-is-proper-way-to-detect-all-available-serial-ports-on-windows

  // for (const auto &port : this->ports)
  //   delete port;

  std::regex valid_port (".*USB.?");

  this->devices.clear ();

  /// TODO: Add implementation for Windows
  for (auto &p : fs::directory_iterator ("/dev/"))
    {
      std::string port_path = p.path ();
      if (std::regex_match (port_path, valid_port))
        {
                auto port_name = p.path ().filename ();
          auto port = new asio::serial_port (this->io);

          // Configuration of the port
          // TODO: Read the config values from an external file
          port->open (port_path);
          port->set_option (serial_port::baud_rate (115200));
          port->set_option (serial_port_base::character_size (8));

          this->ports[port_name] = port;

          this->devices[port_name] = std::vector<device_t> ();
        }
    }
}

// Number of ports registered at the moment
std::vector<std::string>
DeviceManager::available_ports ()
{
  std::vector<std::string> ports;

  transform(begin(this->devices), end(this->devices), back_inserter(ports),
            [](auto const& pair) { return pair.first; });

    return ports;
}

// Number of ports registered at the moment
DeviceTree
DeviceManager::device_tree ()
{
  return this->devices;
}

// void
// DeviceManager::broadcast_header (const header_t &header)
// {
//   // for (auto port : this->ports)
//   //   {
//   //     /// TODO: Async write header
//   //   }
// }

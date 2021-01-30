/**
 * @file device_manager.hpp
 *
 * @brief Function prototypes for the device manager.
 *
 * @author Sergio Vinagrero (servinagrero)
 */

#pragma once

#include <filesystem>
#include <map>
#include <regex>
#include <string>
#include <vector>

namespace fs = std::filesystem;

#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>

using namespace boost;
using boost::asio::serial_port;
using boost::asio::serial_port_base;

#include "include/packet.hpp"

/**
 * Number of devices connected in a chain
 */
#define NUM_DEVS_PER_CHAIN 10

/**
 * Status of each connected device
 */
struct dev_status_t
{
  /// Hex string with the device ID.
  std::string board_id;
  /// Position of the device in the chain.
  uint8_t TTL;
  /// If the device is powered on.
  bool is_on;
  // TODO: Store the memory size of each device.
  // To use this protocol with different board models, we need to keep track of
  // the amount of memory per model so that we don't read outside of the SRAM
  // bounds
  /// Maximum size, in KiB, of the SRAM
  uint16_t max_ram;
};

/// Map for port name and serial port object
using PortMap = std::unordered_map<std::string, asio::serial_port *>;

/// Map for port name and list of devices in the chain
using DeviceMap = std::unordered_map<std::string, std::vector<dev_status_t> >;

/**
 * @class DeviceManager
 */
class DeviceManager
{
private:
  /**
   * Map which relates the port name with the port object.
   *
   * The port object is the one used internally to communicate with the
   * devices.
   */
  PortMap ports;

  /**
   * Map with relates a port name with a list of devices in a chain connected
   * to that port.
   *
   * @see dev_status_t
   */
  DeviceMap devices;

  /**
   * Boost asio context to communicate with the devices.
   */
  asio::io_context ctx;

public:
  /**
   * @brief Default constructor.
   */
  DeviceManager (){};
  /**
   * @brief Default destructor.
   */
  ~DeviceManager ();

  /**
   * @brief Register the connected ports into the station.
   *
   * @returns Void.
   */
  void register_ports ();

  /**
   * @brief Register the connected ports into the
   * Return the names of ports registered at the moment. Only the device name
   * is returned and not the full path. In linux systems, 'dev' is removed from
   * the name of the device.
   * @returns Vector of names of the registered ports.
   */
  std::vector<std::string> available_ports ();

  /**
   * @brief Register the connected ports into the station.
   *
   * @returns Map with the port names and the device list.
   */
  DeviceMap device_map ();

  /**
   * @brief Register the connected ports into the station.
   *
   * @returns Void.
   */
  void register_devices ();

  /**
   * @brief Power on the devices connected to a port.
   *
   * Here we are using an Yepkit USB Switchable Hub so that we can control
   * the power to one or multiple ports at a time.
   *
   * Power cycling devices is very important in order to get reliable results.
   *
   * @returns Void.
   */
  void power_on ();

  /**
   * @brief Power off the devices connected a port.
   *
   * @see power_on
   *
   * @returns Void.
   */
  void power_off ();

  /**
   * @brief Power off the devices connected a port.
   *
   * @see power_on
   *
   * @returns Void.
   */
  void broadcast_blocking (const uint8_t *buf, const int &len);

  /**
   * @brief Power off the devices connected a port.
   *
   * @see power_on
   *
   * @returns Void.
   */
  std::vector<std::pair<std::string, header_t> >
  listen_headers_block (const int num_devices);

  /**
   * @brief Power off the devices connected a port.
   *
   * @see power_on
   *
   * @returns Void.
   */
  body_t listen_body_block (const std::string &port_name);
};

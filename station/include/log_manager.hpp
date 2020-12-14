#pragma once

#include <chrono>
#include <string>

#include "include/influxdb.hpp"
#include "include/manager.hpp"

/// InfluxDB Manager
/// Measurements:
///   - sensors
///     - Temperature
///     - Voltage
///   - devices
///     - Connected devices
///     - Power cycles
///   - packets
///     - Dropped packets
///
/// TODO: Authentication
class Logger : public IManager
{
private:
  influxdb_cpp::server_info server;

public:
  Logger ();
  Logger (const std::string &host, const int &port, const std::string &db)
      : server (host, port, db){};
  ~Logger (){};

  // Manager
  void init (){};
  void stop (){};

  // Devices
  void log_register_port (const std::string &port);
  void log_power_cycle (const std::string &status);

  // Packets
  void log_header_sent ();
  void log_header_received ();

  // Sensors
  void log_temperature (const std::string &board_id, const float &temp);
  void log_voltage (const std::string &board_id, const float &vdd);
};

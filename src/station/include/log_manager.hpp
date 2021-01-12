#pragma once

#include <chrono>
#include <string>

#include "include/influxdb.hpp"

/// InfluxDB Manager for logging capabilities
/// Measurements:
///   - sensors
///     - Temperature
///     - Voltage
///   - devices
///     - Connected devices
///     - Power cycles
///   - packets
class Logger
{
private:
  influxdb_cpp::server_info server;

public:
  Logger ();
  Logger (const std::string &host, const int &port, const std::string &db)
      : server (host, port, db){};
  ~Logger (){};

  // Devices
  void log_register_port (const std::string &port);
  void log_power_cycle (const std::string &status);

  // Commands
  void log_command (const std::string &cmd_type, const std::string &cmd_name);
  void log_mem_read (const std::string &board_id,
                     const std::string &mem_address);
  void log_mem_write (const std::string &board_id,
                      const std::string &mem_address);

  // Sensors
  void log_temperature (const std::string &board_id, const float &temp);
  void log_voltage (const std::string &board_id, const float &vdd);
};

/**
 * @file log_manager.hpp
 *
 * @brief Function prototypes for the station.
 *
 * @author Sergio Vinagrero (servinagrero)
 */

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

/**
 * @class Logger
 *
 */
class Logger
{
private:
  /**
   * Connection to the influxdb server.
   *
   */
  influxdb_cpp::server_info server;

public:
  /**
   * @brief Default constructor.
   */
  Logger ();

  /**
   * @brief Register a connected port.
   *
   * @param host Host for the connection to InfluxDB.
   * @param port Port for the connection to InfluxDB.
   * @param db Name of the InfluxDB database.
   */
  Logger (const std::string &host, const int &port, const std::string &db)
      : server (host, port, db){};

  /**
   * @brief Default destructor.
   */
  ~Logger (){};

  /**
   * @brief Register a connected port.
   *
   * @param port Which port was registered.
   *
   * @return Void.
   */
        void log_port_cmd (const std::string &port, const std::string &status);

        /**
   * @brief Register a power cycle.
   *
   * @param status The status of the port.
   * @param port_name The port name. It's value is 'All' if all ports' status.
   * is changed.
   *
   * @return Void.
   */
  void log_power_cycle (const std::string &status,
                        const std::string &port_name);

  /**
   * @brief Register a power cycle.
   *
   * @param type Type of command executed.
   * @param value Value of the command executed.
   *
   * @return Void.
   */
  void log_command (const std::string &type, const std::string &value);

  /**
   * @brief Register a power cycle.
   *
   * @param dev_id The ID of the device.
   * @param cmd_name The name of the command executed.
   * @param mem_address Memory address of the command.
   *
   * @return Void.
   */
  void log_dev_cmd (const std::string &dev_id, const std::string &cmd_name,
                    const std::string &mem_address);

  /**
   * @brief Register a value from one sensor.
   *
   * @param dev_id The ID of the device.
   * @param sensor The type of sensor data to register.
   * @param value The value of the measurement.
   *
   * @return Void.
   */
  void log_sensor (const std::string &dev_id, const std::string &sensor,
                   const float &value);
};

/**
 * @brief Return the current timestamp.
 *
 * @return The current timestamp.
 */
unsigned long int get_timestamp ();

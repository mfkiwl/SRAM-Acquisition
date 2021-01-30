#include "include/log_manager.hpp"

#include <chrono>

unsigned long int
get_timestamp ()
{
  using namespace std::chrono;
  auto now = std::chrono::system_clock::now ().time_since_epoch ();
  return now.count ();
}

/// Default constructor
Logger::Logger () : server ("127.0.0.1", 8086, "station") {}

void
Logger::log_port_cmd (const std::string &port, const std::string &status)
{
        influxdb_cpp::builder ()
                .meas ("devices")
                .tag ("port", port)
                .field ("status", "registered")
                .timestamp (get_timestamp ())
                .post_http (this->server);
}

void
Logger::log_power_cycle (const std::string &status,
                         const std::string &port_name)
{
  influxdb_cpp::builder ()
      .meas ("devices")
      .tag ("status", "power")
      .field ("status", status)
      .field ("port", port_name)
      .timestamp (get_timestamp ())
      .post_http (this->server);
}

void
Logger::log_command (const std::string &type, const std::string &value)
{
  influxdb_cpp::builder ()
      .meas ("commands")
      .tag ("type", type)
      .field ("action", value)
      .timestamp (get_timestamp ())
      .post_http (this->server);
}

void
Logger::log_dev_cmd (const std::string &dev_id, const std::string &cmd_name,
                     const std::string &mem_address)
{
  influxdb_cpp::builder ()
      .meas ("commands")
      .tag ("command", cmd_name)
      .field ("device_id", dev_id)
      .field ("mem_address", mem_address)
      .timestamp (get_timestamp ())
      .post_http (this->server);
}

void
Logger::log_sensor (const std::string &dev_id, const std::string &sensor,
                    const float &value)
{
  influxdb_cpp::builder ()
      .meas ("sensors")
      .tag ("device_id", dev_id)
      .field (sensor, value)
      .timestamp (get_timestamp ())
      .post_http (this->server);
}

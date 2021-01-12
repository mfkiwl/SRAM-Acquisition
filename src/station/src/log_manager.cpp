#include "include/log_manager.hpp"

/// Return the current timestamp
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
Logger::log_power_cycle (const std::string &status)
{
  influxdb_cpp::builder ()
      .meas ("devices")
      .tag ("status", "power_cycle")
      .field ("status", status)
      .timestamp (get_timestamp ())
      .post_http (this->server);
}

void
Logger::log_temperature (const std::string &board_id, const float &temp)
{
  influxdb_cpp::builder ()
      .meas ("sensors")
      .tag ("board_id", board_id)
      .field ("temperature", temp)
      .timestamp (get_timestamp ())
      .post_http (this->server);
}

void
Logger::log_voltage (const std::string &board_id, const float &vdd)
{

  influxdb_cpp::builder ()
      .meas ("sensors")
      .tag ("board_id", board_id)
      .field ("voltage", vdd)
      .timestamp (get_timestamp ())
      .post_http (this->server);
}

void
Logger::log_command (const std::string &cmd_type, const std::string &cmd_name)
{
  influxdb_cpp::builder ()
      .meas ("commands")
      .tag ("type", cmd_type)
      .field ("command", cmd_name)
      .timestamp (get_timestamp ())
      .post_http (this->server);
}

void
Logger::log_mem_read (const std::string &board_id,
                      const std::string &mem_address)
{
  influxdb_cpp::builder ()
      .meas ("commands")
      .tag ("command", "read")
      .field ("board_id", board_id)
      .field ("mem_address", mem_address)
      .timestamp (get_timestamp ())
      .post_http (this->server);
}

void
Logger::log_mem_write (const std::string &board_id,
                       const std::string &mem_address)
{
  influxdb_cpp::builder ()
      .meas ("commands")
      .tag ("command", "write")
      .field ("board_id", board_id)
      .field ("mem_address", mem_address)
      .timestamp (get_timestamp ())
      .post_http (this->server);
}

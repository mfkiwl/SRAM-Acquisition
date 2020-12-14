#include "include/log_manager.hpp"

/// Return the current timestamp
unsigned long int
get_timestamp ()
{
  using namespace std::chrono;
  auto now = std::chrono::system_clock::now ().time_since_epoch ();
  return now.count ();
}

Logger::Logger () : server ("127.0.0.1", 8086, "station") {}

void
Logger::log_register_port (const std::string &port)
{
  influxdb_cpp::builder ()
      .meas ("devices")
      .tag ("status", "registered")
      .field ("port", port)
      .timestamp (get_timestamp ())
      .post_http (this->server);
}

void
Logger::log_power_cycle (const std::string &status)
{
  influxdb_cpp::builder ()
      .meas ("devices")
      // .tag ("", )
      .field ("power_cycle", status)
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

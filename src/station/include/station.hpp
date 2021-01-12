#pragma once

#include <filesystem>
#include <regex>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <served/served.hpp>

#include "include/db_manager.hpp"
#include "include/device_manager.hpp"
#include "include/log_manager.hpp"

#define NUM_THREADS_API 5

class Station
{
private:
  served::multiplexer mux;

  /// Managers
  DBManager db_manager;
  DeviceManager dev_manager;
  Logger logger;

public:
  Station (){};
  ~Station (){};

  int run (const std::string &host, const std::string &port);
};

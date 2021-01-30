/**
 * @file device_manager.hpp
 *
 * @brief Function prototypes for the station.
 *
 * @author Sergio Vinagrero (servinagrero)
 */

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

/**
 * Number of threads the server will use.
 */
#define NUM_THREADS_API 2

/**
 * @brief Station.
 *
 */
class Station
{
private:
  /**
   * Number of threads the server will use.
   */
  served::multiplexer mux;

  /**
   * Number of threads the server will use.
   */
  DBManager db_manager;

  /**
   * Number of threads the server will use.
   */
  DeviceManager dev_manager;

  /**
   * Number of threads the server will use.
   */
  Logger logger;

public:
  /**
   * @brief Default constructor.
   */

  Station (){};
  /**
   * @brief Default destructor.
   */
  ~Station (){};

  /**
   * @brief Run the station.
   *
   * @param host to host the station.
   * @param port Port to be.
   * @returns Should not return.
   */
  int run (const std::string &host, const std::string &port);
};

#pragma once

#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <filesystem>
#include <regex>

#include "dbmanager.h"
#include "devices.h"
#include "logging.h"

using namespace boost;
using boost::asio::serial_port;
using boost::asio::serial_port_base;

class Server
{
private:
  asio::io_context ctx;

public:
  DBManager db_manager;
  DeviceManager dev_manager;

  Logger logger;

  Server ();
  ~Server (){};
  void register_devices ();
  void ping ();
};

Server::Server () {}

void
Server::register_devices ()
{
  auto ports = dev_manager.register_ports ();
  if (ports.size () == 0)
    return;
  for (const auto &port : ports)
    {
      logger.log_register_port (port);
    }
}

// void
// Server::ping ()
// {
//   header_t header = {
//     .type = (uint8_t)header_type::PING,
//     .ttl = 0,
//     .length = 0,
//     .crc = 0x45,
//     // .bid_high = 808994562,
//     // .bid_medium = 808923190,
//     // .bid_low = 2752569,
//     .bid_high = 0,
//     .bid_medium = 0,
//     .bid_low = 0,
//   };

// char header_bin = new char[sizeof(header)];
// char *msg = new char[sizeof(header)];
// header_t h;
// memcpy(header_bin, &header, sizeof(header));

// // auto port = this->devices[0];
// for(const auto &p: this->devices) {
//         std::cout << "Sending PING header\n";
//         p->write_some(asio::buffer(header_bin, sizeof(header)));
// }

/* for(auto i = 0; i < 5; ++i) { */
/*         for(const auto &p: this->devices) { */
/*                 p->read_some (asio::buffer (msg, sizeof(header))); */
/*                 memcpy(&h, msg, sizeof(header)); */
/*                 fmt::print("{}\n", h); */
/*         } */
/* } */
/* } */

/* void */
/* Server::read_region () */
/* { */

/*   header_t header = { */
/*     .type = (uint8_t)header_type::READ, */
/*     .ttl = 0, */
/*     .length = 0, */
/*     .crc = 0x45, */
/*     .bid_high = 808994562, */
/*     .bid_medium = 808923190, */
/*     .bid_low = 2752569, */
/*   }; */

/*   body_t body = { */
/*     .type = (uint8_t)body_type::REGION, */
/*     .crc = 0x33, */
/*     .bid_high = 808994562, */
/*     .bid_medium = 808923190, */
/*     .bid_low = 2752569, */
/*     .start = 0x0000, */
/*     .end = 0x000c, */
/*   }; */

/*   char serialized_header[18]; */
/*   char serialized_body[19]; */
/*   char full_msg[19 + 12]; */
/*   memcpy (serialized_header, &header, sizeof (header)); */

/*   // Send first header */
/*   std::cout << "Sending READ header\n"; */
/*   fmt::print ("{}\n", header); */
/*   for (const auto &p : this->devices) */
/*     p->write_some (asio::buffer (serialized_header, sizeof (header))); */
/*   char *header_data = new char[17]; */

/*   header_t h; */
/*   for (const auto &p : this->devices) */
/*     p->read_some (asio::buffer (header_data, sizeof (header))); */
/*   memcpy (&h, header_data, sizeof (header)); */
/*   std::cout << "Received ACK header\n"; */
/*   fmt::print ("{}\n", h); */

/*   std::cout << "Sending REGION body\n"; */
/*   fmt::print ("{}\n", body); */
/*   memcpy (serialized_body, &body, sizeof (body)); */
/*   for (const auto &p : this->devices) */
/*     p->write_some (asio::buffer (serialized_body, 18)); */

/*   body_t b; */
/*   char mem[13]; */
/*   for (const auto &p : this->devices) */
/*     p->read_some (asio::buffer (full_msg, 30)); */
/*   memcpy (&b, full_msg, sizeof (body)); */
/*   memcpy (&mem, (full_msg + sizeof (body)), body.end - body.start); */
/*   std::cout << "Received REGION data\n"; */
/*   fmt::print ("{}\n", b); */
/*   for (auto c : mem) */
/*     std::cout << unsigned (c) << ","; */
/* } */

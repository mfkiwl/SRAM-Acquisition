#include <string>
#include <thread>

#include <boost/property_tree/json_parser.hpp>
#include <served/served.hpp>

#include "include/db_manager.hpp"
#include "include/station.hpp"

using namespace std::chrono_literals;
namespace bpt = boost::property_tree;

int
Station::run (const std::string &host, const std::string &port)
{

  mux.handle ("/ports/register")
      .get ([this] (served::response &res, const served::request &) {
        bpt::ptree root;
        bpt::ptree portArr;
        std::stringstream ss;

        this->dev_manager.register_ports ();
        this->logger.log_command ("ports", "register");

        root.put ("message", "ports registered");

        bpt::json_parser::write_json (ss, root, true);

        res.set_status (200);
        res << ss.str ();
      });

  mux.handle ("/ports/available")
      .get ([this] (served::response &res, const served::request &) {
        bpt::ptree root;
        bpt::ptree portArr;
        std::stringstream ss;

        auto ports = dev_manager.available_ports ();

        std::transform (ports.begin (), ports.end (),
                        std::back_inserter (portArr),
                        [] (std::string p) -> bpt::ptree::value_type {
                          return bpt::ptree::value_type ("", p);
                        });

        root.add_child ("ports", portArr);
        res.set_status (200);

        bpt::json_parser::write_json (ss, root, true);
        res << ss.str ();
      });

  mux.handle ("/devices/register")
      .get ([this] (served::response &res, const served::request &) {
        bpt::ptree root;
        std::stringstream ss;

        this->dev_manager.register_devices ();
        this->logger.log_command ("devices", "register");

        root.put ("message", "devices registered");

        bpt::json_parser::write_json (ss, root, true);
        res.set_status (200);
        res << ss.str ();
      });

  mux.handle ("/devices/available")
      .get ([this] (served::response &res, const served::request &) {
        bpt::ptree root;
        bpt::ptree portArr;
        std::stringstream ss;

        auto device_tree = dev_manager.device_map ();

        for (const auto &[port, devices] : device_tree)
          {
            bpt::ptree devArr;
            bpt::ptree dev_node;

            for (const auto &dev : devices)
              {
                dev_node.put ("board_id", dev.board_id);
                dev_node.put ("TTL", dev.TTL);
                dev_node.put ("is_on", dev.is_on);
                devArr.push_back (bpt::ptree::value_type ("", dev_node));
              }
            portArr.add_child (port, devArr);
          }
        root.add_child ("ports", portArr);

        res.set_status (200);

        bpt::json_parser::write_json (ss, root, true);
        res << ss.str ();
      });

  mux.handle ("/devices/poweron")
      .get ([this] (served::response &res, const served::request &) {
        this->dev_manager.power_on ();
        this->logger.log_power_cycle ("on");

        bpt::ptree root;
        std::stringstream ss;
        root.put ("message", "all ports powered on");
        bpt::json_parser::write_json (ss, root, true);

        res.set_status (200);

        res << ss.str ();
      });

  mux.handle ("/devices/poweroff")
      .get ([this] (served::response &res, const served::request &) {
        this->dev_manager.power_off ();
        this->logger.log_power_cycle ("off");

        bpt::ptree root;
        std::stringstream ss;
        root.put ("message", "all ports powered off");
        bpt::json_parser::write_json (ss, root, true);

        res.set_status (200);

        res << ss.str ();
      });

  mux.handle ("/commands/read")
      .post ([this] (served::response &res, const served::request &req) {
        std::stringstream input;
        bpt::ptree pt;

        uint16_t mem_address;
        std::string board_id;
        std::string port_name;

        // Parameter parsing
        try
          {
            input << req.body ();
            bpt::json_parser::read_json (input, pt);

            board_id = pt.get<std::string> ("board_id");
            mem_address = pt.get<uint16_t> ("mem_address");
            port_name = pt.get<std::string> ("port_name");
          }
        catch (std::exception &e)
          {
            std::cout << e.what () << "\n";
          }

        uint32_t bid_high = stoul (board_id.substr (2, 8), 0, 16);
        uint32_t bid_medium = stoul (board_id.substr (10, 8), 0, 16);
        uint32_t bid_low = stoul (board_id.substr (18, 8), 0, 16);

        header_t read_header = {
          .type = (uint8_t)header_type::READ,
          .TTL = 0,
          .CRC = 0x69,
          .bid_high = bid_high,
          .bid_medium = bid_medium,
          .bid_low = bid_low,
        };
        auto ser_header = new uint8_t[sizeof (header_t)];
        memcpy (ser_header, &read_header, sizeof (header_t));

        // Send READ header
        this->dev_manager.broadcast_blocking (ser_header, sizeof (header_t));
        // ACK from READ received
        this->dev_manager.listen_headers_block (1);

        body_t read_body = { .type = (uint8_t)body_type::MEMORY,
                             .CRC = 0x69,
                             .bid_high = bid_high,
                             .bid_medium = bid_medium,
                             .bid_low = bid_low,
                             .mem_address = mem_address,
                             .data = { 0 } };

        auto ser_body = new uint8_t[sizeof (body_t)];
        memcpy (ser_body, &read_body, sizeof (body_t));

        // Send READ body
        this->dev_manager.broadcast_blocking (ser_body, sizeof (body_t));
        // ACK from READ received
        auto ack_body = this->dev_manager.listen_body_block (port_name);

        this->logger.log_mem_read (board_id,
                                   fmt::format ("0x{:08x}", mem_address));

        delete ser_header;
        delete ser_body;

        bool exists = this->db_manager.reference_present (
            board_id, fmt::format ("0x{:08x}", mem_address));
        bson_doc body_doc = this->db_manager.body_to_doc (ack_body);

        if (exists)
          {
            this->db_manager.insert_one (body_doc, "samples");
          }
        else
          {
            this->db_manager.insert_one (body_doc, "references");
          }

        bpt::ptree root;
        std::stringstream ss;

        root.put ("board_id", board_id);
        root.put ("mem_address", fmt::format ("0x{:08x}", mem_address));
        root.put ("message", "region of memory read");

        bpt::ptree data_arr;
        for (int byte = 0; byte < 512; ++byte)
          {
            bpt::ptree node;
            node.put ("", (unsigned)ack_body.data[byte]);
            data_arr.push_back (std::make_pair ("", node));
          }

        root.add_child ("data", data_arr);

        bpt::json_parser::write_json (ss, pt, true);
        res.set_status (200);

        res << ss.str ();
      });

  /// Write to memory the opposite values from the reference sample
  mux.handle ("/commands/write_invert")
      .post ([this] (served::response &res, const served::request &req) {
        std::stringstream input;
        bpt::ptree pt;

        uint16_t mem_address;
        std::string board_id;
        std::string port_name;

        // Parameter parsing
        try
          {
            input << req.body ();
            bpt::json_parser::read_json (input, pt);

            board_id = pt.get<std::string> ("board_id");
            mem_address = pt.get<uint16_t> ("mem_address");
            port_name = pt.get<std::string> ("port_name");
          }
        catch (std::exception &e)
          {
            std::cout << e.what () << "\n";
          }

        uint32_t bid_high = stoul (board_id.substr (2, 8), 0, 16);
        uint32_t bid_medium = stoul (board_id.substr (10, 8), 0, 16);
        uint32_t bid_low = stoul (board_id.substr (18, 8), 0, 16);

        header_t write_header = {
          .type = (uint8_t)header_type::WRITE,
          .TTL = 0,
          .CRC = 0x34,
          .bid_high = bid_high,
          .bid_medium = bid_medium,
          .bid_low = bid_low,
        };
        auto ser_header = new uint8_t[sizeof (header_t)];
        memcpy (ser_header, &write_header, sizeof (header_t));

        // Send WRITE header
        this->dev_manager.broadcast_blocking (ser_header, sizeof (header_t));
        // ACK from WRITE received
        this->dev_manager.listen_headers_block (1);

        body_t write_body = { .type = (uint8_t)body_type::MEMORY,
                              .CRC = 0x50,
                              .bid_high = bid_high,
                              .bid_medium = bid_medium,
                              .bid_low = bid_low,
                              .mem_address = mem_address,
                              .data = { 0 } };

        // TODO: Error when no array is found
        std::vector<uint8_t> bytes = this->db_manager.get_data_vector (
            board_id, fmt::format ("0x{:08x}", mem_address));
        std::vector<uint8_t> inverted_bytes = invert_bytes_arr (bytes);

        for (int b = 0; b < 512; ++b)
          {
            write_body.data[b] = inverted_bytes[b];
          }

        auto ser_body = new uint8_t[sizeof (body_t)];
        memcpy (ser_body, &write_body, sizeof (body_t));

        // Send WRITE body
        std::cout << "SEND WRITE BODY\n";
        this->dev_manager.broadcast_blocking (ser_body, sizeof (body_t));

        this->logger.log_mem_write (board_id,
                                    fmt::format ("0x{:08x}", mem_address));

        delete ser_header;
        delete ser_body;

        bpt::ptree root;
        std::stringstream ss;

        root.put ("board_id", board_id);
        root.put ("mem_address", fmt::format ("0x{:08x}", mem_address));
        root.put ("message", "region of memory written");

        bpt::json_parser::write_json (ss, root, true);
        res.set_status (200);

        res << ss.str ();
      });

  served::net::server server (host, port, this->mux);
  std::cout << "Server listening on " << host << ":" << port << "\n";

  server.run (NUM_THREADS_API);

  return (EXIT_SUCCESS);
}

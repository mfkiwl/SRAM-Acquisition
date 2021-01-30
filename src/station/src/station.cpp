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
        bpt::ptree msg;
        bpt::ptree portArr;
        std::stringstream msg_ss;

        this->dev_manager.register_ports ();
        auto ports = this->dev_manager.available_ports ();
        for (const auto &port : ports)
          {
                  this->logger.log_port_cmd (port, "REGISTERED");
          }

        msg.put ("message", "ports registered");

        bpt::json_parser::write_json (msg_ss, msg, true);

        res.set_status (200);
        res << msg_ss.str ();
      });

  mux.handle ("/ports/available")
      .get ([this] (served::response &res, const served::request &) {
        bpt::ptree msg;
        bpt::ptree portArr;
        std::stringstream msg_ss;

        auto ports = dev_manager.available_ports ();

        std::transform (ports.begin (), ports.end (),
                        std::back_inserter (portArr),
                        [] (std::string p) -> bpt::ptree::value_type {
                          return bpt::ptree::value_type ("", p);
                        });

        msg.add_child ("ports", portArr);

        bpt::json_parser::write_json (msg_ss, msg, true);

        res.set_status (200);
        res << msg_ss.str ();
      });

  mux.handle ("/devices/register")
      .get ([this] (served::response &res, const served::request &) {
        bpt::ptree msg;
        std::stringstream msg_ss;

        this->dev_manager.register_devices ();
        this->logger.log_command ("devices", "register");

        msg.put ("message", "devices registered");

        bpt::json_parser::write_json (msg_ss, msg, true);

        res.set_status (200);
        res << msg_ss.str ();
      });

  mux.handle ("/devices/available")
      .get ([this] (served::response &res, const served::request &) {
        bpt::ptree msg;
        bpt::ptree portArr;
        std::stringstream msg_ss;

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
        msg.add_child ("ports", portArr);

        bpt::json_parser::write_json (msg_ss, msg, true);

        res.set_status (200);
        res << msg_ss.str ();
      });

  mux.handle ("/devices/poweron")
      .get ([this] (served::response &res, const served::request &) {
        bpt::ptree msg;
        std::stringstream msg_ss;

        this->dev_manager.power_on ();
        this->logger.log_power_cycle ("ON", "All");

        msg.put ("message", "all ports powered on");
        bpt::json_parser::write_json (msg_ss, msg, true);

        res.set_status (200);
        res << msg_ss.str ();
      });

  mux.handle ("/devices/poweroff")
      .get ([this] (served::response &res, const served::request &) {
        bpt::ptree msg;
        std::stringstream msg_ss;

        this->dev_manager.power_off ();
        this->logger.log_power_cycle ("OFF", "ALL");

        msg.put ("message", "all ports powered off");
        bpt::json_parser::write_json (msg_ss, msg, true);

        res.set_status (200);
        res << msg_ss.str ();
      });

  mux.handle ("/commands/read")
      .post ([this] (served::response &res, const served::request &req) {
        bpt::ptree msg, input_pt;
        std::stringstream msg_ss, input_ss;

        uint16_t address_offset, mem_address;
        std::string address_str, board_id, port_name;

        try
          {
            input_ss << req.body ();
            bpt::json_parser::read_json (input_ss, input_pt);

            board_id = input_pt.get<std::string> ("board_id");
            address_offset = input_pt.get<uint16_t> ("address_offset");
            port_name = input_pt.get<std::string> ("port_name");
          }
        catch (std::exception &e)
          {
            msg.put ("message", e.what ());
            bpt::json_parser::write_json (msg_ss, msg, true);

            res.set_status (400);
            res << msg_ss.str ();
            return;
          }

        uint32_t bid_high = stoul (board_id.substr (2, 8), 0, 16);
        uint32_t bid_medium = stoul (board_id.substr (10, 8), 0, 16);
        uint32_t bid_low = stoul (board_id.substr (18, 8), 0, 16);
        mem_address = address_offset * PAYLOAD_SIZE;
        address_str = fmt::format ("0x{:08x}", mem_address);

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

        this->dev_manager.broadcast_blocking (ser_header, sizeof (header_t));
        this->dev_manager.listen_headers_block (1);

        body_t read_body = { .type = (uint8_t)body_type::MEMORY,
                             .CRC = 0x69,
                             .bid_high = bid_high,
                             .bid_medium = bid_medium,
                             .bid_low = bid_low,
                             .address_offset = address_offset,
                             .data = { 0 } };

        auto ser_body = new uint8_t[sizeof (body_t)];
        memcpy (ser_body, &read_body, sizeof (body_t));

        this->dev_manager.broadcast_blocking (ser_body, sizeof (body_t));
        auto ack_body = this->dev_manager.listen_body_block (port_name);

        this->logger.log_dev_cmd(board_id, "READ", address_str);

        delete ser_header;
        delete ser_body;

        bool exists
            = this->db_manager.reference_present (board_id, address_str);
        bson_doc body_doc = this->db_manager.body_to_doc (ack_body);

        if (exists)
          {
            this->db_manager.insert_one (body_doc, "samples");
          }
        else
          {
            this->db_manager.insert_one (body_doc, "references");
          }

        std::stringstream data;

        msg.put ("board_id", board_id);
        msg.put ("mem_address", address_str);
        msg.put ("message", "region of memory read");

        for (int byte = 0; byte < 511; ++byte)
          {
            data << (int)ack_body.data[byte] << ",";
          }
        data << (int)ack_body.data[511];
        msg.put ("data", data.str ());

        bpt::json_parser::write_json (msg_ss, msg, true);

        res.set_status (200);
        res << msg_ss.str ();
      });

  mux.handle ("/commands/write_invert")
      .post ([this] (served::response &res, const served::request &req) {
        bpt::ptree msg, input_pt;
        std::stringstream msg_ss, input_ss;

        uint16_t address_offset, mem_address;
        std::string address_str, board_id, port_name;

        try
          {
            input_ss << req.body ();
            bpt::json_parser::read_json (input_ss, input_pt);

            board_id = input_pt.get<std::string> ("board_id");
            address_offset = input_pt.get<uint16_t> ("address_offset");
            port_name = input_pt.get<std::string> ("port_name");
          }
        catch (std::exception &e)
          {
            msg.put ("message", e.what ());
            bpt::json_parser::write_json (msg_ss, msg, true);

            res.set_status (400);
            res << msg_ss.str ();
            return;
          }

        uint32_t bid_high = stoul (board_id.substr (2, 8), 0, 16);
        uint32_t bid_medium = stoul (board_id.substr (10, 8), 0, 16);
        uint32_t bid_low = stoul (board_id.substr (18, 8), 0, 16);
        mem_address = address_offset * PAYLOAD_SIZE;
        address_str = fmt::format ("0x{:08x}", mem_address);

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

        this->dev_manager.broadcast_blocking (ser_header, sizeof (header_t));
        this->dev_manager.listen_headers_block (1);

        body_t write_body = { .type = (uint8_t)body_type::MEMORY,
                              .CRC = 0x50,
                              .bid_high = bid_high,
                              .bid_medium = bid_medium,
                              .bid_low = bid_low,
                              .address_offset = mem_address,
                              .data = { 0 } };

        std::vector<uint8_t> bytes
            = this->db_manager.get_data_vector (board_id, address_str);

        if (bytes.size () == 0)
          {
            msg.put ("message",
                     "There is no reference sample with this criteria.");
            bpt::json_parser::write_json (msg_ss, msg, true);

            res.set_status (400);
            res << msg_ss.str ();
            return;
          }

        std::vector<uint8_t> inverted_bytes = invert_bytes_arr (bytes);

        for (int b = 0; b < 512; ++b)
          {
            write_body.data[b] = inverted_bytes[b];
          }

        auto ser_body = new uint8_t[sizeof (body_t)];
        memcpy (ser_body, &write_body, sizeof (body_t));

        this->dev_manager.broadcast_blocking (ser_body, sizeof (body_t));

        this->logger.log_dev_cmd(board_id, "WRITE", address_str);

        delete ser_header;
        delete ser_body;

        msg.put ("board_id", board_id);
        msg.put ("mem_address", address_str);
        msg.put ("message", "region of memory written");

        bpt::json_parser::write_json (msg_ss, msg, true);

        res.set_status (200);
        res << msg_ss.str ();
      });

  served::net::server server (host, port, this->mux);
  std::cout << "Server listening on " << host << ":" << port << "\n";

  server.run (NUM_THREADS_API);

  return (EXIT_SUCCESS);
}

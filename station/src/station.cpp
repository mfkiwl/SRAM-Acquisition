#include <string>

#include <served/served.hpp>

#include "include/station.hpp"

namespace bpt = boost::property_tree;

int
Station::run (const std::string &host, const std::string &port)
{

  mux.handle ("/ports/register")
      .get ([this] (served::response &res, const served::request &) {
        bpt::ptree root;
        bpt::ptree portArr;
        std::stringstream ss;

        dev_manager.register_ports ();
        auto ports = dev_manager.available_ports ();
        std::transform(ports.begin(), ports.end(),
                       std::back_inserter(portArr),
                       [](std::string p) -> bpt::ptree::value_type {
                               return bpt::ptree::value_type("", p);
                       });
        // for (auto &p : ports)
        //   portArr.push_back (bpt::ptree::value_type ("", p));
        root.add_child ("ports", portArr);

        res.set_status (200);

        bpt::json_parser::write_json (ss, root, true);
        res << ss.str ();
      });

  mux.handle ("/ports/available")
      .get ([this] (served::response &res, const served::request &) {
        bpt::ptree root;
        bpt::ptree portArr;
        std::stringstream ss;

        auto ports = dev_manager.available_ports ();

        std::transform(ports.begin(), ports.end(),
                       std::back_inserter(portArr),
                       [](std::string p) -> bpt::ptree::value_type {
                               return bpt::ptree::value_type("", p);
                       });

        // for (auto &p : ports)
        //   portArr.push_back (bpt::ptree::value_type ("", p));

        root.add_child ("ports", portArr);
        res.set_status (200);

        bpt::json_parser::write_json (ss, root, true);
        res << ss.str ();
      });

  mux.handle ("/devices/available")
      .get ([this] (served::response &res, const served::request &) {
        bpt::ptree root;
        bpt::ptree portArr;
        std::stringstream ss;

        auto device_tree = dev_manager.device_tree ();

        for (const auto& [port, devices] : device_tree) {
                bpt::ptree devArr;
                std::transform(devices.begin(), devices.end(),
                               std::back_inserter(devArr),
                               [](device_t d) -> bpt::ptree::value_type {
                                       return bpt::ptree::value_type("", d.board_id);
                               });
                portArr.add_child (port, devArr);
        }
        root.put_child("ports", portArr);

        res.set_status (200);

        bpt::json_parser::write_json (ss, root, true);
        res << ss.str ();
      });

  mux.handle ("/packets/ping/{bid:0x[A-Z0-9]{22}}")
      .get ([this] (served::response &res, const served::request &) {
        res.set_status (200);

        res << "Ping to device " << req.params["bid"];
      });

  mux.handle ("/packets/ping")
      .get ([this] (served::response &res, const served::request &) {
        res.set_status (200);

        res << "Global ping";
      });

  served::net::server server (host, port, this->mux);
  std::cout << "Server listening on " << host << ":" << port << "\n";

  server.run (NUM_THREADS_API);

  return (EXIT_SUCCESS);
}

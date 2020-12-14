#pragma once

#include <chrono>
#include <string>

#include <fmt/core.h>

#include <boost/optional.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>

using namespace bsoncxx;

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

#include "include/packet.hpp"

class DBManager
{
private:
  /// TODO: Use Pool instead of client. See
  /// https://stackoverflow.com/questions/41249065/mongocxx-3-1-0-how-can-i-close-connection
  mongocxx::instance instance{};
  mongocxx::uri client_uri;
  mongocxx::client client;
  mongocxx::database db;

  std::string collection;

public:
  DBManager ();
  DBManager (const std::string &uri, const std::string &db_name,
             const std::string &collection_name);
  ~DBManager (){};

  void init ();
  void stop ();

  // Utils
  bsoncxx::document::value header_to_document (const header_t &header);
  bsoncxx::document::value body_to_document (const body_t &body);

  boost::optional<mongocxx::v_noabi::result::insert_one>
  insert_document (const bsoncxx::document::value &doc);
};

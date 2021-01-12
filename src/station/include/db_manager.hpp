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
using namespace bsoncxx::document;

using bson_doc = bsoncxx::builder::basic::document;
using bson_value = bsoncxx::document::value;

using MaybeResult = boost::optional<mongocxx::result::insert_one>;

#include "include/packet.hpp"

class DBManager
{
private:
  // TODO: Use Pool instead of client in the future.
  // See
  // https://stackoverflow.com/questions/41249065/mongocxx-3-1-0-how-can-i-close-connection
  mongocxx::instance instance{};
  mongocxx::uri client_uri;
  mongocxx::client client;
  mongocxx::database db;

public:
  DBManager ();
  DBManager (const std::string &uri, const std::string &db_name);
  ~DBManager (){};

  /// Conversion functions
  bson_doc header_to_doc (const header_t &header);
  bson_doc body_to_doc (const body_t &body);

  // TODO: Implement in the future
  // Store only the index of the bits that change
  //   bson_doc bit_flip_to_doc (const body_t &body);

  /// Utilities
  MaybeResult insert_one (const bson_doc &doc, const std::string &coll_name);

  bool reference_present (const std::string &board_id,
                          const std::string &mem_address);
  std::vector<uint8_t>
        get_data_vector(const std::string &board_id, const std::string &mem_address);
};

std::vector<uint8_t>
bytes_to_bitarr (std::vector<uint8_t> &bytes);

std::vector<uint8_t>
bitarr_to_bytes (const std::vector<uint8_t> &bit_arr);

std::vector<uint8_t>
invert_bit_arr (const std::vector<uint8_t> &bit_arr);

std::vector<uint8_t>
invert_bytes_arr(std::vector<uint8_t> &bytes);

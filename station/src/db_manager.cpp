#include <iostream>
#include <string>

#include "include/db_manager.hpp"

/// Default constructor
DBManager::DBManager ()
{
  this->client_uri = mongocxx::uri ("mongodb://localhost:27017");
  this->client = mongocxx::client (client_uri);
  this->db = client["SRAM"];

  this->collection = "Samples";
}

/// Parametrized constructor
DBManager::DBManager (const std::string &uri, const std::string &db_name,
                      const std::string &collection_name)
{
  this->client_uri = mongocxx::uri (uri);
  this->client = mongocxx::client (client_uri);
  this->db = client[db_name];
  this->collection = collection_name;
}

/// mongocxx::client does not provide a way to close the connection directly.
void
DBManager::stop ()
{
  std::cout << "Closed connection\n";
}

bsoncxx::document::value
header_to_document (const header_t &header)
{
  auto builder = bsoncxx::builder::stream::document{};

  std::string bid = fmt::format ("0x{0:08X}{1:08X}{2:08X}", header.bid_high,
                                 header.bid_medium, header.bid_low);

  auto date = bsoncxx::types::b_date (std::chrono::system_clock::now ());

  bsoncxx::document::value doc
      = builder << "header_type" << header.type << "TTL" << header.ttl
                << "body_length" << header.length << "CRC" << header.crc
                << "board_id" << bid << "creation_time" << date
                << bsoncxx::builder::stream::finalize;

  return doc;
}

bsoncxx::document::value
body_to_document (const body_t &body)
{
  auto builder = bsoncxx::builder::stream::document{};

  std::string bid = fmt::format ("0x{0:08X}{1:08X}{2:08X}", body.bid_high,
                                 body.bid_medium, body.bid_low);

  auto date = bsoncxx::types::b_date (std::chrono::system_clock::now ());

  bsoncxx::document::value doc
      = builder << "body_type"
                << body.type
                // << "body_length" << body.length // TODO: Should be used?
                << "CRC" << body.crc << "board_id" << bid << "creation_time"
                << date << bsoncxx::builder::stream::finalize;

  return doc;
}

boost::optional<mongocxx::v_noabi::result::insert_one>
DBManager::insert_document (const bsoncxx::document::value &doc)
{
  auto coll = this->db[this->collection];

  auto view = doc.view ();
  auto result = coll.insert_one (view);
  return result;
}

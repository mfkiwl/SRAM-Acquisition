#include <iostream>
#include <sstream>
#include <string>

#include "include/db_manager.hpp"
#include "include/packet.hpp"

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::make_document;
using bsoncxx::builder::basic::sub_array;

std::map<uint8_t, std::string> packet_name
    = { { (uint8_t)header_type::ACK, "ACK" },
        { (uint8_t)header_type::PING, "PING" },
        { (uint8_t)header_type::READ, "READ" },
        { (uint8_t)header_type::WRITE, "WRITE" },
        { (uint8_t)header_type::EXEC, "EXEC" },
        { (uint8_t)body_type::MEMORY, "MEMORY" },
        { (uint8_t)body_type::SENSORS, "SENSORS" },
        { (uint8_t)body_type::CODE, "CODE" } };

DBManager::DBManager ()
{
  this->client_uri = mongocxx::uri ("mongodb://localhost:27017");
  this->client = mongocxx::client (client_uri);
  this->db = client["SRAM"];
}

DBManager::DBManager (const std::string &uri, const std::string &db_name)
{
  this->client_uri = mongocxx::uri (uri);
  this->client = mongocxx::client (client_uri);
  this->db = client[db_name];
}

std::vector<uint8_t>
invert_bytes_arr (std::vector<uint8_t> &bytes)
{
  std::vector<uint8_t> inverted (bytes.size ());
  for (size_t b = 0; b < bytes.size (); ++b)
    {
      inverted[b] = bytes[b] ^ 0xFF;
    }
  return inverted;
}

bson_doc
DBManager::header_to_doc (const header_t &header)
{
  auto doc = bson_doc{};

  std::string bid = fmt::format ("0x{0:08X}{1:08X}{2:08X}", header.bid_high,
                                 header.bid_medium, header.bid_low);

  auto date = bsoncxx::types::b_date (std::chrono::system_clock::now ());

  doc.append (kvp ("header_type", packet_name[header.type]));
  doc.append (kvp ("CRC", header.CRC));
  doc.append (kvp ("TTL", header.TTL));
  doc.append (kvp ("board_id", bid));
  doc.append (kvp ("timestamp", date));

  return doc;
}

bson_doc
DBManager::body_to_doc (const body_t &body)
{
  auto doc = bson_doc{};
  auto data_arr = bsoncxx::builder::basic::array{};

  std::string bid = fmt::format ("0x{0:08X}{1:08X}{2:08X}", body.bid_high,
                                 body.bid_medium, body.bid_low);

  auto date = bsoncxx::types::b_date (std::chrono::system_clock::now ());
  auto mem_address
      = fmt::format ("0x{:08x}", body.address_offset * PAYLOAD_SIZE);

  doc.append (kvp ("packet_type", packet_name[body.type]));
  doc.append (kvp ("CRC", body.CRC));
  doc.append (kvp ("board_id", bid));
  doc.append (kvp ("timestamp", date));
  doc.append (kvp ("mem_address", mem_address));

  std::stringstream data_ss;

  for (int byte = 0; byte < 511; ++byte)
    data_ss << (int)body.data[byte] << ",";
  data_ss << (int)body.data[511];

  doc.append (kvp ("data", data_ss.str ()));

  return doc;
}

MaybeResult
DBManager::insert_one (const bson_doc &doc, const std::string &coll_name)
{
  auto coll = this->db[coll_name];

  auto view = doc.view ();
  auto result = coll.insert_one (view);
  return result;
}

bool
DBManager::reference_present (const std::string &board_id,
                              const std::string &mem_address)
{
  auto cursor = db["references"].find (make_document (
      kvp ("board_id", board_id), kvp ("mem_address", mem_address)));
  return std::distance (cursor.begin (), cursor.end ()) > 0;
}

std::vector<uint8_t>
DBManager::get_data_vector (const std::string &board_id,
                            const std::string &mem_address)
{
  std::vector<uint8_t> values;

  auto cursor = this->db["references"].find (make_document (
      kvp ("board_id", board_id), kvp ("mem_address", mem_address)));

  for (auto &doc : cursor)
    {
      bsoncxx::document::element ele = doc["data"];
      std::string data_str = ele.get_utf8 ().value.to_string ();
      std::vector<std::string> split_bytes;

      std::stringstream ss (data_str);
      std::string item;
      while (std::getline (ss, item, ','))
        {
          values.push_back (std::stoi (item));
        }
    }
  return values;
}

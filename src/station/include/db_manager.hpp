/**
 * @file db_manager.hpp
 *
 * @brief Function prototypes for the database manager.
 *
 * This class will manage everything related to storing samples.
 * A document is the name for a data record in MongoDB.
 *
 * @author Sergio Vinagrero (servinagrero)
 */

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

/**
 * @class DBManager
 */
class DBManager
{
private:
  /**
   *
   */
  mongocxx::instance instance{};

  /**
   *
   */
  mongocxx::uri client_uri;

  /**
   *
   */
  mongocxx::client client;

  /**
   *
   */
  mongocxx::database db;

public:
  /**
   * @brief Default constructor.
   */
  DBManager ();

  /**
   * @brief Parametrized constructor.
   *
   * @param uri URI to connect to the database.
   * @param db_name Name of the database.
   *
   * @todo Read config values from file.
   * @todo Authentification to connect to the database.
   */
  DBManager (const std::string &uri, const std::string &db_name);

  /**
   * @brief Default destructor.
   *
   * mongocxx::client does not provide a way to close the connection directly.
   */
  ~DBManager (){};

  /**
   * @brief Convert a header into a document.
   *
   * @param header The header to be converted.
   * @returns The mongodb document.
   */
  bson_doc header_to_doc (const header_t &header);

  /**
   * @brief Convert a body into a document.
   *
   * For bodies carrying memory information, the data gets converted
   * into a string with comma separated values to preserve space.
   *
   * @param body The body to be converted.
   * @returns The mongodb document.
   */
  bson_doc body_to_doc (const body_t &body);

  /**
   * @brief Insert one document in the database.
   *
   * @param doc The document to store.
   * @param coll_name The collection to stored the document into.
   * @returns Optional value with the inserted id or nothing.
   */
  MaybeResult insert_one (const bson_doc &doc, const std::string &coll_name);

  /**
   * @brief Check if a reference sample already exists.
   *
   * The reference sample is the first sample that is taken
   * from each board.
   *
   * @param board_id Hex string with the board id.
   * @param mem_address Hex string with the memory address of the sample.
   * @returns True if the documents exists.
   */
  bool reference_present (const std::string &board_id,
                          const std::string &mem_address);

  /**
   * @brief Get the data from one document.
   *
   * @param board_id Hex string with the board id.
   * @param mem_address Hex string with the memory address of the sample.
   * @returns Vector with the bytes in the sample.
   */
  std::vector<uint8_t> get_data_vector (const std::string &board_id,
                                        const std::string &mem_address);
};

/**
 * @brief Invert the values of an array.
 *
 * @param bytes Vector with values to be inverted.
 * @returns Vector with the values inverted.
 */
std::vector<uint8_t> invert_bytes_arr (std::vector<uint8_t> &bytes);

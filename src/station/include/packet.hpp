/**
 * @file packet.hpp
 *
 * @brief Packet definition.
 *
 * Packet can describe both a header or a body.
 *
 * @author Sergio Vinagrero (servinagrero)
 */

#pragma once

#include <cstdint>
#include <map>

#include <fmt/core.h>
#include <fmt/format.h>

/**
 * Size of the data in the body.
 *
 * A body is ment to transmit information for devices to read or write from/to
 * memory. This is the fixed amount of bytes to transmit in the body along with
 * the metadata.
 */
#define PAYLOAD_SIZE 512

/**
 * Operations that can be carried out.
 *
 * These values are used in arguments to various functions in this package.
 * Unless otherwise stated, these functions do not validate whether the data
 * set makes sense in the "from" coordinates.
 */
enum class header_type : uint8_t
{
  /// Acknowledgment from a packet received.
  ACK = 1,
  /// Discover devices connected into a chain.
  PING = 2,
  /// Read a region of memory.
  READ = 3,
  /// Write to a region of memory.
  WRITE = 4,
  /// Execute code stored in memory.
  EXEC = 5,
  /// Error during the communication.
  ERR = 255
};

// Type of data to be read from devices
/**
 * Type of information to be carried out in a body.
 *
 * These values are used in arguments to various functions in this package.
 * Unless otherwise stated, these functions do not validate whether the data
 * set makes sense in the "from" coordinates.
 */
enum class body_type : uint8_t
{
  /// Read or write values from/to memory.
  MEMORY = 6,

  /// Data from sensors. In this case voltage and temperature.
  SENSORS = 7,

  /// Compiled code to be read or written from/to memory
  CODE = 8,
};

/**
 * Type of information to be carried out in a body.
 */
typedef struct header_t
{
  /**
   * Type of operation to be carried out.
   *
   * @see header_type
   */
  uint8_t type;

  /**
   * Counter to represent the position of the device in the chain.
   *
   * This counter only gets incremented during the ping operation when going
   * down the chain. For the rest of operations this value is fixed to 0.
   *
   * Even if it's called TTL, this counter starts from 0 at the station, such
   * that the first device in the chain has a counter of 1.
   */
  uint8_t TTL;

  /**
   * Error Correction Code to check the integrity of the header
   */
  uint8_t CRC;

  /**
   * Upper 32 bits to represent the ID of the device.
   *
   * Devices from ST have a 96 bits identifier split into three 32 bits
   * identifiers, which provide some metadata of the device. This 96 bits are
   * split here in three parts too for easier handling of metadata.
   *
   * The ID contains the following metadata:
   * - 8 bits of unsigned “Wafer number”.
   * - 7 bytes of ASCII Lot number.
   * - 4 bytes of X/Y wafer coordinates in BCD.
   */
  uint32_t bid_high;

  /**
   * Medium 32 bits to represent the ID of the device.
   *
   * @see bid_high
   */
  uint32_t bid_medium;

  /**
   * Lower 32 bits to represent the ID of the device.
   *
   * @see bid_high
   */
  uint32_t bid_low;
} __attribute__ ((packed)) header_t;

/**
 * String formatting for headers.
 *
 * Used for debugging and logging purposes
 */
template <> struct fmt::formatter<header_t>
{
  template <typename ParseContext>
  constexpr auto
  parse (ParseContext &ctx)
  {
    return ctx.begin ();
  }

  template <typename FormatContext>
  auto
  format (const header_t &h, FormatContext &ctx)
  {
    auto bid = fmt::format ("0x{0:08X}{1:08X}{2:08X}", h.bid_high,
                            h.bid_medium, h.bid_low);
    return format_to (ctx.out (), "[{:d}, {:d}, {:#x}, {}]", h.type, h.TTL,
                      h.CRC, bid);
  }
};

/**
 * Lower 32 bits to represent the ID of the device.
 *
 * Used for debugging and logging purposes
 */
typedef struct body_t
{
  /**
   * Type of data the body contains.
   *
   * @see body_type
   */
  uint8_t type;

  /**
   * Error correction code for the communication.
   * It is crucial that not a single bit is change in the data
   * when transmitting the memory from the boards nor when
   */
  uint8_t CRC;

  /**
   * Upper 32 bits to represent the ID of the device.
   */
  uint32_t bid_high;

  /**
   * Medium 32 bits to represent the ID of the device.
   *
   * @see bid_high
   */
  uint32_t bid_medium;

  /**
   * Lower 32 bits to represent the ID of the device.
   *
   * @see bid_high
   */
  uint32_t bid_low;

  /**
   * Offset for memory access to read.
   * Real memory address is address_off * PAYLOAD_SIZE.
   */
  uint16_t address_offset;

  /**
   * Data from the device memory or payload to write to memory.
   *
   * @see PAYLOAD_SIZE
   */
  uint8_t data[PAYLOAD_SIZE] = { 0 };
} __attribute__ ((packed)) body_t;

/**
 * String formatting for bodies.
 *
 * Used for debugging and logging purposes
 */
template <> struct fmt::formatter<body_t>
{
  template <typename ParseContext>
  constexpr auto
  parse (ParseContext &ctx)
  {
    return ctx.begin ();
  }

  template <typename FormatContext>
  auto
  format (const body_t &b, FormatContext &ctx)
  {
    auto bid = fmt::format ("0x{0:08X}{1:08X}{2:08X}", b.bid_high,
                            b.bid_medium, b.bid_low);
    std::string data;
    std::string mem_address = b.address_offset * PAYLOAD_SIZE;

    switch (b.type)
      {
      case (int)body_type::MEMORY:
        data = fmt::format ("AT 0x{:08x}", mem_address);
        break;
      case (int)body_type::SENSORS:
        data = fmt::format ("SENSORS");
        break;
      case (int)body_type::CODE:
        data = fmt::format ("CODE");
        break;
      }
    return format_to (ctx.out (), "[{:d}, {:d}, {}, {}]", b.type, b.CRC, data,
                      bid);
  }
};

/**
 * @brief Compute the CRC-16 of some data.
 *
 * @param buf buffer to read the data from.
 * @param len size of the buffer.
 *
 * @return CRC-16 of the buffer.
 */
uint16_t compute_crc (const uint8_t *buf, const size_t &len);

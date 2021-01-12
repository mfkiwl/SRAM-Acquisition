#pragma once

#include <cstdint>
#include <map>

#include <fmt/core.h>
#include <fmt/format.h>

/// Operations that can be carried out
enum class header_type : uint8_t
{
  ACK = 1,
  PING = 2,
  READ = 3,
  WRITE = 4,
  EXEC = 5,
};


// Type of data to be read from devices
enum class body_type : uint8_t
{
  MEMORY = 6,

  // TODO: Implement in the future
  SENSORS = 7,
  CODE = 8,
};



typedef struct header_t
{
  uint8_t type;
  uint8_t TTL;
  uint8_t CRC;

  // The full ID is 96 bits, too large to fit into a single number
  // Could be managed in one array but the id is stored in 32 ints
  // inside the boards
  uint32_t bid_high;
  uint32_t bid_medium;
  uint32_t bid_low;
} __attribute__ ((packed)) header_t;

/// Header formatting for pretty printing
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

typedef struct body_t
{
  uint8_t type;
  uint8_t CRC;

  uint32_t bid_high;
  uint32_t bid_medium;
  uint32_t bid_low;

  /// Every body is has 512 bytes of data
  /// TODO: Implement this properly for various types of data
  uint16_t mem_address;
  uint8_t data[512] = { 0 };
} __attribute__ ((packed)) body_t;

/// Header formatting for pretty printing
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

    switch (b.type)
      {
      case (int)body_type::MEMORY:
        data = fmt::format ("AT 0x{:08x}", b.mem_address);
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

#pragma once

#include <cstdint>

#include <fmt/core.h>
#include <fmt/format.h>

enum class header_type : uint8_t
{
  ACK = 1,
  PING = 2,
  READ = 3,
  WRITE = 4,
  EXEC = 5,
};

enum class body_type : uint8_t
{
  MEMORY = 6,
  SENSORS = 7,
  CODE = 8,
};

typedef struct header_t
{
  uint8_t type;
  uint8_t ttl;
  uint16_t length;
  uint8_t crc;
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
    std::string bid = fmt::format ("0x{0:08X}{1:08X}{2:08X}", h.bid_high,
                                   h.bid_medium, h.bid_low);
    return format_to (ctx.out (), "[{:d}, {:d}, {:d}B, {:#x}, {}]", h.type,
                      h.ttl, h.length, h.crc, bid);
  }
};

typedef struct body_t
{
  // 112 bits = 14 bytes
  uint8_t type;
  uint8_t crc;
  uint32_t bid_high;
  uint32_t bid_medium;
  uint32_t bid_low;

  uint8_t *data;
} __attribute__ ((packed)) body_t;

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
    std::string bid = fmt::format ("0x{0:08X}{1:08X}{2:08X}", b.bid_high,
                                   b.bid_medium, b.bid_low);
    std::string data;

    switch (b.type)
      {
      case (int)body_type::MEMORY:
        data = fmt::format ("");
        break;
      case (int)body_type::SENSORS:
        data = fmt::format ("");
        break;
      case (int)body_type::CODE:
        data = fmt::format ("");
        break;
      }
    return format_to (ctx.out (), "[{:d}, {:d}, {}, {}]", b.type, b.crc, data,
                      bid);
  }
};

/* class Header */
/* { */
/* public: */
/*   header_t data; */
/*   Header () */
/*   { */
/*     this->binary = std::unique_ptr<char[]> (new char (sizeof (this->data)));
 */
/*   } */

/*   ~Header () {} */

/*   void */
/*   serialize () override */
/*   { */
/*     memcpy (this->binary.get (), &this->data, sizeof (this->data)); */
/*   } */

/*   void */
/*   deserialize () override */
/*   { */
/*     memcpy (&this->data, this->binary.get (), sizeof (this->data)); */
/*   } */
/* }; */

/* class Body : public Packet */
/* { */
/* public: */
/*   body_t data; */

/*   Body () */
/*   { */
/*     this->binary = std::unique_ptr<char[]> (new char (sizeof (this->data)));
 */
/*   } */

/*   ~Body () {} */

/*   void */
/*   serialize () override */
/*   { */
/*     memcpy (this->binary.get (), &this->data, sizeof (this->data)); */
/*   } */

/*   void */
/*   deserialize () override */
/*   { */
/*     memcpy (&this->data, this->binary.get (), sizeof (this->data)); */
/*   } */
/* }; */

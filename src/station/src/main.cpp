#include <algorithm>

#include "include/db_manager.hpp"
#include "include/station.hpp"

int
main ()
{
  Station station;
  station.run ("127.0.0.1", "8123");

  // auto doc = db_manager.body_to_reference (b);
  // db_manager.insert_reference (doc);

  /// To test this operation we need the folowing steps
  /// 1. Register devices using ping protocol
  /// 2. Read reference samples for the full memory for every board
  /// 3. Send inverted memory (from the reference sample) to half the boards
  ///   3.1 Keep a margin of 3K at the start and at the bottom of the memory
  /// 4. Power off for 5 minutes
  /// 5.
}

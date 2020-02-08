
#ifndef COMMON_H
#define COMMON_H

#include <cstdint>
#include <string>

namespace networking
{

/**
 *  An identifier for a Client connected to a Server. The ID of a Connection is
 *  guaranteed to be unique across all actively connected Client instances.
 */
struct Connection {
  uintptr_t id;

  bool
  operator==(Connection other) const {
    return id == other.id;
  }
};

struct ConnectionHash {
  size_t
  operator()(Connection c) const {
    return std::hash<decltype(c.id)>{}(c.id);
  }
};


/**
 *  A Message containing text that can be sent to or was recieved from a given
 *  Connection.
 */
struct Message {
  Connection connection;
  std::string text;
};

} // namespace networking

#endif
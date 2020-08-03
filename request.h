#ifndef REQUEST_H
#define REQUEST_H

#include <cstdint>
#include <iostream>

typedef uint64_t IdType;

// Request information
class SimpleRequest {
private:
  IdType _id;     // request object id
  uint64_t _size; // request size in bytes

public:
  SimpleRequest() {}
  virtual ~SimpleRequest() {}

#endif /* REQUEST_H */

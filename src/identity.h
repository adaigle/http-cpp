#ifndef IDENTITY_H
#define IDENTITY_H

#include "zmq_utility.hpp"

static constexpr size_t IDENTITY_CAPACITY = 256;
using identity_t = zmq_identity<IDENTITY_CAPACITY>;

#endif

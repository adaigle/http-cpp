# Notes on zeroMQ

A quick reference to zeroMQ'as sockets types.
* [REQ/REP](#routing-envelope-for-reqrep)
* [ROUTER](#routing-envelope-for-router)
* [DEALER](#routing-envelope-for-dealer)
* [PUB/SUB](#routing-envelope-for-pubsub)
* [PUSH/PULL](#routing-envelope-for-pushpull)

For the envelope frame, we use `n` as the lenght of the content of each frame.

## Routing envelope for REQ/REP

A REQ socket automatically adds an identity frame followed by an empty frame to the message and a REP sockets expects the same format and removes it automatically.

```cpp
void* socket = zmq_socket(context, ZMQ_REQ);
zmq_bind(socket, "...");
// Sending data, REQ socket adds the identity and empty frame.
zmq_send(socket, "data", 4, 0);
```

```
Frame 1: [n] identity   (added by REQ / remove by REP)
Frame 2: [0]            (added by REQ / remove by REP)
Frame 3: [n] data
```

```cpp
void* socket = zmq_socket(context, ZMQ_REP);
zmq_connect(socket, "...");
// Receiving data, REP socket removes the identity and empty frame.
std::array<char, 4> buffer;
zmq_recv(socket, &buffer[0], 4, 0); // buffer contains "data".
```

When mixing `REQ -> ROUTER` or `DEALER -> REP`, we need to be careful in the processing of the messages since REQ automatically adds the identity frame and REP expects it. So ROUTER has to receive the request properly and DEALER has to manually add the identity and empty frame.

## Routing envelope for ROUTER

A ROUTER socket append an identity frame to received message and expects an identity frame when sending message to determine the identity of the client.

```
Frame 1: [n] identity   (added by ROUTER on receiving / remove by ROUTER on sending)
Frame 2: [n] data
```

```cpp
void* socket = zmq_socket(context, ZMQ_ROUTER);
zmq_bind(socket, "...");

// Receive the identity of a client and a request.
std::array<char, 256> identity;
std::array<char, 256> request;
zmq_recv(socket, &identity[0], 256, 0);
zmq_recv(socket, &request[0], 256, 0);

// Compute a response (potentially asynchronously)...
std::array<char, 256> response;

// Eventually reply to the same client (based on it's identity)
zmq_send(socket, &identity[0], 256, ZMQ_SNDMORE);
zmq_send(socket, &response[0], 256, 0);
```

## Routing envelope for DEALER

A DEALER socket simply forward the messages directly without adding any additional frame. A common pattern is to use a ROUTER socket for client facing server and then passing the identity to subsequent DEALER sockets.

## Routing envelope for PUB/SUB

In PUB/SUB configuration, sending a single message works as there is no identity required. However, a suggested pattern is the following to enable filtering and respond to the publisher.

```cpp
void* socket = zmq_socket(context, ZMQ_PUB);
zmq_bind(socket, "...");
// Sending the subscription key and two additional frames.
zmq_send(socket, "key", 3, ZMQ_SNDMORE);
zmq_send(socket, "sender_address", 14, ZMQ_SNDMORE);
zmq_send(socket, "data", 4, 0);
```

```
Frame 1: [n] key            (used for filtering)
Frame 2: [n] sender address (used for replying on a different socket if needed)
Frame 3: [n] data
```

```cpp
void* socket = zmq_socket(context, ZMQ_SUB);
zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "key", 3);
zmq_connect(socket, "...");
// Receiving data in two frames, the first is discarded for the subscription key.
std::array<char, 14> address;
zmq_recv(socket, &sender_address[0], 14, 0);
std::array<char, 4> data;
zmq_recv(socket, &data[0], 4, 0);
```

## Routing envelope for PUSH/PULL

TODO

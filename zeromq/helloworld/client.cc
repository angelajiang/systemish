#include <iostream>
#include <string>
#include <zmq.hpp>

int main() {
  // Prepare our context and socket
  zmq::context_t context(1);
  zmq::socket_t socket(context, ZMQ_REQ);

  std::cout << "Connecting to hello world server" << std::endl;
  socket.connect("tcp://localhost:5555");

  zmq::message_t request(7);
  memcpy(request.data(), "Request", 5);
  std::cout << "Sending request " << std::endl;
  socket.send(request);

  // Get the reply.
  zmq::message_t reply;
  socket.recv(&reply);
  std::cout << "Received reply " << std::endl;
  return 0;
}

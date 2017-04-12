#include <iostream>
#include <string>
#include <zmq.hpp>

int main() {
  // Prepare our context and socket
  zmq::context_t context(1);
  zmq::socket_t socket(context, ZMQ_REQ);

  socket.connect("tcp://localhost:5555");

  std::cout << "Sending request " << std::endl;
  zmq::message_t request(7);
  memcpy(request.data(), "Request", 5);
  socket.send(request);

  zmq::message_t reply;
  socket.recv(&reply);
  std::cout << "Received reply " << std::endl;
  return 0;
}

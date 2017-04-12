#include <enet/enet.h>
#include <stdio.h>
#include <string.h>

int main() {
  if (enet_initialize() != 0) {
    fprintf(stderr, "An error occurred while initializing ENet.\n");
    return EXIT_FAILURE;
  }

  printf("ENet initialization successful.\n");
  atexit(enet_deinitialize);

  ENetHost* client = enet_host_create(nullptr, 32, 2, 0, 0);
  if (client == nullptr) {
    fprintf(stderr,
            "An error occurred while trying to create an ENet client host.\n");
    exit(EXIT_FAILURE);
  }

  ENetAddress address;
  enet_address_set_host(&address, "localhost");
  address.port = 26000;

  // Initiate the connection, allocating the two channels 0 and 1
  ENetPeer *peer = enet_host_connect(client, &address, 32, 2);
  if (peer == nullptr) {
    fprintf(stderr, "No available peers for initiating an ENet connection.\n");
    exit(EXIT_FAILURE);
  }

  // Enter event loop
  ENetEvent event;
  if (enet_host_service(client, &event, 5000) > 0 &&
      event.type == ENET_EVENT_TYPE_CONNECT) {
    printf("Connection to localhost:1234 succeeded.\n");
  } else {
    enet_peer_reset(peer);
    printf("Connection to localhost:1234 failed.\n");
  }

  const char* hello = "Hello enet";
  ENetPacket* packet =
      enet_packet_create(hello, strlen(hello) + 1, ENET_PACKET_FLAG_RELIABLE);
  enet_peer_send(peer, 0, packet);
  enet_host_service(client, &event, 5000);

  enet_host_destroy(client);
}

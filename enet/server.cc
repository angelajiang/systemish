#include <assert.h>
#include <enet/enet.h>
#include <stdio.h>

#include "common.h"

int main() {
  if (enet_initialize() != 0) {
    fprintf(stderr, "An error occurred while initializing ENet.\n");
    return EXIT_FAILURE;
  }
  printf("ENet initialization successful.\n");
  atexit(enet_deinitialize);

  ENetAddress address;
  enet_address_set_host(&address, "localhost");
  address.host = ENET_HOST_ANY;
  address.port = kUdpPort;

  ENetHost *server =
      enet_host_create(&address, kMaxConnections, kChannels, 0, 0);
  if (server == nullptr) {
    fprintf(stderr,
            "An error occurred while trying to create an ENet server host.\n");
    exit(EXIT_FAILURE);
  }

  printf("Entering event loop.\n");
  ENetEvent event;
  size_t num_pkts_rx = 0;
  while (enet_host_service(server, &event, 100000) > 0) {
    switch (event.type) {
      case ENET_EVENT_TYPE_CONNECT:
        printf("Server: connected from client %u.\n", event.data);
        event.peer->data = new PeerData(event.data);
        break;

      case ENET_EVENT_TYPE_RECEIVE:
        printf("Server: Received pkt of length %zu from client %u.\n",
            event.packet->dataLength,
            static_cast<PeerData *>(event.peer->data)->id);

        enet_packet_destroy(event.packet);
        num_pkts_rx++;
        if (num_pkts_rx >= 80) {
          exit(0);
        }
        break;

      case ENET_EVENT_TYPE_DISCONNECT:
        printf("Disconnected.\n");
        event.peer->data = nullptr;
        break;

      default:
        printf("Unknown type\n");
        break;
    }
  }

  enet_host_destroy(server);
}

#include <enet/enet.h>
#include <stdio.h>
#include <assert.h>

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
  address.port = 26000;

  ENetHost *server = enet_host_create(&address, 32, 2, 0, 0);
  if (server == nullptr) {
    fprintf(stderr,
            "An error occurred while trying to create an ENet server host.\n");
    exit(EXIT_FAILURE);
  }

  printf("Entering event loop.\n");
  ENetEvent event;
  while (enet_host_service(server, &event, 4000) > 0) {
    switch (event.type) {
      case ENET_EVENT_TYPE_CONNECT:
        printf("A new client connected from %x:%u.\n", event.peer->address.host,
               event.peer->address.port);
        assert(false);
        break;

      case ENET_EVENT_TYPE_RECEIVE:
        printf(
            "A packet of length %zu containing %s was received on "
            "channel %u.\n",
            event.packet->dataLength, (char *)event.packet->data,
            event.channelID);
        enet_packet_destroy(event.packet);
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

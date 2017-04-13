#include <assert.h>
#include <enet/enet.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

void run_event_loop_one(ENetHost *client, ENetEvent *event) {
  int ret = enet_host_service(client, event, 0);
  if (ret != 0) {
    // Handle the event
    switch (event->type) {
      case ENET_EVENT_TYPE_DISCONNECT:
        printf("Client: Disconnected. Peer data = %u. Exiting.\n",
               static_cast<PeerData *>(event->peer->data)->id);
        delete static_cast<PeerData *>(event->peer->data);
        exit(0);
        break;

      default:
        printf("Unknown type\n");
        break;
    }
  }
}

int main(int argc, char *argv[]) {
  uint32_t client_id;
  if (argc == 2) {
    client_id = static_cast<uint32_t>(atoi(argv[1]));
  } else {
    client_id = 0;
  }

  if (enet_initialize() != 0) {
    fprintf(stderr, "An error occurred while initializing ENet.\n");
    return EXIT_FAILURE;
  }

  printf("ENet initialization successful.\n");
  atexit(enet_deinitialize);

  ENetHost *client =
      enet_host_create(nullptr, kMaxConnections, kChannels, 0, 0);
  if (client == nullptr) {
    fprintf(stderr,
            "An error occurred while trying to create an ENet client host.\n");
    exit(EXIT_FAILURE);
  }

  ENetAddress address;
  enet_address_set_host(&address, "localhost");
  address.port = kUdpPort;

  // Initiate the connection. Send the client ID as the connection data.
  ENetPeer *peer = enet_host_connect(client, &address, kChannels, client_id);
  if (peer == nullptr) {
    fprintf(stderr, "Client: Failed to connect.\n");
    exit(EXIT_FAILURE);
  }

  peer->data = static_cast<void *>(new PeerData(3185));

  // Reduce timeout for peer failures. This needs work.
  enet_peer_timeout(peer, 1, 1, 1);

  // Run event loop to connect to the peer
  ENetEvent event;
  if (enet_host_service(client, &event, 100) > 0 &&
      event.type == ENET_EVENT_TYPE_CONNECT) {
    printf("Client: Connection to server succeeded.\n");
  } else {
    enet_peer_reset(peer);
    printf("Client: Failed to connect.\n");
    exit(-1);
  }

  while (1) {
    printf("Client: Sending 10 packets.\n");

    for (size_t i = 0; i < 10; i++) {
      // Create a multi-MTU "packet"
      ENetPacket *packet =
          enet_packet_create(nullptr, kDataSize, ENET_PACKET_FLAG_RELIABLE);
      if (packet == nullptr) {
        printf("Client: Failed to create packet. Exiting.\n");
        exit(-1);
      }

      int send_success = enet_peer_send(peer, 0, packet);
      if (send_success < 0) {
        printf("Client: Send failed. Exiting.\n");
        exit(-1);
      }

      // Run the event loop to transmit the packet and initialize
      // peer->reliableDataInTransit.
      run_event_loop_one(client, &event);

      // Run the event loop until until all packets are reliably transmitted
      while (peer->reliableDataInTransit > 0) {
        run_event_loop_one(client, &event);
      }
    }
  }

  enet_host_destroy(client);
}

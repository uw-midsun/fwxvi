/************************************************************************************************
 * @file   mpxe.c
 *
 * @brief  Multi Project x86 Emulation library
 *
 * @date   2025-03-09
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "client_interface.h"
#include "mpxe.h"

static ClientInstance *client = NULL;

void handle_signal(int sig) {
  printf("Signal %d received, cleaning up...\n", sig);

  if (client) {
    client_disconnect(client);
    client_destroy(client);
  }

  exit(0);
}

void mpxe_init(int argc, char *argv[]) {
  /* Default connection parameters */
  const char *server_ip = "127.0.0.1";
  int server_port = 8080;
  const char *client_name = NULL;

  /* Parse command line arguments if provided */
  if (argc >= 2) {
    client_name = argv[1];
  }

  if (argc >= 3) {
    server_port = atoi(argv[2]);
  }

  if (argc >= 4) {
    server_ip = argv[3];
  }

  printf("Starting client application...\n");
  printf("Connecting to server at %s:%d\n", server_ip, server_port);

  /* Set up signal handlers for graceful termination */
  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);

  /* Create the client instance with NULL callbacks - to use the C++ defaults */
  client = client_create(server_ip, server_port, NULL, NULL, NULL);
  if (!client) {
    printf("Failed to create client instance\n");
    return;
  }

  if (client_name != NULL) {
    client_set_name(client, client_name);
  }

  /* Connect to the server */
  int result = client_connect(client);
  if (result != 0) {
    printf("Failed to connect to server, error code: %d\n", result);
    client_destroy(client);
    return;
  }

  printf("Client connected and running. Press Ctrl+C to terminate.\n");
}

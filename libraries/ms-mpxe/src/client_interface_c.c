/************************************************************************************************
 * @file   client_interface.c
 *
 * @brief  C interface file for C++ client class
 *
 * @date   2025-03-09
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "client_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

static void internal_message_callback(void *context, const char *message, int length);
static void internal_connect_callback(void *context);

extern void *client_cpp_create(const char *ip_address, int port, void (*msg_callback)(void *, const char *, int), void (*connect_callback)(void *), void *context);
extern int client_cpp_connect(void *cpp_client);
extern int client_cpp_send_message(void *cpp_client, const char *message, int length);
extern int client_cpp_disconnect(void *cpp_client);
extern void client_cpp_destroy(void *cpp_client);

/** @brief  Structure to hold C++ client instance and callback information */
struct ClientInstance {
  void *cpp_client;             /**< Pointer to C++ Client object */
  ClientMsgCallback msg_cb;     /**< User-provided message callback */
  ClientConnectCallback con_cb; /**< User-provided connect callback */
  void *user_context;           /**< User context pointer for callbacks */
};

ClientInstance *client_create(const char *ip_address, int port, ClientMsgCallback msg_callback, ClientConnectCallback connect_callback, void *context) {
  if (!ip_address) {
    return NULL;
  }

  ClientInstance *instance = (ClientInstance *)malloc(sizeof(ClientInstance));

  instance->msg_cb = msg_callback;
  instance->con_cb = connect_callback;
  instance->user_context = context;

  void (*cpp_msg_callback)(void *, const char *, int) = msg_callback ? internal_message_callback : NULL;
  void (*cpp_connect_callback)(void *) = connect_callback ? internal_connect_callback : NULL;

  instance->cpp_client = client_cpp_create(ip_address, port, cpp_msg_callback, cpp_connect_callback, (void *)instance);

  if (!instance->cpp_client) {
    free(instance);
    return NULL;
  }

  return instance;
}

int client_connect(ClientInstance *instance) {
  if (instance == NULL || instance->cpp_client == NULL) {
    return -1;
  }

  return client_cpp_connect(instance->cpp_client);
}

int client_send_message(ClientInstance *instance, const char *message, int length) {
  if (instance == NULL || instance->cpp_client == NULL || message == NULL || length <= 0) {
    return -1;
  }

  return client_cpp_send_message(instance->cpp_client, message, length);
}

int client_disconnect(ClientInstance *instance) {
  if (instance == NULL || instance->cpp_client == NULL) {
    return -1;
  }

  return client_cpp_disconnect(instance->cpp_client);
}

void client_destroy(ClientInstance *instance) {
  if (instance == NULL) {
    return;
  }

  if (instance->cpp_client != NULL) {
    client_cpp_disconnect(instance->cpp_client);
    client_cpp_destroy(instance->cpp_client);
  }

  free(instance);
}

/* Internal callback function for message events, redirects to user callback */
static void internal_message_callback(void *context, const char *message, int length) {
  ClientInstance *instance = (ClientInstance *)context;
  printf("MESSAGE CALL BACK\n");
  if (instance && instance->msg_cb) {
    instance->msg_cb(instance->user_context, message, length);
  }
}

/* Internal callback function for connect events, redirects to user callback */
static void internal_connect_callback(void *context) {
  ClientInstance *instance = (ClientInstance *)context;
  if (instance && instance->con_cb) {
    instance->con_cb(instance->user_context);
  }
}

#ifdef __cplusplus
}
#endif

#pragma once

/************************************************************************************************
 * @file   client_interface.h
 *
 * @brief  C interface header for C++ client class
 *
 * @date   2025-03-09
 * @author Aryan Kashem
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup MPXE
 * @brief    Multi Project x86 Emulation Client Interface library for C applications
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Handle for client instance */
typedef struct ClientInstance ClientInstance;

/** @brief  Client message callback */
typedef void (*ClientMsgCallback)(void *context, const char *message, int length);

/** @brief  Client connection callback */
typedef void (*ClientConnectCallback)(void *context);

/**
 * @brief   Create a client instance
 * @param   ip_address IP address to connect to
 * @param   port Port number to connect to
 * @param   msg_callback Callback function for received messages
 * @param   connect_callback Callback function for connection events
 * @param   context User context pointer passed to callbacks
 * @return  Pointer to client instance or NULL on failure
 */
ClientInstance *client_create(const char *ip_address, int port, ClientMsgCallback msg_callback, ClientConnectCallback connect_callback, void *context);

/**
 * @brief   Connect to the server
 * @param   instance Client instance returned by client_create
 * @return  0 on success, negative error code on failure
 */
int client_connect(ClientInstance *instance);

/**
 * @brief   Send a message to the server
 * @param   instance Client instance
 * @param   message Message buffer to send
 * @param   length Length of the message in bytes
 * @return  Number of bytes sent or negative error code
 */
int client_send_message(ClientInstance *instance, const char *message, int length);

/**
 * @brief   Set the client name
 * @param   instance Client instance
 * @param   name Pointer to the name buffer
 */
int client_set_name(ClientInstance *instance, const char *name);

/**
 * @brief   Disconnect from the server
 * @param   instance Client instance
 * @return  0 on success, negative error code on failure
 */
int client_disconnect(ClientInstance *instance);

/**
 * @brief   Destroy the client instance and free resources
 * @param   instance Client instance
 */
void client_destroy(ClientInstance *instance);

#ifdef __cplusplus
}
#endif

/** @} */

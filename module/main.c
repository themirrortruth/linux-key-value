/*****************************************************************************
 * File:        main.c                                                       *
 * Description:                                                              *
 * Author:      Erik Ramos <c03ers@cs.umu.se>                                *
 *              Igor Ramazanov <ens17irm@cs.umu.se>                          *
 * Version:     20171006                                                     *
 *****************************************************************************/
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/rwsem.h>
#include "logger.h"
#include "map.h"
#include "nlsocket.h"
#include "storage.h"

#define SHARED_MAP_HEADER 17
#ifndef SHARED_MAP_PROTOCOL
#define SHARED_MAP_PROTOCOL 31
#endif

// @formatter:off
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Shared key-value pair database.");
MODULE_AUTHOR("Erik Ramos <c03ers@cs.umu.se>");
MODULE_AUTHOR("Igor Ramazanov <ens17irv@cs.umu.se>");
MODULE_AUTHOR("Bastien Harendarczyk<ens17bhk@cs.umu.se>");
MODULE_VERSION("0.9");
// @formatter:on

DEFINE_MUTEX(mut);

static void request_handler(pid_t, void *, size_t);
static void handle_insert_request(pid_t, const message_t);
static void handle_lookup_request(pid_t, const message_t);
static void restore_data(const char *, const void *, size_t);

static int __init shared_map_init(void) {

  /* Initialize the hashtable. */
  if (map_init()) {
    logger_error("failed to initialize rhashtable\n");
    return 1;
  }

  /* Try to recover data. */
  if (storage_init(restore_data) || storage_load())
    logger_warn("persistent storage is not available\n");

  /* Initialize Netlink. */
  if (nlsocket_init(SHARED_MAP_PROTOCOL, SHARED_MAP_HEADER, request_handler)) {
    logger_error("failed to initialize Netlink\n");
    return 1;
  }
 
  /* Initialization successful. */
  logger_info("module installed\n");
  return 0;
}

static void __exit shared_map_exit(void) {
  nlsocket_destroy();
  map_destroy();
  logger_info("module removed");
}

/*
 * Handles messages sent to the module's Netlink socket.
 */
static void request_handler(pid_t user, void *data, size_t size) {
  message_t request = message_cast(data);
  if (!request)
    return;

  switch (request->type) {
  case MESSAGE_LOOKUP:
    mutex_lock(&mut);
    handle_lookup_request(user, request);
    mutex_unlock(&mut);
    break;
  case MESSAGE_INSERT:
    mutex_lock(&mut);
    handle_insert_request(user, request);
    mutex_unlock(&mut);
    break;
  default:
    break;
  }
}

/*
 * Handles requests to create a mapping.
 */
void handle_insert_request(pid_t user, const message_t request) {
  message_t response;

  /* Attempt to create the mapping. */
  switch (map_insert(request->key, request->value, request->value_length)) {
  case MAP_INSERT_SUCCESS:

    /* Successful insertion of a new element. */
    response = message_value_inserted();
    break;
  case MAP_INSERT_REPLACED:

    /* Successful replacement of an old element. */
    response = message_value_replaced();
    break;
  default:

    /* Insertion failed. */
    response = message_error();
    return;
  }

  /* Send a response. */
  if (response)
    nlsocket_sendto(user, response, message_length(response));
  message_free(response);

  /* Save the mapping. */
  storage_save(request->key, request->value, request->value_length);
}

void handle_lookup_request(pid_t user, const message_t request) {
  message_t response;
  size_t length;
  void *data;

  /* Insert the value and generate an appropriate response. */
  response = !map_lookup(request->key, &data, &length)
      ? message_lookup_ok(data, length)
      : message_key_not_found();

  /* Send the response. */
  if (response)
    nlsocket_sendto(user, response, message_length(response));
  message_free(response);
}

inline void restore_data(const char *key, const void *value, size_t length) {
  map_insert(key, value, length);
}

module_init(shared_map_init);
module_exit(shared_map_exit);

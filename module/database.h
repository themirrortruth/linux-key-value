/*****************************************************************************
 * File:        database.h                                                   *
 * Description:                                                              *
 * Version:                                                                  *
 *****************************************************************************/
#pragma once

enum database_error {
  DB_INIT_SUCCESS    = 0,
  DB_INIT_NETLINK    = 1,
  DB_INIT_RHASHTABLE = 2
};

int database_init(void);
void database_free(void);
void database_save(void);

/* Technically these don't have to be in the header - just for testing. */
int database_insert(char *key, char *data, size_t length);
int database_lookup(char *ey, char **data, size_t *length);

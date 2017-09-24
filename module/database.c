/*****************************************************************************
 * File:        database.c                                                   *
 * Description:                                                              *
 * Version:                                                                  *
 *****************************************************************************/
#include <linux/rcupdate.h>
#include <linux/rhashtable.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include "database.h"
#include "database_entry.h"
#include "error_code.h"

/* "Private member functions". */
static void database_rhashtable_cleanup(void *, void *);
static int database_key_compare(struct rhashtable_compare_arg *, const void *);

struct database {
  struct rhashtable table;
  struct rhashtable_params params;
  spinlock_t lock;
};

static const struct rhashtable_params params = {
  .nelem_hint = 1024,
  .head_offset = offsetof(struct database_entry, rhash),
  .key_offset = offsetof(struct database_entry, key),
  .key_len = sizeof(char *),
  .max_size = 1048576,
  .min_size = 256,
  .automatic_shrinking = true,
  .obj_cmpfn = database_key_compare
};

static struct database database;

int database_init(void) {

  /* Initialize the rhashtable. */
  if (rhashtable_init(&database.table, &params))
    return DB_INIT_RHASHTABLE;

  spinlock_init(&database.lock);
  return SUCCESS;
}

void database_free(void) {
  /* No destroying of the lock necessary? */
  rhashtable_free_and_destroy(&database.table, database_rhashtable_cleanup, 0);
}

void database_rhashtable_cleanup(void *ptr, void *arg) {
  struct database_entry *entry;
  struct database_entry *next;

  entry = (struct database_entry *) ptr;

  while (entry) {
    next = rcu_access_pointer(entry->next);
    /* TODO free key and data. */
    kfree(entry->key);
    kfree(entry->value);
    kfree_rcu(entry, rcu);
    entry = next;
  }
}

int database_key_compare(struct rhashtable_compare_arg *arg, const void *obj) {
  struct database_entry *entry = (struct database_entry *) obj;
  return strcmp(entry->key, arg->key);
}

int database_insert(char *key, char *value, size_t length) {
  struct rhash_head *head;
  int err;

  spin_lock_lock(&database.lock);

  head = rhashtable_lookup_fast(&database.table, key, params);

  if (!head) {

    /* Insert a new element. */
    /* TODO create new entry. */
    err = rhashtable_insert_fast(&database.rhash, &entry.rhash, params);
  } else {
  }

  spinlock_unlock(&database.lock);
  return 0;
}

int database_lookup(char *key, char **value, size_t *length) {
  *value = NULL;
  *length = 0;
  return 0;
}

inline int database_has_key(const char *key) {
  /* FIXME make sure we hold RCU lock here. */
  /* FIXME this only finds the first element in the list. */
  return (rhashtable_lookup_fast(&database.table, key, params) != NULL);
}

/* TODO actual code here. */
int database_save(void) {
  return 0;
}

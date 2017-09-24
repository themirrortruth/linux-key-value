/*****************************************************************************
 * File:        command.c                                                    *
 * Description:                                                              *
 * Version:                                                                  *
 *****************************************************************************/
#include <linux/slab.h>
#include <linux/string.h>
#include "command.h"

command_t command_new(void) {
  command_t command = (command_t) kzalloc(sizeof(struct command), GFP_KERNEL);
  return command;
}

void command_free(command_t command) {
  kfree(command->key);
  kfree(command->value);
  kfree(command);
}

void command_serialize(command_t command, char *data) {
  char *char_ptr;
  int *int_ptr;
  int i;

  *data = command->operation;
  data++;

  int_ptr = (int *) data;
  *int_ptr = command->key_size;
  int_ptr++;
  *int_ptr = command->value_size;
  int_ptr++;

  char_ptr = (char *) int_ptr;
  i = 0;
  while (i < command->key_size) {
    *char_ptr = command->key[i];
    char_ptr++;
    i++;
  }

  i = 0;
  while (i < command->value_size) {
    *char_ptr = command->value[i];
    char_ptr++;
    i++;
  }
};

void command_deserialize(struct command *command, char *data) {
  char *char_ptr;
  int *int_ptr;
  int i;

  command->operation = *data;
  data++;
  int_ptr = (int *) data;
  command->key_size = *int_ptr;
  int_ptr++;
  command->value_size = *int_ptr;
  int_ptr++;

  command->key = kmalloc(sizeof(char) * (command->key_size), GFP_KERNEL);
  command->value = kmalloc(sizeof(char) * (command->value_size), GFP_KERNEL);
  char_ptr = (char *) int_ptr;

  i = 0;
  while (i < command->key_size) {
    command->key[i] = *char_ptr;
    char_ptr++;
    i++;
  }

  i = 0;
  while (i < command->value_size) {
    command->value[i] = *char_ptr;
    char_ptr++;
    i++;
  }
};
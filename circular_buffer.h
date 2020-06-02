#ifndef CYCLIC_BUFFER_H
#define CYCLIC_BUFFER_H

#define BUFFER_MAX_SIZE  1024

#include <linux/kernel.h>
#include <stdbool.h>


struct buffer
{
  int head;
  int tail;
  bool isfull;
  unsigned char data[BUFFER_MAX_SIZE];
};


void initialize_buffer(struct buffer* buffer);
void clear_buffer(struct buffer* buffer);
void reset_buffer(struct buffer* buffer);
int  push_character(struct buffer* buffer, unsigned char character);
int  pull_character(struct buffer* buffer, unsigned char* character);
int  push_string(struct buffer* buffer, unsigned char* string, unsigned int string_size);
int  get_buffer_room(struct buffer* buffer); //get the size of the empty portion of the buffer
int  get_buffer_size(struct buffer* buffer); //get the size of the full portion of the buffer
bool  isBufferFull(struct buffer* buffer);
bool  isBufferEmpty(struct buffer* buffer);

#endif
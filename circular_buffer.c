#include "circular_buffer.h"

void initialize_buffer(struct buffer* buffer)
{
    buffer->head=0;
    buffer->tail=0;
    buffer->isfull=false;
}

void reset_buffer(struct buffer* buffer)
{
    buffer->head=0;
    buffer->tail=0;
    buffer->isfull=false;
}

void clear_buffer(struct buffer* buffer)
{
    buffer->head=0;
    buffer->tail=0;
    buffer->isfull=false;
    unsigned int i=0;
    while(i<BUFFER_MAX_SIZE) 
    {
        buffer->data[i]=0;
        i++;
    }
}

//returns 1 if full
bool  isBufferFull(struct buffer* buffer)
{
    return buffer->isfull;
}
//returns 1 if empty
bool  isBufferEmpty(struct buffer* buffer)
{
    return ((buffer->head == buffer->tail) && (buffer->isfull == false)) ? true:false;
}

int  push_character(struct buffer* buffer, unsigned char character)
{
    if(!isBufferFull(buffer))
    {
        buffer->data[buffer->head] = character;
        buffer->head = (buffer->head +1) % BUFFER_MAX_SIZE;
        if (buffer->head == buffer->tail)
        {
            buffer->isfull = true;
        }
    }
    else
    {
        printk("cyclic_buffer.c : can't push : Buffer is full \n");
        return -1;
    } 
    return 0;
}

int  pull_character(struct buffer* buffer, unsigned char* character)
{
    if(!isBufferEmpty(buffer))
    {
        *character = buffer->data[buffer->tail];
        buffer->tail++;
        buffer->isfull = false;
        if (buffer->tail == buffer->head)
        {
            buffer->tail = 0;
            buffer->head = 0;
        }
    }
    else{
        //printk("cyclic_buffer.c : can't pull : Buffer empty \n");
        return -1;
    }
    return 0;
}


int  push_string(struct buffer* buffer, unsigned char* string, unsigned int string_size)
{

    unsigned int i=0;
    if (string_size != 0)
    {
        while(i<string_size && push_character(buffer,string[i])==0)
            i++;
    }
    

    return 0;
}

//get the size of the empty portion of the buffer
int  get_buffer_room(struct buffer* buffer)
{
    if(buffer->head > buffer->tail) return (BUFFER_MAX_SIZE - buffer->tail) + buffer->tail;
    else if (buffer->head <= buffer->tail) return buffer->tail - buffer->head;
    return -1;
}

//get the size of the full portion of the buffer
int  get_buffer_size(struct buffer* buffer)
{
    return BUFFER_MAX_SIZE - get_buffer_room(buffer) ;
}


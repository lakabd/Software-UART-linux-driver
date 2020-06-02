#include<stdio.h>
//#include "buffer.h"
#include "cyclic_buffer.c"


int main(){
    struct buffer test;
    unsigned char data;
    initialize_buffer(&test);
    pull_character(&test,&data);
    printf("for value %c we have head = %d and tail = %d and flag = %d\n",data,test.head,test.tail,test.isfull);
    push_character(&test, (unsigned char) 'a');
    push_character(&test, 'b');
    push_character(&test, 'd');
    pull_character(&test,&data);
    printf("for value %c we have head = %d and tail = %d and flag = %d\n",data,test.head,test.tail,test.isfull);
    push_character(&test, 'e');
    push_character(&test, 'r');
    pull_character(&test,&data);
    printf("for value %c we have head = %d and tail = %d and flag = %d\n",data,test.head,test.tail,test.isfull);
    pull_character(&test,&data);
    printf("for value %c we have head = %d and tail = %d and flag = %d\n",data,test.head,test.tail,test.isfull);
    pull_character(&test,&data);
    printf("for value %c we have head = %d and tail = %d and flag = %d\n",data,test.head,test.tail,test.isfull);
    pull_character(&test,&data);
    printf("for value %c we have head = %d and tail = %d and flag = %d\n",data,test.head,test.tail,test.isfull);
    for(int i=0; i<257;i++){
        push_character(&test, 'j');
    }
    pull_character(&test,&data);
    pull_character(&test,&data);
    printf("for value %c we have head = %d and tail = %d and flag = %d\n",data,test.head,test.tail,test.isfull);
    push_string(&test,"heeho", sizeof("heeho"));
    //pull_character(&test,&data);
    printf("for value %c we have head = %d and tail = %d and flag = %d\n",data,test.head,test.tail,test.isfull);
    printf("buffer room is %d \n", get_buffer_room(&test));
    printf("buffer size is %d \n", get_buffer_size(&test));

    flush_buffer(&test);
    printf("for value %c we have head = %d and tail = %d and flag = %d\n",data,test.head,test.tail,test.isfull);

    push_string(&test,"Abder", sizeof("Abder"));
    printf("size of Abder %ld \n", sizeof("Abder"));
    pull_character(&test,&data);
    printf("for value %c we have head = %d and tail = %d and flag = %d\n",data,test.head,test.tail,test.isfull);
    pull_character(&test,&data);
    printf("for value %c we have head = %d and tail = %d and flag = %d\n",data,test.head,test.tail,test.isfull);
    pull_character(&test,&data);
    printf("for value %c we have head = %d and tail = %d and flag = %d\n",data,test.head,test.tail,test.isfull);
    pull_character(&test,&data);
    printf("for value %c we have head = %d and tail = %d and flag = %d\n",data,test.head,test.tail,test.isfull);
    if(pull_character(&test,&data)==0)
        printf("for value %c we have head = %d and tail = %d and flag = %d\n",data,test.head,test.tail,test.isfull);

    printf("buffer room is %d \n", get_buffer_room(&test));
    printf("buffer size is %d \n", get_buffer_size(&test));
    return 0;
}
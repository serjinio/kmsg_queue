#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "comdefs.h"

#define PROC_FILE_PATH_LENGTH 255


FILE* open_msg_queue(char* mode) {
  char fpath[PROC_FILE_PATH_LENGTH];
  FILE *msg_stream;

  sprintf(fpath, "/proc/%s", PROCFS_NAME);
  /* printf("Will open message queue at path: %s...\n", fpath); */
  msg_stream = fopen(fpath, mode);
  if(msg_stream == NULL)
    {
      printf("ERROR: failed to open: %s\n", fpath);
      exit(1);
    }

  return msg_stream;
}

void close_msg_queue(FILE* msg_stream) {
  fclose(msg_stream);
}


void write_msg(FILE* msg_stream, char* msg) {
  /* printf("will write a message to the queue: %s\n", msg); */
  fprintf(msg_stream, "%s\n", msg);
}

char* read_msg(FILE* msg_stream, size_t buff_sz) {
  char* buffer;
  int retcode;

  buffer = malloc(sizeof(char) * buff_sz);
  if (buffer == NULL) {
    printf("Failed to allocate read buffer!\n");
    exit(1);
  }

  retcode = fread(buffer, sizeof(char) * buff_sz, 1, msg_stream);
  /* printf("Read buffer contents: %s\n", buffer); */

  if (ferror(msg_stream)){
    printf("Failed to read from message queue!\n");
    free(buffer);
    exit(1);
  }

  return buffer;
}

void test1() {
  FILE *msg_stream;
  char* msg;

  printf("Test1: will write & read one message...\n");

  msg_stream = open_msg_queue("w");
  write_msg(msg_stream, "Hello world!");
  close_msg_queue(msg_stream);


  msg_stream = open_msg_queue("r");
  msg = read_msg(msg_stream, MSG_MAX_SIZE);
  msg[strcspn(msg, "\n")] = 0;
  if (strcmp(msg, "Hello world!") != 0) {
    printf("ERROR: Expected: %s; obtained: %s", "Hello world!", msg);
    exit(1);
  }
  free(msg);
  close_msg_queue(msg_stream);

  printf("Test1: finished\n");
}

void test2() {
  FILE *msg_stream;
  char* msg;

  printf("Test2: read into small buffer...\n");

  msg_stream = open_msg_queue("w");
  write_msg(msg_stream, "Hello world!");
  close_msg_queue(msg_stream);

  msg_stream = open_msg_queue("r");
  // read into a small buffer
  msg = read_msg(msg_stream, 3);
  msg[strcspn(msg, "\n")] = 0;
  if (strcmp(msg, "Hel") != 0) {
    printf("ERROR: Expected: %s; obtained: %s", "Hel", msg);
    exit(1);
  }
  free(msg);
  close_msg_queue(msg_stream);

  printf("Test2: finished\n");
}

void test3() {
  FILE *msg_stream;
  char *read_buf, *write_buf;
  int i;

  printf("Test3: write 100,000 messages...\n");

  write_buf = malloc(sizeof(char) * 255);

  for (i = 0; i < 100000; i++) {
    sprintf(write_buf, "Hello world #%d", i);
    msg_stream = open_msg_queue("w");
    write_msg(msg_stream, write_buf);
    close_msg_queue(msg_stream);
  }

  // read into a small buffer
  for (i = 0; i < 100000; i++) {
    msg_stream = open_msg_queue("r");
    read_buf = read_msg(msg_stream, MSG_MAX_SIZE);
    read_buf[strcspn(read_buf, "\n")] = 0;
    sprintf(write_buf, "Hello world #%d", i);
    /* printf("Obtained a message from the queue: %s\n", read_buf); */
    /* printf("write_buf: %s\n", write_buf); */
    if (strcmp(read_buf, write_buf) != 0) {
      printf("ERROR: Expected: %s; obtained: %s", write_buf, read_buf);
      exit(1);
    }
    free(read_buf);
    close_msg_queue(msg_stream);
  }

  free(write_buf);

  printf("Test3: finished\n");
}

int main()
{
  test1();
  test2();
  test3();

  return 0;
}

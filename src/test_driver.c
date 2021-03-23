#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

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

  buffer = calloc(buff_sz, sizeof(char));
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

void test_read_write_once() {
  FILE *msg_stream;
  char* msg;

  printf("Test write & read one message...\n");

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

  printf("Test write & read one message - finished\n");
}

void test_read_into_small_buffer() {
  FILE *msg_stream;
  char* msg;

  printf("Test read into small buffer...\n");

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

  printf("Test read into small buffer - finished\n");
}

void test_large_write() {
  FILE *msg_stream;
  char *read_buf, *write_buf;
  int i;

  printf("Test write 100,000 messages...\n");

  write_buf = calloc(255, sizeof(char));

  for (i = 0; i < 100000; i++) {
    sprintf(write_buf, "Hello world #%d", i);
    msg_stream = open_msg_queue("w");
    write_msg(msg_stream, write_buf);
    close_msg_queue(msg_stream);
  }

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

  printf("Test write 100,000 messages - finished\n");
}

void test_read_from_empty_queue() {
  FILE *msg_stream;
  char *read_buf;

  printf("Test read from an empty queue...\n");

  msg_stream = open_msg_queue("r");
  read_buf = read_msg(msg_stream, MSG_MAX_SIZE);
  if (strlen(read_buf) > 0) {
    printf("Queue must be empty!");
    exit(1);
  }
  free(read_buf);

  printf("Test read from an empty queue - finished\n");
}


struct writer_info {
  char* name;
  size_t batch_size;
};


struct reader_info {
  char *name;
  size_t batch_size;
};


void* put_message_batch(void *ptr) {
  FILE *msg_stream;
  char *read_buf, *write_buf;
  int i;
  struct writer_info *info;

  info = (struct writer_info*) ptr;

  printf("%s staring to write batch of size %ld...\n", info->name, info->batch_size);

  write_buf = calloc(255, sizeof(char));

  for (i = 0; i < info->batch_size; i++) {
    sprintf(write_buf, "%s: Hello world #%d", info->name, i);
    msg_stream = open_msg_queue("w");
    write_msg(msg_stream, write_buf);
    close_msg_queue(msg_stream);
  }

  printf("%s finished writing batch of size %ld\n", info->name, info->batch_size);
}

void* get_message_batch(void *ptr) {

}

void test_parallel_insert() {
  int threads_num, batch_size;
  pthread_t threads[10];
  struct writer_info writer_infos[10], *info_ptr;
  int i;

  FILE *msg_stream;
  char *read_buf;

  printf("Test parallel insert...\n");

  threads_num = 10;
  batch_size = 1000;

  for (i = 0; i < threads_num; i++) {
    info_ptr = &writer_infos[i];
    info_ptr->name = calloc(255, sizeof(char));
    sprintf(info_ptr->name, "writer #%d", i);
    info_ptr->batch_size = batch_size;
    pthread_create(&threads[i], NULL, put_message_batch, (void*) info_ptr);
  }

  sleep(2);

  for (i = 0; i < threads_num * (batch_size); i++) {
    msg_stream = open_msg_queue("r");
    read_buf = read_msg(msg_stream, MSG_MAX_SIZE);
    read_buf[strcspn(read_buf, "\n")] = 0;
    /* printf("Obtained a message from the queue: %s\n", read_buf); */
    if (strlen(read_buf) == 0) {
      printf("All messages MUST be non-empty. Queue seems to be corrupted!");
      exit(1);
    }
    free(read_buf);
    close_msg_queue(msg_stream);
  }

  printf("Test parallel insert - finished\n");
}

int main()
{
  test_read_from_empty_queue();
  test_read_write_once();
  test_read_into_small_buffer();
  test_large_write();
  test_parallel_insert();

  return 0;
}

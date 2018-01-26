//#define _XOPEN_SOURCE 500
//#include <ftw.h>
#include <sys/select.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
//#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <ftw.h>
#include "networking.h"

#define MEM_ERR 42
#define BUFFER_LENGTH 256

void process(char *s);
void subserver(int from_client);

int client_response(char * message, int client_socket){
    char buffer[BUFFER_SIZE];
    while(strcmp(buffer,message)) {
        read(client_socket, buffer, sizeof(buffer));

        //if it gets error message instead of confirmation
        if(strstr(buffer,ERROR_RESPONSE)) {
            write(client_socket, ERROR_WAIT, sizeof(ERROR_WAIT));
            read(client_socket, buffer, sizeof(buffer)); //reading follow up error message
            printf("%s",buffer);
            return -1;
        }
    }
    return 0;
}

int null_bytes(char* s){
    char * nul_pos = strchr(s,'\0');
    //checks if NULL is actually in the string
    if(nul_pos != NULL)
        return sizeof(char)* (int)(nul_pos - s);
    else
        return sizeof(char)*PACKET_SIZE;
}

int repo_checker_s(char * name){
  FILE *fs = fopen(name, "r");
  if(fs == NULL)
    {
      printf("ERROR: File %s not found.\n", name);
      return 0;
    }
  return 1;
  //checks if repo exists on the server
  //returns 0 if doesnt exist and 1 if it exists

}

char ** chub_parse(char * line, char * arg){

  char **args = (char**)calloc(64, sizeof(char *));
  int i = 0;
  while(line){
    args[i++] = strsep(&line, arg);
  }
  args[i] = NULL;

  return args;
}

char * file_send_c(char *filename, int sockfd){
  char buffer[BUFFER_SIZE];
  FILE * name = fopen(filename, "w");
  unsigned long fsize;
  if(repo_checker_c){
    printf("found file %s\n", filename);
    fseek(name, 0, SEEK_END);
    fsize = ftell(name);
    rewind(name);

    printf("file contains %ld bytes\n", fsize);
    printf("sending file\n");

    while (1){
      int bytes_read = fread(buffer, sizeof(char),BUFFER_LENGTH, name);
      if (bytes_read == 0) // done reading file
	break;
      if (bytes_read < 0)
	{
      printf("problem reading from file\n");
	}
      //write will return how many bytes were written.
      // p keeps track of where in the buffer we are,
      //bytes_read to keep track of how many bytes are left to write.
      void *p = buffer;
      while (bytes_read > 0)
	{
        int bytes_written = write(sockfd, buffer, bytes_read);
        if (bytes_written <= 0)
        {
            printf("ERROR writing to socket\n");
        }
        bytes_read -= bytes_written;
        p += bytes_written;
	}
    }
    printf("file sent\n");
    return "receiving_file";
 }
 return 0;

}

int parse_s(char buffer[], int client_socket){
  char file[BUFFER_SIZE];
  char fcontent[PACKET_SIZE];
  int fd;
  // char buffer[BUFFER_LENGTH +1];
  // buffer[BUFFER_LENGTH]=0;
  char **parsed;
  // read(client_socket, buffer, BUFFER_LENGTH);
  parsed = chub_parse(buffer, " ");
  if(parsed[0]){

    ///for creating repo on server
    /*   if(!strncmp(parsed[0], "FILE", 5)){
      //printf("HI YO\n");
      write(client_socket, "1", sizeof("1")); //responds to client

      read(client_socket, file, sizeof(file)); //receives file name
       printf("[Server]: received [%s]\n", file);
       }*/
    if(!strncmp("create", parsed[0], 5)){
      if(parsed[1]){
        int exists=repo_checker_s(parsed[1]);
	if(!exists){
          // printf("stuff\n");
          mkdir(parsed[1], 0666);
          printf("Created repository named: %s \n", parsed[1]);
          return 1;
	}
	printf("Repository with that name already exists. Could not create. \n");
      }
    }
    ///for cloning repo
    if(!strncmp("clone", parsed[0], 5)){
      if(parsed[0]){
        write(client_socket, "1", sizeof("1")); //telling client that it understands it wants to pull
        read(client_socket, file, sizeof(file)); //receiving file name to copy
        printf("Recieved file named %s\n", file);
        int exists=repo_checker_s(file);
        if(exists){
          write(client_socket, "We found the file! Will attempt to get it to you\n", sizeof("We found the file! Will attempt to get it to you\n"));
	  //accessing file contents
	  memset(fcontent,0,sizeof(fcontent));
	  read(fd, fcontent, sizeof(fcontent));
	  //sending file contents up to NULL
	  client_response("3", client_socket);
	  write(client_socket, fcontent, null_bytes(fcontent));
	  close(fd);
	  printf("[Server]: pulled from '%s'\n", file);
	  //file_send_c(parsed[1], client_socket);
	  printf("Sent file named: %s \n", parsed[1]);
		// file_receive_c(parsed[1], client_socket);

          return 1;
        }else{
          write(client_socket, "ERROR: File doesn't exist. Unable to clone.\n", sizeof("ERROR: File doesn't exist. Unable to clone.\n"));
        }

      }
    }
    printf("Something went wrong... please try again.\n");
    return 0;
  }
}


void subserver(int client_socket) {
  char buffer[BUFFER_SIZE];

  //for testing client select statement
  strncpy(buffer, "hello client", sizeof(buffer));
  write(client_socket, buffer, sizeof(buffer));

  while (read(client_socket, buffer, sizeof(buffer))) {

    parse_s(buffer, client_socket);
    printf("[subserver %d] received: [%s]\n", getpid(), buffer);
    process(buffer);
    write(client_socket, buffer, sizeof(buffer));
  }//end read loop
  close(client_socket);
  exit(0);
}

void process(char * s) {
  while (*s) {
    if (*s >= 'a' && *s <= 'z')
      *s = ((*s - 'a') + 13) % 26 + 'a';
    else  if (*s >= 'A' && *s <= 'Z')
      *s = ((*s - 'a') + 13) % 26 + 'a';
    s++;
  }
}

int main() {

  int listen_socket;
  int client_socket;
  int f;
  int subserver_count = 0;
  char buffer[BUFFER_SIZE];

  //set of file descriptors to read from
  fd_set read_fds;

  listen_socket = server_setup();

  while (1) {

    //select() modifies read_fds
    //we must reset it at each iteration
    FD_ZERO(&read_fds); //0 out fd set
    FD_SET(STDIN_FILENO, &read_fds); //add stdin to fd set
    FD_SET(listen_socket, &read_fds); //add socket to fd set

    //select will block until either fd is ready
    select(listen_socket + 1, &read_fds, NULL, NULL, NULL);

    //if listen_socket triggered select
    if (FD_ISSET(listen_socket, &read_fds)) {
     client_socket = server_connect(listen_socket);

     f = fork();
     if (f == 0)
       subserver(client_socket);
     else {
       subserver_count++;
       close(client_socket);
     }
    }//end listen_socket select

    //if stdin triggered select
    if (FD_ISSET(STDIN_FILENO, &read_fds)) {
      //if you don't read from stdin, it will continue to trigger select()
      fgets(buffer, sizeof(buffer), stdin);
      printf("[server] subserver count: %d\n", subserver_count);
    }//end stdin select
  }
}

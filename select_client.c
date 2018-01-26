#include "networking.h"
#include <time.h>

//#define MEM_ERR 42
//#define BUFFER_LENGTH 256

int file_receive_c(char *FILENAME, int sockfd);
char ** chub_parse(char * line, char * arg);
int repo_checker_c(char *name);
int parse_c(char buffer[], int server_socket);

int null_bytes(char* s){
    char * nul_pos = strchr(s,'\0');
    //checks if NULL is actually in the string
    if(nul_pos != NULL)
        return sizeof(char)* (int)(nul_pos - s);
    else
        return sizeof(char)*PACKET_SIZE;
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

int repo_checker_c(char *name){
  FILE *fs = fopen(name, "r");
  if(fs == NULL)
    {
      printf("ERROR: File %s not found.\n", name);
      exit(1);
    }
}
int parse_c(char buffer[], int server_socket){
  // char buffer[BUFFER_LENGTH +1];
  // buffer[BUFFER_LENGTH]=0;
  char **parsed;
  char * name = "sample.txt";
  // read(client_socket, buffer, BUFFER_LENGTH);
  parsed = chub_parse(buffer, " ");
  if(parsed[0]){

    ///for creating repo on server
    if(!strncmp("clone", parsed[0], 5)){
      if(parsed[1]){
        int exists=repo_checker_c(parsed[1]);
	if(!exists){
          // printf("stuff\n");
	  file_receive_c(name, server_socket);
          //mkdir(parsed[1], 0666);
          printf("Received file called %s \n", parsed[1]);
          return 1;
	}
	printf("Repository with that name already exists. Could not create. \n");
      }
    }
    printf("Something went wrong... please try again.\n");
    return 0;
  }
}

int file_receive_c(char * FILENAME, int sockfd){
  char buffer[BUFFER_SIZE];
  int bytesReceived = 0;
  FILE *fp;
  printf("stuff 1\n");
  fp = fopen(FILENAME, "r");
  printf("stuff 2\n");
  if(NULL == fp)
    {
      printf("Error opening file");
      return 1;
    }
  printf("stuff 3\n");
  /* Receive data in chunks of 256 bytes */
  while((bytesReceived = read(sockfd, buffer, 256)) > 0)
    {
      printf("stuff 4\n");
      printf("Bytes received %d\n",bytesReceived);
      // recvBuff[n] = 0;
      fwrite(buffer, 1,bytesReceived,fp);
      // printf("%s \n", recvBuff);
    }
  printf("stuff 5\n");
  if(bytesReceived < 0)
    {
      printf("\n Read Error \n");
    }

  printf("stuff 6\n");
  return 0;
}




//waits for message from server
int serv_response(char * message, int server_socket){
    char buffer[BUFFER_SIZE];
    while(strcmp(buffer,message)) {
        read(server_socket, buffer, sizeof(buffer));

        //if it gets error message instead of confirmation
        if(strstr(buffer,ERROR_RESPONSE)) {
            write(server_socket, ERROR_WAIT, sizeof(ERROR_WAIT));
            read(server_socket, buffer, sizeof(buffer)); //reading follow up error message
            printf("%s",buffer);
            return -1;
        }
    }
    return 0;
}



int main(int argc, char **argv) {

  int server_socket;
  char buffer[BUFFER_SIZE];
  char file[BUFFER_SIZE]; //packet size is max size of file we r sending
  char fcontent[PACKET_SIZE];
  char res[BUFFER_SIZE];


  printf("Type the IP address of the server you want to connect to.\n");
   //char * buffer0 = calloc(1024,sizeof(char));
   memset(buffer, 0, sizeof(buffer));
   fgets(buffer, 1024, stdin);
   char * newline = strchr(buffer,'\n');
   *newline = 0;


   fd_set read_fds;

   if (argc == 2)
     server_socket = client_setup( argv[1]);
   else
     server_socket = client_setup( buffer);


    fflush(stdout);

    //select() modifies read_fds
    //we must reset it at each iteration
    FD_ZERO(&read_fds);
    FD_SET(STDIN_FILENO, &read_fds); //add stdin to fd set
    FD_SET(server_socket, &read_fds); //add socket to fd set

    //select will block until either fd is ready
    select(server_socket + 1, &read_fds, NULL, NULL, NULL);

    if (FD_ISSET(STDIN_FILENO, &read_fds)) {
      fgets(buffer, sizeof(buffer), stdin);
      *strchr(buffer, '\n') = 0;
      write(server_socket, buffer, sizeof(buffer));
      read(server_socket, buffer, sizeof(buffer));
      //parse_c(buffer, server_socket);
      printf("received: [%s]\n", buffer);
      //  if (strcmp(buffer,"[clone]")):
    }//end stdin select


    if (FD_ISSET(server_socket, &read_fds)) {
      read(server_socket, buffer, sizeof(buffer));
      printf("[SERVER BROADCAST] [%s]\n", buffer);
      printf("enter data: ");
      parse_c(buffer, server_socket);
      //the above printf does not have \n
      //flush the buffer to immediately print
      fflush(stdout);
    }



  }//end loop

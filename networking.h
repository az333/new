#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <signal.h>

#define MEM_ERR 42
#define NO_ARGS 7
#define USER_EXIT 0

#define ERROR_RESPONSE "-1"
#define ERROR_WAIT "-2"
#define PACKET_SIZE 43243

int null_bytes(char *s);

void chub_prompt();

char * chub_read();

char ** chub_parse(char * line, char * arg);

int chub_functions(char ** args);

void chub_initiate();

int repo_checker_s(char * name);

int repo_create_s(char * name);

int repo_delete_s(char * name);

int repo_create_c(char * name);

int repo_checker_c(char * name);

int repo_delete_c(char *name);



#ifndef NETWORKING_H
#define NETWORKING_H

#define BUFFER_SIZE 256
#define PORT "9001"
#define TEST_IP "0.0.0"

void error_check(int i, char *s);
int server_setup();
int server_connect(int sd);
int client_setup(char * server);


#endif

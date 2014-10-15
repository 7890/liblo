/*
 *  Copyright (C) 2012 Steve Harris, Stephen Sinclair
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation; either version 2.1 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  $Id$
 */

//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "lo/lo.h"

int done = 0;
int count = 0;

void print_lo_info();

void error(int num, const char *m, const char *path);

int default_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data);

void ctrlc(int sig)
{
    done = 1;
}

int main(int argc, char *argv[])
{
    if(argc<2)
    {
       printf("need args: 1) allowed incoming max msg size in bytes\n");
       return 1;
    }

    int desired_max_tcp_size=atoi(argv[1]);

    const char *port = "7890";
    int do_send = 0;

    /* start a new server on port 7770 */
    lo_server_thread st = lo_server_thread_new_with_proto(port, LO_TCP, error);
    if (!st) {
        printf("Could not create server thread.\n");
        exit(1);
    }

    print_lo_info();

    lo_server s = lo_server_thread_get_server(st);

    int ret_set_size=lo_server_max_msg_size(s, desired_max_tcp_size);
    printf("set tcp max size return: %d\n",ret_set_size);

    /* add method that will match any path and args */
    lo_server_thread_add_method(st, NULL, NULL, default_handler, s);

    lo_server_thread_start(st);

    signal(SIGINT,ctrlc);

    printf("Listening on TCP port %s\n", port);

    while (!done) {
#ifdef WIN32
        Sleep(1);
#else
        sleep(1);
#endif
    }

    lo_server_thread_free(st);

    return 0;
}

void error(int num, const char *msg, const char *path)
{
    printf("liblo server error %d in path %s: %s\n", num, path, msg);
    fflush(stdout);
}

/* catch any incoming messages, display them */
int default_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data)
{
    int i;
    lo_message m = (lo_message)data;
    lo_address a = lo_message_get_source(m);
    lo_server s = (lo_server)user_data;
    const char *host = lo_address_get_hostname(a);
    const char *port = lo_address_get_port(a);

    count ++;

    fprintf(stderr,"path: <%s>\n", path);
    for (i = 0; i < argc; i++) {
        fprintf(stderr,"arg %d '%c' ", i, types[i]);
        lo_arg_pp((lo_type)types[i], argv[i]);
	fflush(stdout);
        fprintf(stderr,"\n");
    }

    if (!a) {
        fprintf(stderr,"Couldn't get message source, quitting.\n");
        done = 1;
        return 0;
    }

    return 0;
}

void print_lo_info()
{
    int major, minor, lt_maj, lt_min, lt_bug;
    char extra[256];
    char string[256];

    lo_version(string, 256,
               &major, &minor, extra, 256,
               &lt_maj, &lt_min, &lt_bug);

    printf("liblo version string `%s'\n", string);
    printf("liblo version: %d.%d%s\n", major, minor, extra);
    printf("liblo libtool version: %d.%d.%d\n", lt_maj, lt_min, lt_bug);

    printf("liblo MAX_MSG_SIZE: %d\n", LO_MAX_MSG_SIZE);
    printf("liblo MAX_UDP_MSG_SIZE: %d\n", LO_MAX_UDP_MSG_SIZE);
    printf("liblo LO_DEFAULT_MAX_MSG_SIZE: %d\n", LO_DEFAULT_MAX_MSG_SIZE);
    printf("\n");

    //return 0;
}

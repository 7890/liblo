//OSC daemon that listens for any message (default) on UDP port 7770 (default).
//program will quit as soon as message is received.
//arguments of a message aren't considered (none or any).
//this is only good enough for quick hacks.

//example use: listen for /shutdown on port 7890
//   osc_continue "/shutdown" 7890 && sudo shutdown -h now

//example use: listen to any message on standard port 7770
//   osc_continue && myscript.sh

//tb/1707
//gcc -o osc_continue osc_continue.c -llo

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lo/lo.h"
void error(int num, const char *m, const char *path);
int generic_handler(const char *path, const char *types, lo_arg ** argv,
                     int argc, void *data, void *user_data);
int done;
char *continue_string;
char *port;

int main(int argc, char *argv[])
{
	if(argc == 1)
	{
		continue_string=NULL; //all
		port="7770";
	}
	else if(argc == 2)
	{
		continue_string=argv[1];
		port="7770";
	}
	else if(argc == 3)
	{
		continue_string=argv[1];
		port=argv[2];
	}
	else
	{
		fprintf(stderr,"wrong args\n");
		return 1;
	}
	fprintf(stderr,"continue_string: %s\n", continue_string);
	fprintf(stderr,"port: %s\n", port);

	lo_server_thread st = lo_server_thread_new(port, error);
	lo_server_thread_add_method(st, continue_string, NULL, generic_handler, NULL);
	lo_server_thread_start(st);
	done=0;
	while (done==0) {
#ifdef WIN32
		Sleep(1);
#else
		usleep(1000);
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

int generic_handler(const char *path, const char *types, lo_arg ** argv,
                     int argc, void *data, void *user_data)
{
	lo_address src = lo_message_get_source(data);
	printf("continue signal %s received from host %s\n", continue_string, lo_address_get_url(src));
	fflush(stdout);
	done=1;
	return 0;
}

//EOF

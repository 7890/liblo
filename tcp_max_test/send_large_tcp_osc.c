#include <stdio.h>
#include <stdlib.h>
#include <lo/lo.h>

//gcc -o send_large_tcp_osc send_large_tcp_osc.c `pkg-config --cflags --libs liblo`

//while true; do nc -l 7890 > /dev/null; done

//tb/14

//TCP server
lo_server_thread lo_st_tcp;
lo_address loa_tcp;

void print_lo_info();

int
main (int argc, char *argv[])
{
	if(argc<3)
	{
		printf("need args: 1) msg size in bytes 2) remote host\n");
		return 1;
	}

	print_lo_info();

	//first arg sets blob size
	int blob_size=atoi(argv[1]);
	char *host=argv[2];
	char *lport="9900";
	char *rport="7890";

	printf ("will send from :%s to %s:%s\n", lport,host,rport);

	lo_st_tcp = lo_server_thread_new_with_proto(lport, LO_TCP, NULL);
	lo_server_thread_start(lo_st_tcp);
	loa_tcp = lo_address_new_with_proto(LO_TCP, host, rport);

//	lo_address_set_stream_slip(loa_tcp, 1);

	printf("trying to establish connection...\n");

	//////for receiver only
	//lo_server lo_s_tcp=lo_server_thread_get_server(lo_st_tcp);
	//int ret_set_size=lo_server_max_msg_size(lo_s_tcp, blob_size*2);

	void* tmp;
	tmp=malloc(blob_size);

	int counter=0;

	while(1==1)
	{
		lo_message mm=lo_message_new();
		lo_blob b=lo_blob_new(blob_size,tmp);
		lo_message_add_int32(mm,counter);
		lo_message_add_blob(mm,b);
		//lo_message_add_blob(mm,b);

		size_t msg_len=lo_message_length(mm,"/data");
		fprintf(stderr,"msg #%d (%zu bytes): ",counter,msg_len);

		int ret=-1;
		while( (ret=lo_send_message(loa_tcp,"/data",mm)) && ret < 0 )
		{
			fprintf(stderr,"msg #%d: lo return: %d %d %s\n",counter,ret,lo_address_errno(loa_tcp),lo_address_errstr(loa_tcp));
			usleep(500000);
		}

		fprintf(stderr,"done. lo return: %d %d lo_errstr: %s\n",ret,lo_address_errno(loa_tcp),lo_address_errstr(loa_tcp));

		//check if reported sent bytes equal to msg size
		if(ret!=msg_len)
		{
			fprintf(stderr,"warn: effective length - confirmed sent: %zu\n",msg_len-ret);
		}

		usleep(50000);
		counter++;

		lo_message_free(mm);
		free(b);
	}

	exit(0);
}

//from src/testlo.c test_version
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


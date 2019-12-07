#include <stdio.h>  //fprintf
#include <stdlib.h> //malloc
#include <stdint.h>
#include <lo/lo.h>  //lo_*

//gcc -o send_large_tcp_osc send_large_tcp_osc.c `pkg-config --cflags --libs liblo`

//while true; do nc -l 7890 > /dev/null; done

//tb/14 /19

static void usage()
{
	fprintf(stderr, "send_large_tcp_osc help\n");
	fprintf(stderr, "need args: 1) msg size in bytes 2) msg count or 0 3) local port 4) remote host 5) remote port\n");
}

int main (int argc, char *argv[])
{
	//TCP server
	lo_server_thread lo_st_tcp;
	lo_address loa_tcp;

	const char *lport;
	const char *host;
	const char *rport;
	uint64_t limit_blob_size=1000000 * 100;

	if(argc<6)
	{
		usage();
		return 1;
	}

	//first arg sets blob size
	uint64_t blob_size=       atoll(argv[1]);
	uint64_t limit_send_count=atoll(argv[2]);
	lport=argv[3];
	host= argv[4];
	rport=argv[5];

	if(blob_size<1 || (limit_blob_size>0 && blob_size>limit_blob_size))
	{
		usage();
		fprintf(stderr, "error: too small or large blob size: %llu\n", (long long unsigned int)blob_size);
		return 1;
	}

	printf ("will send blobs of size %llu from :%s to %s:%s\n", (long long unsigned int)blob_size, lport, host, rport);

	void* tmp; //for blob
	tmp=calloc(blob_size,1);

	loa_tcp =   lo_address_new_with_proto(LO_TCP, host, rport);
	lo_st_tcp = lo_server_thread_new_with_proto(lport, LO_TCP, NULL);

	lo_server_thread_start(lo_st_tcp);
//	lo_address_set_stream_slip(loa_tcp, 1);

	printf("trying to establish connection...\n");

	uint64_t counter=0;
	lo_message mm=NULL;
	lo_blob b=NULL;
	size_t msg_len;

	while(1==1)
	{
		mm=lo_message_new();
		b=lo_blob_new(blob_size, tmp);
		if(!b)
		{
			fprintf(stderr, "error: cannot create blob.\n");
			goto label_error;
		}

		lo_message_add_int64(mm, counter);
		lo_message_add_blob(mm, b);

		msg_len=lo_message_length(mm, "/data");
		fprintf(stderr, "\rmsg #%llu (%zu bytes): ", (long long unsigned int)counter+1, msg_len);

		int ret=-1;
		while( (ret=lo_send_message(loa_tcp, "/data", mm)) && ret < 0 )
		{
			fprintf(stderr, "msg #%llu: lo return: %d %d %s\n", (long long unsigned int)counter+1, ret, lo_address_errno(loa_tcp), lo_address_errstr(loa_tcp));
			usleep(500000);
		}

		fprintf(stderr, "done. lo return: %d %d %s", ret, lo_address_errno(loa_tcp), lo_address_errstr(loa_tcp));

		//check if reported sent bytes count is equal to msg size
		if(ret!=msg_len)
		{
			fprintf(stderr, "\nwarn: effective length - confirmed sent: %zu\n", msg_len-ret);
		}

		lo_message_free(mm);
		free(b);
		mm=NULL;
		b=NULL;

//		usleep(50000);
		counter++;

		if(limit_send_count>0 && counter>=limit_send_count)
		{
			fprintf(stderr, "\nsent %llu messages, done. %f MB\n"
				, (long long unsigned int)limit_send_count
				, (double) ( (limit_send_count*blob_size)/1000000 ) 
			);
			break;
		}
	}

	//clean up
	free(tmp);
	lo_address_free(loa_tcp);
	lo_server_thread_stop(lo_st_tcp);
	lo_server_thread_free(lo_st_tcp);
	return 0;

label_error:
	if(mm){lo_message_free(mm);}
	if(b) {lo_message_free(b);}

	free(tmp);
	lo_address_free(loa_tcp);
	lo_server_thread_stop(lo_st_tcp);
	lo_server_thread_free(lo_st_tcp);
	return 1;
} //end main()

//lo_server lo_s_tcp=lo_server_thread_get_server(lo_st_tcp);
//int ret_set_size=lo_server_max_msg_size(lo_s_tcp, blob_size*2);

//EOF

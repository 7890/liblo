#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lo/lo.h"

//tb/151101

//gcc -o nest nest.c -llo && ./nest > /tmp/a && hexdump -c /tmp/a

//trying to wrap lo_message_add
lo_blob sub(const char *types, ... )
{
	lo_message msg=lo_message_new();
	const char* path="/";

	//from src/message.c, int lo_message_add(lo_message msg, const char *types, ...)
	va_list ap;
	const char *file = "";
	const int line = 0;
	va_start(ap, types);

	//return
	lo_message_add_varargs_internal(msg, types, ap, file, line);
	//va_end (ap);
	//some problem here:liblo error: lo_send, lo_message_add, or lo_message_add_varargs called with mismatching types and data at:0, exiting.

	void * msgbytes=calloc(lo_message_length(msg,path),sizeof(char));
	size_t size_ret;
	lo_message_serialise (msg, path, msgbytes, &size_ret);

	lo_message_free(msg);
	//free(msgbytes); ////!?

	fprintf(stderr,"serialized %lu bytes\n",size_ret);
	lo_blob b=lo_blob_new(size_ret,msgbytes);
	fprintf(stderr,"blob size %d bytes\n",lo_blobsize(b));

	//blob to be freed by caller
	return b;
}

int main(int argc, char *argv[])
{
	const float float_array[3] = {0.1,0.2,0.3};
	lo_blob bfloat = lo_blob_new(sizeof(float_array), float_array);

	lo_blob b1=sub("s","gugus aaaa");
	lo_blob b2=
		sub("bbbiif"
			,b1 //containing previously created msg blob
			,bfloat //containing (non-msg) float array blob
			,sub("sb","inner" //another "inline" msg blob"
				,sub("fff" //recurse...
					,0.1
					,0.2
					,0.3
				)
			)
		,1
		,2
		,3.4);

	lo_message msg=lo_message_new();
	lo_message_add(msg,"ifsb",128,0.2,"foo",b2);

	const char * path="/test";

	int msg_length=lo_message_length(msg,path);
	void * msg_bytes=calloc(msg_length,sizeof(char));
	size_t size_ret;
	lo_message_serialise (msg, path, msg_bytes, &size_ret);
	fprintf(stderr,"serialized %lu bytes\n",size_ret);

	unsigned char *q = msg_bytes;
	int k;
	for (k = 0; k < msg_length; k++)
	{
		printf("%c",q[k]);
	}
	fflush(stdout);
	fprintf(stderr,"\n");

	lo_blob_free(bfloat);
	lo_blob_free(b1);
	lo_blob_free(b2);
	lo_message_free(msg);

	return 0;
}

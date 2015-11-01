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

	uint8_t midi_data[4] = { 0xff, 0xf7, 0xAA, 0x00 };

	//lo_timetag timetag;
	//lo_timetag_now(&timetag);

	lo_timetag timetag = { 0x1, 0x80000000 };
	//lo_timetag timetag = { 0x1, 0x00000000 };

	lo_blob b1=sub("sif","a string grouped with an int and a float",42,0.123);
	lo_blob b2=
	sub("bbbifb"
		,b1 //containing previously created msg blob
		,bfloat //containing (non-msg) float array blob
		,sub("sb"
			,"named float list"
			,sub("fff"
				,0.1
				,0.2
				,0.3
			)
		)
		,1
		,2.3
		,sub("hTdScmtbb" //param without arg in-between
			,0x0123456789abcdefULL //int64
			//(true)
			,0.9999 //double
			,"sym" //symbol
			,'X' //char
			,midi_data
			,timetag
			,sub("TFNIb"
				//(true)
				//(false)
				//(nil)
				//(infinitum)
				,b1
			)
			,b1
		)
	);

	lo_message msg=lo_message_new();
	lo_message_add(msg,"ifsb",128,0.2,"foo",b2);

	const char * path="/test";

	//prepare to write serialised message to stdout for later use
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

/*
expected output for this example:

raw, 392 bytes
(here base64 encoded)

L3Rlc3QAAAAsaWZzYgAAAAAAAIA+TMzNZm9vAAAAAXQvAAAALGJiYmlmYgAAAABALwAAACxzaWYA
AAAAYSBzdHJpbmcgZ3JvdXBlZCB3aXRoIGFuIGludCBhbmQgYSBmbG9hdAAAAAAAAAAqPfvnbQAA
AAzNzMw9zcxMPpqZmT4AAAA4LwAAACxzYgBuYW1lZCBmbG9hdCBsaXN0AAAAAAAAABgvAAAALGZm
ZgAAAAA9zMzNPkzMzT6ZmZoAAAABQBMzMwAAAMwvAAAALGhUZFNjbXRiYgAAASNFZ4mrze8/7/8u
SOinHnN5bQAAAABY//eqAAAAAAGAAAAAAAAAUC8AAAAsVEZOSWIAAAAAAEAvAAAALHNpZgAAAABh
IHN0cmluZyBncm91cGVkIHdpdGggYW4gaW50IGFuZCBhIGZsb2F0AAAAAAAAACo9++dtAAAAQC8A
AAAsc2lmAAAAAGEgc3RyaW5nIGdyb3VwZWQgd2l0aCBhbiBpbnQgYW5kIGEgZmxvYXQAAAAAAAAA
Kj37520=

expanded (see expand.c)

/test ifsb
i 128
f 0.200000
s "foo"
b 376 / bbbifb
   b 68 / sif
      s "a string grouped with an int and a float"
      i 42
      f 0.123000
   b 16 (blob with unknown encoding)
   b 60 / sb
      s "named float list"
      b 28 / fff
         f 0.100000
         f 0.200000
         f 0.300000
   i 1
   f 2.300000
   b 208 / hTdScmtbb
      i 81985529216486895
      i TRUE
      i 0.999900
      i 115
      i X
      m 255 (0xff) 247 (0xf7) 170 (0xaa) 0 (0x00) 
      t 00000001.80000000
      b 84 / TFNIb
         i TRUE
         i FALSE
         i NIL
         i INFINITUM
         b 68 / sif
            s "a string grouped with an int and a float"
            i 42
            f 0.123000
      b 68 / sif
         s "a string grouped with an int and a float"
         i 42
         f 0.123000

*/

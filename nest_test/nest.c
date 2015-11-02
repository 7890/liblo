#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lo/lo.h"

//tb/151101

//gcc -o nest nest.c -llo && ./nest > /tmp/a && hexdump -c /tmp/a

lo_blob sub(const char *path, const char *types, ... );

//=============================================================================
int main(int argc, char *argv[])
{
	const float float_array[12] = {-123.456789,0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,100000};
	lo_blob bfloat = lo_blob_new(sizeof(float_array), float_array);

	const float int_array[12] = {-100000,0,1,2,3,4,5,6,7,8,9,100000};
	lo_blob bint = lo_blob_new(sizeof(int_array), int_array);

	uint8_t midi_data[4] = { 0xff, 0xf7, 0xAA, 0x00 };

	//lo_timetag timetag;
	//lo_timetag_now(&timetag);

	lo_timetag timetag = { 0x1, 0x80000000 };
	//lo_timetag timetag = { 0x1, 0x00000000 };

	lo_blob b1=sub("/b1","sif","a string grouped with an int and a float",42,0.123);
	lo_blob b2=
	sub("/b2","bbbifb"
		,b1 //containing previously created msg blob
		,bfloat //containing (non-msg) float array blob
		,sub("/x","sb"
			,"named float list"
			,sub("/","fff" //
				,0.1
				,0.2
				,0.3
			)
		)
		,1
		,2.3
		,sub("/","hTdScmtbb" //param without arg in-between
			,0x0123456789abcdefULL //int64
			//(true)
			,0.9999 //double
			,"sym" //symbol
			,'X' //char
			,midi_data
			,timetag
			,sub("/hi","TFNIbbb"
				//(true)
				//(false)
				//(nil)
				//(infinitum)
				,b1
				,sub("/tb","cb" //typed blob
					,'f'
					,bfloat
				)
				,sub("/tb","cb" //typed blob
					,'i'
					,bint
				)

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

//trying to wrap lo_message_add
//=============================================================================
lo_blob sub(const char *path, const char *types, ... )
{
	lo_message msg=lo_message_new();
	//const char* path="/";

	va_list ap;
	va_start(ap, types); //last non-... arg

	//from src/message.c, int lo_message_add(lo_message msg, const char *types, ...)
	//return lo_message_add_varargs_internal(msg, types, ap, file, line);
	lo_message_add_varargs(msg,types,ap);
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

/*
expected output for this example:

raw, 392 bytes
(here base64 encoded)

L3Rlc3QAAAAsaWZzYgAAAAAAAIA+TMzNZm9vAAAAAiQvYjIALGJiYmlmYgAAAABAL2IxACxzaWYA
AAAAYSBzdHJpbmcgZ3JvdXBlZCB3aXRoIGFuIGludCBhbmQgYSBmbG9hdAAAAAAAAAAqPfvnbQAA
ADDg6fbCAAAAAM3MzD3NzEw+mpmZPs3MzD4AAAA/mpkZPzMzMz/NzEw/ZmZmPwBQw0cAAAA4L3gA
ACxzYgBuYW1lZCBmbG9hdCBsaXN0AAAAAAAAABgvAAAALGZmZgAAAAA9zMzNPkzMzT6ZmZoAAAAB
QBMzMwAAAVgvAAAALGhUZFNjbXRiYgAAASNFZ4mrze8/7/8uSOinHnN5bQAAAABY//eqAAAAAAGA
AAAAAAAA3C9oaQAsVEZOSWJiYgAAAAAAAABAL2IxACxzaWYAAAAAYSBzdHJpbmcgZ3JvdXBlZCB3
aXRoIGFuIGludCBhbmQgYSBmbG9hdAAAAAAAAAAqPfvnbQAAAEAvdGIALGNiAAAAAGYAAAAw4On2
wgAAAADNzMw9zcxMPpqZmT7NzMw+AAAAP5qZGT8zMzM/zcxMP2ZmZj8AUMNHAAAAQC90YgAsY2IA
AAAAaQAAADAAUMPHAAAAAAAAgD8AAABAAABAQAAAgEAAAKBAAADAQAAA4EAAAABBAAAQQQBQw0cA
AABAL2IxACxzaWYAAAAAYSBzdHJpbmcgZ3JvdXBlZCB3aXRoIGFuIGludCBhbmQgYSBmbG9hdAAA
AAAAAAAqPfvnbQ==

expanded (see expand.c)

/test ifsb (580 bytes)
i 128
f 0.200000
s "foo"
b(M) /b2 bbbifb (548 bytes)
   b(M) /b1 sif (64 bytes)
      s "a string grouped with an int and a float"
      i 42
      f 0.123000
   b 52 (blob with unknown encoding)
   b(M) /x sb (56 bytes)
      s "named float list"
      b(M) / fff (24 bytes)
         f 0.100000
         f 0.200000
         f 0.300000
   i 1
   f 2.300000
   b(M) / hTdScmtbb (344 bytes)
      h 81985529216486895
      T TRUE
      d 0.999900
      S 115
      c X
      m 255 (0xff) 247 (0xf7) 170 (0xaa) 0 (0x00) 
      t 00000001.80000000
      b(M) /hi TFNIbbb (220 bytes)
         T TRUE
         F FALSE
         N NIL
         I INFINITUM
         b(M) /b1 sif (64 bytes)
            s "a string grouped with an int and a float"
            i 42
            f 0.123000
         b(M) /tb cb (64 bytes)
            f 12 {-123.456787 0.000000 0.100000 0.200000 0.300000 0.400000 0.500000 0.600000 0.700000 0.800000 0.900000 100000.000000}
         b(M) /tb cb (64 bytes)
            i 12 {-100000 0 1 2 3 4 5 6 7 8 9 100000}
      b(M) /b1 sif (64 bytes)
         s "a string grouped with an int and a float"
         i 42
         f 0.123000

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

#include "lo/lo.h"

//tb/151101

//gcc -o nest nest.c -llo && ./nest > /tmp/a && hexdump -c /tmp/a

static lo_blob sub(const char *path, const char *types, ... );

static uint32_t *ptr_to_blobs[1000];
static uint32_t ptr_index=0;

static uint32_t msg_counter=0;

//=============================================================================
static void free_blobs()
{
	int i=0;
	for(i=0;i<ptr_index+1;i++)
	{
		free(ptr_to_blobs[i]);
	}
	ptr_index=0;
}

//=============================================================================
static lo_blob blob_new(int32_t size, const void *data)
{
	lo_blob b = lo_blob_new(size,data);
	ptr_to_blobs[ptr_index]=b;
	//fprintf(stderr,"ptr_index %d\n",ptr_index);
	ptr_index++;
	return b;
}

//=============================================================================
static unsigned char* create_demo_dump(uint32_t *size)
{
	const float float_array[12] = {-123.456789,0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,100000};
	lo_blob bfloat = blob_new(sizeof(float_array), float_array); //note != lo_blob_new

	const int32_t int_array[12] = {-100000,0,1,2,3,4,5,6,7,8,9,100000};
	lo_blob bint = blob_new(sizeof(int_array), int_array);

	const uint8_t midi_data[4] = { 0xff, 0xf7, 0xAA, 0x00 };

	//lo_timetag timetag;
	//lo_timetag_now(&timetag);

	const lo_timetag timetag = { 0x1, 0x80000000 };
	//lo_timetag timetag = { 0x1, 0x00000000 };

	const lo_blob b1=sub("/b1","sif","a string grouped with an int and a float",42,0.123);
	const lo_blob b2=
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
	char * path;

	//some pseudo variance
	if(msg_counter!=0 && msg_counter % 3 == 0)
	{
		path="/another";
		lo_message_add(msg,"hfsb",msg_counter,0.3,"baz",b1);
	}
	else
	{
		path="/test";
		lo_message_add(msg,"hfsb",msg_counter,0.2,"foo",b2);
	}

	//prepare to write serialised message to stdout for later use
//	const 
	uint32_t msg_length=lo_message_length(msg,path);
	void * msg_bytes=calloc(msg_length,sizeof(char));
	size_t size_ret;
	lo_message_serialise (msg, path, msg_bytes, &size_ret);
	msg_counter++;
	fprintf(stderr,"serialized %lu bytes\n",size_ret);

/*
!!!!!!
//use blob_new() and free_blobs()
	lo_blob_free(bfloat);
	lo_blob_free(bint);
	lo_blob_free(b1);
	lo_blob_free(b2);
*/

	lo_message_free(msg);
//	free(msg_bytes);

	(*size)=msg_length;;

	//caller must clean up
	return msg_bytes;
}

//=============================================================================
static void dump_memory(unsigned char *q, const uint32_t len)
{
	int k;
	for (k = 0; k <len; k++)
	{
		printf("%c",q[k]);
	}
	fflush(stdout);
	fprintf(stderr,"\n");
}

//=============================================================================
static uint32_t dump_header(const uint32_t len, const uint32_t prev_pos)
{
	lo_message msg=lo_message_new();
	lo_message_add(msg,"hh",len,prev_pos);
	const char * path="/.";

	uint32_t msg_length=lo_message_length(msg,path);
	void * msg_bytes=calloc(msg_length,sizeof(char));
	size_t size_ret;
	lo_message_serialise (msg, path, msg_bytes, &size_ret);
	fprintf(stderr,"serialized %lu bytes\n",size_ret);

	dump_memory(msg_bytes,size_ret);
	lo_message_free(msg);
	free(msg_bytes);

	return size_ret;
}

//=============================================================================
static uint32_t test(const int with_header, const uint32_t prev_pos)
{
	unsigned char *q=NULL;
	uint32_t size;
	uint32_t total_size;
	q=create_demo_dump(&size);
	fprintf(stderr,"==dumping %"PRId32" bytes\n",size);

	total_size=size;

	if(with_header==1)
	{
		total_size+=dump_header(size,prev_pos);
	}

	dump_memory(q,size);
	free(q);

	return total_size;
}

//=============================================================================
static void test_multi()
{
	uint32_t size=0;
	uint32_t pos=0;
	uint32_t prev_pos=0;

	//create pseudo linked list for experimental parsers
	int i=0;
	for(i=0;i<100000;i++)
//	while(1==1) //check for memory leaks
	{
		//write header (/. h) followed by raw osc message
		prev_pos=pos-size;
		size=test(1,prev_pos);
		pos+=size;
		//clean up all blobs created by test()
		free_blobs();
	}

/*
a) ==header tells next msg is 580 bytes long, prev started at pos 0 //special case (parser can ignore if at pos 0)
b) ==header tells next msg is 580 bytes long, prev started at pos 0 //first usable reference (to first record at start of file)
c) ==header tells next msg is 580 bytes long, prev started at pos 604 //start of prev header
d) ==header tells next msg is 580 bytes long, prev started at pos 1812 //start of prev header
...

hexdump -s 1812 -n 24 -c osc.dump 
0000714   /   .  \0  \0   ,   h   h  \0  \0  \0  \0  \0  \0  \0 002   D
0000724  \0  \0  \0  \0  \0  \0  \a 024                                

*/
}

//=============================================================================
int main(int argc, char *argv[])
{
	//write raw osc message bytes to stdout
//	test(0,0);

	//write raw osc message bytes to stdout with heder
//	test(1,0);

	test_multi();

	return 0;
}

//trying to wrap lo_message_add
//=============================================================================
static lo_blob sub(const char *path, const char *types, ... )
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

	lo_blob b=blob_new(lo_message_length(msg,path),0);
	size_t size_ret;
	lo_message_serialise (msg, path, lo_blob_dataptr(b), &size_ret);
	lo_message_free(msg);

	fprintf(stderr,"serialized %lu bytes\n",size_ret);
	fprintf(stderr,"blob size %d bytes\n",lo_blobsize(b));

	//blob to be freed by caller (via free_blobs())
	return b;
}

/*
expected output for this example:

(here base64 encoded)

L3Rlc3QAAAAsaWZzYgAAAAAAAIA+TMzNZm9vAAAAAiQvYjIALGJiYmlmYgAAAABAL2IxACxzaWYA
AAAAYSBzdHJpbmcgZ3JvdXBlZCB3aXRoIGFuIGludCBhbmQgYSBmbG9hdAAAAAAAAAAqPfvnbQAA
ADDg6fbCAAAAAM3MzD3NzEw+mpmZPs3MzD4AAAA/mpkZPzMzMz/NzEw/ZmZmPwBQw0cAAAA4L3gA
ACxzYgBuYW1lZCBmbG9hdCBsaXN0AAAAAAAAABgvAAAALGZmZgAAAAA9zMzNPkzMzT6ZmZoAAAAB
QBMzMwAAAVgvAAAALGhUZFNjbXRiYgAAASNFZ4mrze8/7/8uSOinHnN5bQAAAABY//eqAAAAAAGA
AAAAAAAA3C9oaQAsVEZOSWJiYgAAAAAAAABAL2IxACxzaWYAAAAAYSBzdHJpbmcgZ3JvdXBlZCB3
aXRoIGFuIGludCBhbmQgYSBmbG9hdAAAAAAAAAAqPfvnbQAAAEAvdGIALGNiAAAAAGYAAAAw4On2
wgAAAADNzMw9zcxMPpqZmT7NzMw+AAAAP5qZGT8zMzM/zcxMP2ZmZj8AUMNHAAAAQC90YgAsY2IA
AAAAaQAAADBgef7/AAAAAAEAAAACAAAAAwAAAAQAAAAFAAAABgAAAAcAAAAIAAAACQAAAKCGAQAA
AABAL2IxACxzaWYAAAAAYSBzdHJpbmcgZ3JvdXBlZCB3aXRoIGFuIGludCBhbmQgYSBmbG9hdAAA
AAAAAAAqPfvnbQ==

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

expanded (see expand.c)

*/

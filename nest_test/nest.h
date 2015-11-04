#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

#include "lo/lo.h"

//tb/151101

static lo_blob blob_new(int32_t size, const void *data);
static void free_blobs();
static void dump_memory(unsigned char *q, const uint32_t len);
static uint32_t dump_header(const uint32_t len, const uint64_t prev_pos);
static lo_blob b_internal(const char *path, const char *types, ... );

#define b(path,types...) \
	b_internal(path,types,LO_ARGS_END)

#define MAX_BLOBS_PER_MESSAGE 1000

static uint32_t *ptr_to_blobs[MAX_BLOBS_PER_MESSAGE];
static uint32_t ptr_index=0;

static uint64_t msg_counter=0;

//=============================================================================
static lo_blob blob_new(int32_t size, const void *data)
{
	lo_blob b = lo_blob_new(size,data);
	ptr_to_blobs[ptr_index]=b;
//	fprintf(stderr,"ptr_index %d\n",ptr_index);
	ptr_index++;
	return b;
}

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
static uint32_t dump_header(const uint32_t len, const uint64_t prev_pos)
{
	lo_message msg=lo_message_new();
	lo_message_add(msg,"ih",len,prev_pos);
	const char * path="/.";

	uint32_t msg_length=lo_message_length(msg,path);
	void * msg_bytes=calloc(msg_length,sizeof(char));
	size_t size_ret;
	lo_message_serialise (msg, path, msg_bytes, &size_ret);
//	fprintf(stderr,"serialized %lu bytes\n",size_ret);

	dump_memory(msg_bytes,size_ret);
	lo_message_free(msg);
	free(msg_bytes);

	return size_ret;
}

//=============================================================================
static lo_blob b_internal(const char *path, const char *types, ... )
{
	lo_message msg=lo_message_new();

	va_list ap;
	va_start(ap, types); //last non-... arg

	lo_message_add_varargs(msg,types,ap);

	lo_blob b=blob_new(lo_message_length(msg,path),0);
	size_t size_ret;
	lo_message_serialise (msg, path, lo_blob_dataptr(b), &size_ret);
	lo_message_free(msg);

//	fprintf(stderr,"serialized %lu bytes\n",size_ret);
	fprintf(stderr,"blob size %d bytes\n",lo_blobsize(b));

	//blob to be freed by caller (via free_blobs())
	return b;
}

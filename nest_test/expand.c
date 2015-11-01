#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lo/lo.h"

/*
//tb/151101

//gcc -o expand expand.c -llo && ./expand /tmp/a

//example of /tmp/a (base64 encoded)
//extract to binary: echo -n "<string below>" | base64 -d > /tmp/a

L3Rlc3QAAAAsaWZzYgAAAAAAAIA+TMzNZm9vAAAAAHAvAAAALGJiYmlpZgAAAAAULwAAACxzAABndWd1cyBhYWFhAAAAAAAMzczMPc3MTD6amZk+AAAALC8AAAAsc2IAaW5uZXIAAAAAAAAYLwAAACxmZmYAAAAAPczMzT5MzM0+mZmaAAAAAQAAAAJAWZma

//full test

echo -n "L3Rlc3QAAAAsaWZzYgAAAAAAAIA+TMzNZm9vAAAAAHAvAAAALGJiYmlpZgAAAAAULwAAACxzAABndWd1cyBhYWFhAAAAAAAMzczMPc3MTD6amZk+AAAALC8AAAAsc2IAaW5uZXIAAAAAAAAYLwAAACxmZmYAAAAAPczMzT5MzM0+mZmaAAAAAQAAAAJAWZma" | base64 -d > /tmp/a && gcc -o expand expand.c -llo && ./expand /tmp/a

//alternatively use nest.c

//expected structure / print output

/test ifsb
i 128
f 0.200000
s foo
b 116 / bbbiif
   b 24 / s
      s gugus aaaa
   b 16 (blob with unknown encoding)
   b 48 / sb
      s inner
      b 28 / fff
         f 0.100000
         f 0.200000
         f 0.300000
   i 1
   i 2
   f 3.400000
*/

void print_blob(lo_blob b, int indent);
void print_indent(int indent);
void print_msg(lo_message msg, const char * path, int indent);

void print_blob(lo_blob b, int indent)
{
/*
	unsigned char *q = lo_blob_dataptr(b);
	int k;
	for (k = 0; k < lo_blobsize(b)-4; k++) //-4 !
	{
		fprintf(stderr,"%c",q[k]);
	}
	fprintf(stderr,"\n");
*/

	const char *path = lo_get_path(lo_blob_dataptr(b), lo_blobsize(b));

	if(*path!='/')
	{
		fprintf(stderr,"(blob with unknown encoding)\n");
	}
	else
	{
		lo_message msg=lo_message_deserialise(lo_blob_dataptr(b),lo_blobsize(b)-4,NULL); //-4!
		indent++;
		//======
		print_msg(msg,path,indent);
		lo_message_free(msg);
	}
}

void print_indent(int indent)
{
	int spaces=3;
	int i;
	for(i=0;i<indent*spaces;i++)
	{
		fprintf(stderr," ");
	}
}

void print_msg(lo_message msg, const char * path, int indent)
{
	int arg_count=lo_message_get_argc(msg);
	const char *types=lo_message_get_types(msg);
	lo_arg ** arg_values=lo_message_get_argv(msg);

	fprintf(stderr,"%s %s\n",path,types);

	int i;
	for(i=0;i<arg_count;i++)
	{
		if(types[i]=='b')
		{
			print_indent(indent);
			fprintf(stderr,"b %d ",lo_blobsize((lo_blob)arg_values[i]));
			print_blob((lo_blob)arg_values[i],indent);
		}
		else if(types[i]=='s')
		{
			print_indent(indent);
			fprintf(stderr,"s %s\n",&arg_values[i]->s);
		}
		else if(types[i]=='i')
		{
			print_indent(indent);
			fprintf(stderr,"i %d\n",arg_values[i]->i);

		}
		else if(types[i]=='f')
		{
			print_indent(indent);
			fprintf(stderr,"f %f\n",arg_values[i]->f);
		}
	}

	return;
}

int main(int argc, char *argv[])
{
	//http://stackoverflow.com/questions/14002954/c-programming-how-to-read-the-whole-file-contents-into-a-buffer
	FILE *f = fopen(argv[1], "rb");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *bytes = malloc(fsize);
	fread(bytes, fsize, 1, f);
	fclose(f);

	const char *path = lo_get_path(bytes, fsize);
	lo_message msg=lo_message_deserialise(bytes,fsize,NULL);

	//======
	print_msg(msg,path,0);
	//print_msg(msg,path,0);

	lo_message_free(msg);

	return 0;
}

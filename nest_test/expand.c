#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lo/lo.h"

/*
//tb/151101

//gcc -o expand expand.c -llo && ./expand osc.dump

//also see nest.c

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
		else if(types[i]=='f')
		{
			print_indent(indent);
			fprintf(stderr,"f %f\n",arg_values[i]->f);
		}
		else if(types[i]=='i')
		{
			print_indent(indent);
			fprintf(stderr,"i %d\n",arg_values[i]->i);

		}
		else if(types[i]=='s')
		{
			print_indent(indent);
			fprintf(stderr,"s \"%s\"\n",&arg_values[i]->s);
		}
		else if(types[i]=='m')
		{
			print_indent(indent);
			fprintf(stderr,"m ");
			int w;
			for(w=0;w<4;w++)
			{
				fprintf(stderr,"%d (0x%02x) ",(arg_values[i]->m)[w],(arg_values[i]->m)[w]);
			}

			fprintf(stderr,"\n");
		}
		else if(types[i]=='h')
		{
			print_indent(indent);
			fprintf(stderr,"i %lu\n",arg_values[i]->h);
		}
		else if(types[i]=='t')
		{
			print_indent(indent);
			lo_timetag tt=(lo_timetag)arg_values[i]->t;
/*
uint32_t lo_timetag::frac
The fractions of a second offset from above, expressed as 1/2^32nds of a second

uint32_t lo_timetag::sec
The number of seconds since Jan 1st 1900 in the UTC timezone.
*/
			fprintf(stderr,"t %08x.%08x\n",tt.sec,tt.frac);
		}
		else if(types[i]=='d')
		{
			print_indent(indent);
			fprintf(stderr,"i %f\n",arg_values[i]->d);
		}
		else if(types[i]=='S')
		{
			print_indent(indent);
			fprintf(stderr,"i %d\n",arg_values[i]->S);
		}
		else if(types[i]=='c')
		{
			print_indent(indent);
			fprintf(stderr,"i %c\n",arg_values[i]->c);
		}
		else if(types[i]=='T')
		{
			print_indent(indent);
			fprintf(stderr,"i TRUE\n");
		}
		else if(types[i]=='F')
		{
			print_indent(indent);
			fprintf(stderr,"i FALSE\n");
		}
		else if(types[i]=='N')
		{
			print_indent(indent);
			fprintf(stderr,"i NIL\n");
		}
		else if(types[i]=='I')
		{
			print_indent(indent);
			fprintf(stderr,"i INFINITUM\n");
		}
		else
		{
			print_indent(indent);
			fprintf(stderr,"%c UNKNOWN TYPE\n",types[i]);
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

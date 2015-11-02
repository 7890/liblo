#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lo/lo.h"

/*
//tb/151101

//gcc -o expand expand.c -llo && ./expand osc.dump

//also see nest.c

/*
lo_blobsize:
A function to calculate the amount of OSC message space required by a lo_blob object.
Returns the storage size in bytes, which will always be a multiple of four.

//padded, inclduing length header
uint32_t lo_blobsize(lo_blob b)
{
    const uint32_t len = sizeof(uint32_t) + b->size;
    return 4 * ((len + 3) / 4);
}

//data body
uint32_t lo_blob_datasize(lo_blob b)
{
    return b->size;
}

lo_blob_datasize < lo_blobsize
*/

static void print_blob(lo_blob b, int indent);
static void print_indent(int indent);
static void print_msg(lo_message msg, const char * path, int indent);

static int bflat_pos=0;
static int vflat_pos=0;

//=============================================================================
void print_blob(const lo_blob b, int indent)
{
/*
	unsigned char *q = lo_blob_dataptr(b);
	int k;
	for (k = 0; k < lo_blob_datasize(b); k++)
	{
		fprintf(stderr,"%c",q[k]);
	}
	fprintf(stderr,"\n");
*/

	const char *path = lo_get_path(lo_blob_dataptr(b), lo_blobsize(b));

	if(*path!='/')
	{
		fprintf(stderr,"b %d ",lo_blobsize(b));
		fprintf(stderr,"(blob with unknown encoding)\n");
	}
	else
	{
//		fprintf(stderr,"b(M) (%d + %d bytes) ",lo_blobsize(b)-lo_blob_datasize(b),lo_blob_datasize(b));
		fprintf(stderr,"b(M) ");

		lo_message msg=lo_message_deserialise(lo_blob_dataptr(b),lo_blob_datasize(b),NULL);
		indent++;
		bflat_pos++;
		//======
		print_msg(msg,path,indent);
		lo_message_free(msg);
	}
}

//=============================================================================
void print_indent(const int indent)
{
	const int spaces=3;
	int i;
	for(i=0;i<indent*spaces;i++)
	{
		fprintf(stderr," ");
	}
}

//=============================================================================
void print_msg(const lo_message msg, const char * path, int indent)
{
	const int arg_count=lo_message_get_argc(msg);
	const char *types=lo_message_get_types(msg);
	lo_arg ** arg_values=lo_message_get_argv(msg);

	fprintf(stderr,"%s %s (%lu bytes)\n",path,types,lo_message_length(msg,path));

	if(!strcmp(path,"/tb") && !strcmp(types,"cb"))
	{
		const lo_blob b=(lo_blob)arg_values[1];
		const uint32_t size = lo_blobsize(b);
		const uint32_t dsize = lo_blob_datasize(b);

		//handle typed blob
		if(arg_values[0]->c=='f')
		{
			const float *q=lo_blob_dataptr(b);

			const int num=dsize/sizeof(float);

			print_indent(indent);
			fprintf(stderr,"f %d {",num);

			int k;
			for (k = 0; k < num; k++)
			{
				fprintf(stderr,"%f",q[k]); //default
//				fprintf(stderr,"%.2f",q[k]); //limit 
				if(k<num-1)
				{
					fprintf(stderr," ");
				}
		
			}
			fprintf(stderr,"}\n");

			vflat_pos++;
			vflat_pos++;
			return;
		}
		else if(arg_values[0]->c=='i')
		{
			///
			const float *q=lo_blob_dataptr(b);

			const int num=dsize/sizeof(int32_t);

			print_indent(indent);
			fprintf(stderr,"i %d {",num);
			int k;
			for (k = 0; k < num; k++)
			{
				fprintf(stderr,"%d",(int32_t)q[k]);
				if(k<num-1)
				{
					fprintf(stderr," ");
				}
			}
			fprintf(stderr,"}\n");

			vflat_pos++;
			vflat_pos++;
			return;
		}
	}

	int i;
	for(i=0;i<arg_count;i++)
	{
		print_indent(indent);
		if(types[i]=='b')
		{
			//fprintf(stderr,"+ %d) ",i);
			print_blob((lo_blob)arg_values[i],indent);
		}
		else if(types[i]=='f')
		{
			//fprintf(stderr,"%d %d) ",vflat_pos,i);
			fprintf(stderr,"f %f\n",arg_values[i]->f);
		}
		else if(types[i]=='i')
		{
			//fprintf(stderr,"%d %d) ",vflat_pos,i);
			fprintf(stderr,"i %d\n",arg_values[i]->i);

		}
		else if(types[i]=='s')
		{
			//fprintf(stderr,"%d %d) ",vflat_pos,i);
			fprintf(stderr,"s \"%s\"\n",&arg_values[i]->s);
		}
		else if(types[i]=='m')
		{
			//fprintf(stderr,"%d %d) ",vflat_pos,i);
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
			//fprintf(stderr,"%d %d) ",vflat_pos,i);
			fprintf(stderr,"h %lu\n",arg_values[i]->h);
		}
		else if(types[i]=='t')
		{
			//fprintf(stderr,"%d %d) ",vflat_pos,i);
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
			//fprintf(stderr,"%d %d) ",vflat_pos,i);
			fprintf(stderr,"d %f\n",arg_values[i]->d);
		}
		else if(types[i]=='S')
		{
			//fprintf(stderr,"%d %d) ",vflat_pos,i);
			fprintf(stderr,"S %d\n",arg_values[i]->S);
		}
		else if(types[i]=='c')
		{
			//fprintf(stderr,"%d %d) ",vflat_pos,i);
			fprintf(stderr,"c %c\n",arg_values[i]->c);
		}
		else if(types[i]=='T')
		{
			//fprintf(stderr,"%d %d) ",vflat_pos,i);
			fprintf(stderr,"T TRUE\n");
		}
		else if(types[i]=='F')
		{
			//fprintf(stderr,"%d %d) ",vflat_pos,i);
			fprintf(stderr,"F FALSE\n");
		}
		else if(types[i]=='N')
		{
			//fprintf(stderr,"%d %d) ",vflat_pos,i);
			fprintf(stderr,"N NIL\n");
		}
		else if(types[i]=='I')
		{
			//fprintf(stderr,"%d %d) ",vflat_pos,i);
			fprintf(stderr,"I INFINITUM\n");
		}
		else
		{
			//fprintf(stderr,"%d %d) ",vflat_pos,i);
			fprintf(stderr,"%c UNKNOWN TYPE\n",types[i]);
		}

		///
		if(types[i]!='b')
		{
			vflat_pos++;
		}
	}

	return;
}

//=============================================================================
int main(int argc, char *argv[])
{
	//http://stackoverflow.com/questions/14002954/c-programming-how-to-read-the-whole-file-contents-into-a-buffer
	FILE *f = fopen(argv[1], "rb");
	fseek(f, 0, SEEK_END);
	const uint32_t fsize = ftell(f);
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

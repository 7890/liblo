#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <stdio.h>

#include <lo/lo.h>

#include <chaiscript/chaiscript.hpp>
#include <chaiscript/chaiscript_stdlib.hpp>

#include "hexdump.h"

//tb/1603
//test chaiscript with liblo
//g++ -std=c++0x -o example2 example2.cpp `pkg-config --libs --cflags chaiscript liblo` -ldl

lo_server_thread st=NULL;
chaiscript::ChaiScript *chai=NULL;

static const char* hexdump_header="= hexdump ===============================================================";

//=============================================================================
int generic_handler(const char *path, const char *types, lo_arg ** argv,
	int argc, void *data, void *user_data)
{
//	fprintf(stderr,"got message for callback: %s\n",(char*)user_data);
	lo_address src = lo_message_get_source(data);
	std::stringstream ss_addr;
	std::string str_addr;
	ss_addr << "OSCAddress(\"" << lo_address_get_hostname(src) << "\"," << lo_address_get_port(src) << ")";
	str_addr=ss_addr.str();
//	fprintf(stderr,"EVALSTRING %s\n",str_addr.c_str());
	std::stringstream ss_msg;
	std::string str_msg;
	ss_msg << "OSCMessage(\"" << path << "\").add_args(\"" << types << "\",[";
	int i=0;
	for(;i<argc;i++)
	{
//		fprintf(stderr,"TYPE %c\n",types[i]);
		if(i>0){ss_msg << ",";}
		if(types[i]=='f')	{ss_msg << argv[i]->f;}
		else if(types[i]=='d')	{ss_msg << argv[i]->d;}
		else if(types[i]=='i')	{ss_msg << argv[i]->i;}
		else if(types[i]=='h')	{ss_msg << argv[i]->h;}
		else if(types[i]=='s')	{ss_msg << "\"" << &argv[i]->s 	<< "\"";}
		else if(types[i]=='c')	{ss_msg << "'" 	<< argv[i]->c 	<< "'";}
		///.......
	}
	//close args list, OSCMessage
	ss_msg << "])";

	str_msg=ss_msg.str();
//	fprintf(stderr,"EVALSTRING %s\n",str_msg.c_str());
	std::stringstream ss_final;
	std::string str_final;
	ss_final << (char*)user_data << "(" << str_addr << "," << str_msg << ")";
	str_final=ss_final.str();
//	fprintf(stderr,"EVALSTRING %s\n",str_final.c_str());
	//evaluate string (call function in chaiscript environment with built-up objects as arguments)
	chai->eval(str_final);
	return 0;
}//end generic_handler()

//=============================================================================
void _lo_server_thread_new(const int port)
{
	char port_str[10];
	sprintf(port_str, "%d", port);
	st = lo_server_thread_new(port_str, NULL);
}

//=============================================================================
void _lo_server_thread_start()
{
	if(st!=NULL)
	{
		lo_server_thread_start(st);
	}
}

//=============================================================================
void _lo_server_thread_add_method(
	const std::string &path_filter
	,const std::string &types_filter
	,const std::string &callback_name
)
{
	if(st!=NULL)
	{
		lo_server_thread_add_method(st, path_filter.c_str(), types_filter.c_str()
			,generic_handler, (void*)(callback_name.c_str()));
	}
}

//=============================================================================
void _lo_server_thread_add_method_generic(const std::string &callback_name)
{
	if(st!=NULL)
	{
		lo_server_thread_add_method(st,NULL,NULL,generic_handler, (void*)(callback_name.c_str()));
	}
}

//=============================================================================
void process_chai_message(
	lo_message &m
	,const std::string &path
	,const std::string &types
	,const std::vector<chaiscript::Boxed_Value> &vargs

)
{
	int i=0;
	for(;i<types.size();i++)
	{
		char c=types[i];
		if(c=='i')
		{
			lo_message_add_int32(m, (int)chaiscript::Boxed_Number( vargs.at(i) ).get_as<int>() );
		}
		else if(c=='h')
		{
			lo_message_add_int64(m, (long)chaiscript::Boxed_Number( vargs.at(i) ).get_as<long>() );
		}
		else if(c=='f')
		{
			lo_message_add_float(m, (float)chaiscript::Boxed_Number( vargs.at(i) ).get_as<float>() );
		}
		else if(c=='d')
		{
			lo_message_add_double(m, (double)chaiscript::Boxed_Number( vargs.at(i) ).get_as<double>() );
		}
		else if(c=='s')
		{
			lo_message_add_string(m, (chaiscript::boxed_cast<const std::string>( vargs.at(i) )).c_str() );
		}
/*
		else if(c=='c')
		{
			lo_message_add_char(m,'?');
		}
*/
		else
		{
			fprintf(stderr,"unknown type: %c\n",c);
		}
	}
}//end process_chai_message()

//=============================================================================
void _lo_message_send(
	const std::string &host
	,const int &port
	,const std::string &path
	,const std::string &types
	,const std::vector<chaiscript::Boxed_Value> &vargs
)
{
//	fprintf(stderr,"_send called size types %zu args %zu\n",types.size(),vargs.size());
	lo_address a=lo_address_new(host.c_str(), (std::to_string(port)).c_str());
	lo_message m=lo_message_new();

	process_chai_message(m,path,types,vargs);

	lo_send_message(a,path.c_str(),m);
	lo_address_free(a);
	lo_message_free(m);
}//end _send()

//=============================================================================
void _lo_server_enable_coercion(const int &int_bool)
{
	lo_server_enable_coercion (lo_server_thread_get_server(st), int_bool);
}

//=============================================================================
void _lo_message_hexdump(
	const std::string &path
	,const std::string &types
	,const std::vector<chaiscript::Boxed_Value> &vargs
)
{
	lo_message m=lo_message_new();
	process_chai_message(m,path,types,vargs);
	size_t mlen=lo_message_length(m,path.c_str());
	void *mbuf=malloc(mlen);
	lo_message_serialise(m,path.c_str(),mbuf,NULL);
/*
= hexdump ===============================================================
  0000  2f 68 65 6c 6c 6f 00 00 2c 73 00 00 77 6f 72 6c  /hello..,s..worl
  0010  64 00 00 00                                      d...
*/
	hexdump (hexdump_header,mbuf, mlen);
	lo_message_free(m);
	free(mbuf);
}

//=============================================================================
int main(int argc, char *argv[])
{
	if(argc<2)
	{
		fprintf(stderr,"syntax: <chaiscript file> (args ...)\n");
		return 1;
	}

	chai= new chaiscript::ChaiScript(chaiscript::Std_Lib::library());

	chai->add(chaiscript::fun(_lo_server_thread_new), "_lo_server_thread_new");
	chai->add(chaiscript::fun(_lo_server_thread_start), "_lo_server_thread_start");
	chai->add(chaiscript::fun(_lo_server_thread_add_method), "_lo_server_thread_add_method");
	chai->add(chaiscript::fun(_lo_server_thread_add_method_generic), "_lo_server_thread_add_method_generic");
	chai->add(chaiscript::fun(_lo_server_enable_coercion), "_lo_server_enable_coercion");
	chai->add(chaiscript::fun(_lo_message_send), "_lo_message_send");
	chai->add(chaiscript::fun(_lo_message_hexdump), "_lo_message_hexdump");


	std::stringstream ss_args;
	//"import" OSC specific classes to script environment
	ss_args << "use(\"OSC.chai\");\n";
	//create vector with command line arguments
	ss_args << "var args=Vector();\n";
	int i=0;
	for(;i<argc;i++)
	{
		ss_args << "args.push_back(\"" << argv[i] << "\");\n";
	}
//	fprintf(stderr,"EVALSTRING %s\n",ss_args.str().c_str());
	chai->eval(ss_args.str());

	//read script file given as first command line argument
	std::ifstream in(argv[1]);
	std::stringstream buffer;
	buffer << in.rdbuf();
	chai->eval(buffer.str());

/*
	chai->eval(R"(
		OSCMessage("/foo").send("localhost",1234);
		OSCMessage("/foo").add_args("fis",.3e5,42,"bar").send("localhost",1234);
	)");
*/

	while (1==1)
	{
#ifdef WIN32
		Sleep(1);
#else
		usleep(1000);
#endif
	}
}//end main()

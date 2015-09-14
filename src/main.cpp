#include <stdint.h>

typedef uint8_t u8; typedef int8_t s8;
typedef uint16_t u16; typedef int16_t s16;
typedef uint32_t u32; typedef int32_t s32;
typedef uint64_t u64; typedef int64_t s64;
typedef unsigned int uint;


#include <iostream>
#include <string>
#include <array>
using namespace std;


#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>



class pipe_fd_arr
{
public:		// variables
	static constexpr u32 output_index = 0, input_index = 1;
	
	static constexpr u32 pipe_fd_arr_size = 2;
	array< int, pipe_fd_arr_size > the_array;
	
public:		// functions
	inline void make_pipe()
	{
		pipe( the_array.data() );
	}
};



void child_main( pipe_fd_arr& parent_write_fd_arr, 
	pipe_fd_arr& parent_read_fd_arr );

void parent_main( pipe_fd_arr& parent_write_fd_arr,
	pipe_fd_arr& parent_read_fd_arr );


int main( int argc, char** argv )
{
	constexpr u32 pipe_fd_size = 2;
	
	
	constexpr u32 fd_read_index = 0, fd_write_index = 1;
	
	//constexpr u32 buf_size = 80;
	//char buf[buf_size];
	
	pipe_fd_arr parent_write_fd_arr, parent_read_fd_arr;
	
	pid_t childpid;
	
	parent_write_fd_arr.make_pipe();
	parent_read_fd_arr.make_pipe();
	
	
	if ( ( childpid = fork() ) == -1 )
	{
		cout << "There was an error in fork()\n";
		exit(1);
	}
	
	if ( childpid == 0 )
	{
		child_main( parent_write_fd_arr, parent_read_fd_arr );
	}
	else
	{
		parent_main( parent_write_fd_arr, parent_read_fd_arr );
	}
	
	
	
	return 0;
}



void child_main( pipe_fd_arr& parent_write_fd_arr, 
	pipe_fd_arr& parent_read_fd_arr )
{
	constexpr u32 buf_size = 80;
	char buf[buf_size];
	
	constexpr u32 output_index = pipe_fd_arr::output_index,
		input_index = pipe_fd_arr::input_index;
	
	int& child_write_fd = parent_read_fd_arr.the_array[input_index];
	int& child_read_fd = parent_write_fd_arr.the_array[output_index];
	
	
	// Child process closes up the input side of parent_write_fd_arr
	close(parent_write_fd_arr.the_array[input_index]);
	
	// Child process closes up the output side of parent_read_fd_arr
	close(parent_read_fd_arr.the_array[output_index]);
	
	
	string to_send = "Hello from child!\n";
	
	write( child_write_fd, to_send.c_str(), to_send.size() + 1 );
	
	int num_read_bytes = read( child_read_fd, buf, buf_size );
	
	cout << buf;
	
}

void parent_main( pipe_fd_arr& parent_write_fd_arr,
	pipe_fd_arr& parent_read_fd_arr )
{
	constexpr u32 buf_size = 80;
	char buf[buf_size];
	
	constexpr u32 output_index = pipe_fd_arr::output_index,
		input_index = pipe_fd_arr::input_index;
	
	int& parent_write_fd = parent_write_fd_arr.the_array[input_index];
	int& parent_read_fd = parent_read_fd_arr.the_array[output_index];
	
	
	// Parent process closes up the output side of parent_write_fd_arr
	close(parent_write_fd_arr.the_array[output_index]);
	
	// Parent process closes up the input side of parent_read_fd_arr
	close(parent_read_fd_arr.the_array[input_index]);
	
	string to_send = "Hello from parent!\n";
	
	int num_read_bytes = read( parent_read_fd, buf, buf_size );
	
	cout << buf;
	
	write( parent_write_fd, to_send.c_str(), to_send.size() + 1 );
	
}

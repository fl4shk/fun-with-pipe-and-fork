// This file is part of fun-with-pipe-and-fork.
// 
// Copyright 2015-2017 by Andrew Clark (FL4SHK).
// 
// fun-with-pipe-and-fork is free software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
// 
// fun-with-pipe-and-fork is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along
// with fun-with-pipe-and-fork.  If not, see <http://www.gnu.org/licenses/>.

#include <stdint.h>

typedef uint8_t u8; typedef int8_t s8;
typedef uint16_t u16; typedef int16_t s16;
typedef uint32_t u32; typedef int32_t s32;
typedef uint64_t u64; typedef int64_t s64;
typedef unsigned int uint;


#include <iostream>
#include <string>
#include <array>
using std::cout;
using std::cin;
using std::cerr;
using std::endl;


#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>



class pipe_fd_arr
{
public:		// variables
	static constexpr size_t output_index = 0, input_index = 1;
	
	static constexpr size_t arr_size = 2;
	std::array< int, arr_size > the_array;
	
public:		// functions
	inline void make_pipe()
	{
		pipe( the_array.data() );
	}
};


class pipe_master_base
{
public:		// constants
	//static constexpr size_t pipe_fd_size = 2;
	//static constexpr size_t fd_read_index = 0, fd_write_index = 1;
	static constexpr size_t output_index = pipe_fd_arr::output_index,
		input_index = pipe_fd_arr::input_index;
	
private:		// variables
	pid_t internal_some_pid;
	
	pipe_fd_arr internal_orig_write_fd_arr, internal_orig_read_fd_arr;
	
	
public:		// functions
	void run();
	virtual ~pipe_master_base()
	{
	}
	
	
protected:		// functions
	inline int& child_write_fd()
	{
		return orig_read_fd_arr().the_array[input_index];
	}
	inline int& child_read_fd()
	{
		return orig_write_fd_arr().the_array[output_index];
	}
	inline int& parent_write_fd()
	{
		return orig_write_fd_arr().the_array[input_index];
	}
	inline int& parent_read_fd()
	{
		return orig_read_fd_arr().the_array[output_index];
	}
	
	
	// These are intended to be overrided in derived classes
	virtual void run_child() = 0;
	virtual void run_parent() = 0;
	
private:		// functions
	inline pid_t& some_pid()
	{
		return internal_some_pid;
	}
	
	inline pipe_fd_arr& orig_write_fd_arr()
	{
		return internal_orig_write_fd_arr;
	}
	inline pipe_fd_arr& orig_read_fd_arr()
	{
		return internal_orig_read_fd_arr;
	}
	
};

void pipe_master_base::run()
{
	orig_write_fd_arr().make_pipe();
	orig_read_fd_arr().make_pipe();
	
	
	if ( ( some_pid() = fork() ) == -1 )
	{
		std::cerr << "There was an error in fork()\n";
		exit(1);
	}
	
	if ( some_pid() == 0 )
	{
		// Child process closes up the input side of orig_write_fd_arr()
		close(orig_write_fd_arr().the_array[input_index]);
		
		// Child process closes up the output side of orig_read_fd_arr()
		close(orig_read_fd_arr().the_array[output_index]);
		
		run_child();
	}
	else
	{
		// Parent process closes up the output side of orig_write_fd_arr()
		close(orig_write_fd_arr().the_array[output_index]);
		
		// Parent process closes up the input side of orig_read_fd_arr()
		close(orig_read_fd_arr().the_array[input_index]);
		
		run_parent();
	}
}

class pipe_master_example : public pipe_master_base
{
protected:		// functions
	virtual void run_child();
	virtual void run_parent();
	
	
};


void pipe_master_example::run_child()
{
	static constexpr size_t buf_size = 80;
	char buf[buf_size];
	
	const std::string to_send = "Hello from child!\n";
	
	write( child_write_fd(), to_send.c_str(), to_send.size() + 1 );
	
	const ssize_t num_read_bytes = read( child_read_fd(), buf, buf_size );
	
	cout << buf;
}
void pipe_master_example::run_parent()
{
	static constexpr size_t buf_size = 80;
	char buf[buf_size];
	
	const std::string to_send = "Hello from parent!\n";
	
	const ssize_t num_read_bytes = read( parent_read_fd(), buf, buf_size );
	
	cout << buf;
	
	write( parent_write_fd(), to_send.c_str(), to_send.size() + 1 );
}


int main( int argc, char** argv )
{
	pipe_master_example pm;
	pm.run();
	
	return 0;
}


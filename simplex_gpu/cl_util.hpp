#ifndef CL_UTIL_HPP
#define CL_UTIL_HPP

#include <vector>
#include <CL/cl.hpp>
#include <string>

namespace cl_util
{
	class simple_env
	{
	public:
		simple_env();
	    void parse_args(int argc, char *argv[]);
		std::string& get_info();
		cl::Context& get_context() {return *context;};
		cl::CommandQueue& get_queue() {return *queue;};
	public:
		std::vector<cl::Platform> platforms;
		std::vector<cl::Device> devices;
		cl::Device device;
	protected:
		std::string info;
		cl::Context* context;
		cl::CommandQueue* queue;
	};

	std::string load_prog(const std::string& fname);

	std::string get_build_log(const char*);
}

#endif

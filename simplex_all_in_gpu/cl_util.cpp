#include "cl_util.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
using namespace cl;

namespace cl_util
{
    simple_env::simple_env()
	{
		Platform::get(&platforms);

		for (int i = 0; i < platforms.size(); i++)
		{
			cl_uint num = 0;
			vector<Device> plat_devices;
			platforms[i].getDevices(CL_DEVICE_TYPE_ALL, &plat_devices);
			devices.insert(devices.end(), plat_devices.begin(), plat_devices.end());
		}

		device = devices[0];
		vector<Device> tmp;
		tmp.push_back(device);
		context = new Context(tmp);
		queue = new CommandQueue(*context, device);
	}

	void simple_env::parse_args(int argc, char *argv[])
	{
		for (int i = 1; i < argc; i++)
		{
			string val(argv[i]);
			if (val == "--cl_list")
			{
				if (devices.size() == 0)
					cout << "There is no OpenCL devices.\n";
				else
				{
					cout << "OpenCL Devices:\n";
					for (int i = 0; i < devices.size(); i++)
					{
						string name;
						devices[i].getInfo(CL_DEVICE_NAME, &name);
						cout << i << ": " << name << "\n";
					}
					cout << "\n";
				}
				exit(0);
			}
			else if (val == "--cl_dev")
			{
				if (++i >= argc)
				{
					cout << "Invalid OpenCL device index.\n";
					exit(EXIT_FAILURE);
				}
				try
				{
					int id = stoi(argv[i]);
					if (id < devices.size())
					{
						device = devices[id];
						delete context;
						delete queue;
						vector<Device> tmp;
						tmp.push_back(device);
						context = new Context(tmp);
						queue = new CommandQueue(*context, device);
					}
				}
				catch (invalid_argument const &e)
				{
					cout << "OpenCL index is not number.\n";
				}
				catch (out_of_range const &e)
				{
					cout << "OpenCL index is out of range.\n";
				}
			}
		}
	}

	string& simple_env::get_info()
	{
		device.getInfo(CL_DEVICE_NAME, &info);
		return info;
	}

	string load_prog(const string& fname)
	{
		ifstream f(fname);
		stringstream ss;
		ss << f.rdbuf();
		return string(ss.str());
	}

	string get_build_log(const char* fname)
	{
		vector<Platform> platforms;
		Platform::get(&platforms);
		Platform plat = platforms[0];
		vector<Device> devs;
		plat.getDevices(CL_DEVICE_TYPE_ALL, &devs);
		Device dev = devs[0];
		vector<Device> tmp;
		tmp.push_back(dev);
		cl::Context	context(tmp);

		cl::Program prog(context, cl_util::load_prog(fname), true);
		return prog.getBuildInfo<CL_PROGRAM_BUILD_LOG>(dev);
	}
}

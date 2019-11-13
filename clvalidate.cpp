#include<iostream>

#define CL_HPP_ENABLE_EXCEPTIONS
#include<CL/cl2.hpp>
#include<vector>
#include<unordered_map>
#include<string>
#include<fstream>
#include<sstream>
using namespace std;

static bool isnumber(const std::string& s,int& output)
{
	char* outptr;
	output=strtol(s.c_str(),&outptr,0);
	return *outptr;
}
static std::string tolowers(std::string s )
{
	for(int i=0;i<s.size();i++) s[i]=std::tolower(s[i]);
	return s;
}
static std::ostream& operator<<(std::ostream& out,const cl::Platform& plat)
{
	return out << plat.getInfo<CL_PLATFORM_NAME>() << ":" << plat.getInfo<CL_PLATFORM_VENDOR>() << " " << plat.getInfo<CL_PLATFORM_VERSION>();	
}
static std::ostream& operator<<(std::ostream& out,const cl::Device& dev)
{
	return out << dev.getInfo<CL_DEVICE_NAME>() << ":" << dev.getInfo<CL_DEVICE_VENDOR>() << " " << dev.getInfo<CL_DEVICE_VERSION>() << " " << dev.getInfo<CL_DEVICE_TYPE>();	
}
static void ReplaceStringInPlace(std::string& subject, const std::string& search,
                          const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
}
class OpenCL_Compiler
{
public:
	struct DeviceSelection
	{
		cl::Platform* plat;
		std::vector<cl::Device*> devices;
	};
	std::vector<cl::Platform> all_platforms;
	std::vector<std::vector<cl::Device>> all_devices;
	OpenCL_Compiler()
	{
		cl::Platform::get(&all_platforms);
		all_devices.resize(all_platforms.size());
		for(size_t i=0;i<all_platforms.size();i++)
		{
			all_platforms[i].getDevices(CL_DEVICE_TYPE_ALL,&all_devices[i]);
		}
	}
	void list_all_platforms()
	{
		for(size_t i=0;i<all_platforms.size();i++)
		{
			std::cout << "Platform [" << i << "]: " << all_platforms[i] << std::endl;
			std::vector<cl::Device>& devices=all_devices[i];
			for(size_t j=0;j<all_devices.size();j++)
			{
				std::cout  << "\tDevice [" << j << "]: " << all_devices[i][j] << std::endl;
			}
		}
	}
	DeviceSelection select_devices(const std::string& p,const std::vector<std::string>& ds)
	{
		int pdex=-1;
		if(!isnumber(p,pdex))
		{
			if(p=="")
			{
				pdex=0;
			}
			else
			{
				for(size_t i=0;i<all_platforms.size();i++)
				{
					std::string p3=tolowers(p.substr(3));
					std::string pl3=tolowers(std::string(all_platforms[i].getInfo<CL_PLATFORM_NAME>()).substr(3));
					if(p3==pl3)
					{
						pdex=i;
						continue;
					}
				}
			}
		}
		if(pdex >= all_platforms.size())
		{
			throw std::runtime_error("Selected platform index is out of range!");
		}
		else if(pdex < 0)
		{
			throw std::runtime_error("Selected platform was not found;");
		}
		std::vector<cl::Device*> devptrs;
		if(ds.size()==0)
		{
			for(size_t z=0;z<all_devices[pdex].size();z++)
			{
				devptrs.push_back(&all_devices[pdex][z]);
			}
		}
		for(size_t z=0;z<ds.size();z++)
		{
			int sdex=-1;
			if(!isnumber(ds[z],sdex))
			{
				for(size_t i=0;i<all_devices[pdex].size();i++)
				{
					std::string p3=tolowers(ds[z].substr(3));
					std::string pl3=tolowers(std::string(all_devices[pdex][i].getInfo<CL_DEVICE_NAME>()).substr(3));
					if(p3==pl3)
					{
						sdex=i;
						continue;
					}
				}
			}
			if(sdex >= all_devices[pdex].size())
			{
				throw std::runtime_error("Selected device index is out of range!");
			}
			else if(sdex < 0)
			{
				throw std::runtime_error("Selected device was not found;");
			}
			devptrs.push_back(&all_devices[pdex][sdex]);
		}
		return {&all_platforms[pdex],devptrs};
	}
};

class Options
{
public:
	bool compile_and_link;
	bool list_all;
	bool help_activated;
	std::string other_opts;
	std::string platform;
	std::vector<std::string> devices;
	std::vector<std::string> input_files;
	std::unordered_map<std::string,std::string> output_files;
	
	Options(const std::vector<std::string>& args):
		compile_and_link(true),
		list_all(false),
		help_activated(false),
		platform("")
	{
		if(args.size() == 1)
		{
			help(args[0]);
			return;
		}
		for(size_t i=1;i<args.size();i++)
		{
			std::string targ=args[i];
			if(targ[0] != '-')
			{
				throw std::runtime_error("Unrecognized option!");
			}
			else if(targ == "-i")
			{
				if(i+1 >= args.size() || args[i+1][0]=='-')
				{
					throw std::runtime_error("Option -i expects an input file");
				}
				else
				{
					input_files.push_back(args[++i]);
				}
			}
			else if(targ == "-p")
			{
				if(i+1 >= args.size() || args[i+1][0]=='-')
				{
					throw std::runtime_error("Option -p expects a platform");
				}
				else
				{
					platform=args[++i];
				}
			}
			else if(targ == "-d")
			{
				if(i+1 >= args.size() || args[i+1][0]=='-')
				{
					throw std::runtime_error("Option -d expects a device");
				}
				else
				{
					devices.push_back(args[++i]);
				}
			}
			else if(targ == "-oc")
			{
				if(i+1 >= args.size() || args[i+1][0]=='-')
				{
					throw std::runtime_error("Option -oc expects an output file");
				}
				else if(input_files.size()==0)
				{
					throw std::runtime_error("Option -oc requires an input file before each output file");
				}
				else
				{
					output_files[input_files.back()]=args[++i];
				}
			}
			else if(targ == "-list")
			{
				list_all=true;
			}
			else if(targ == "-c")
			{
				compile_and_link=false;
			}
			else if(targ == "-h" || targ == "--help")
			{
				help(args[0]);
				return;
			}
			else 
			{
				other_opts+=targ;
				other_opts+=' ';
			}
		}
		if(!list_all && input_files.empty())
		{
			throw std::runtime_error("No input files specified!");
		}
		if(platform=="" && devices.size())
		{
			throw std::runtime_error("-d Option requires -p option");
		}
	}
	void help(const std::string& arg0)
	{
		help_activated=true;
		std::cerr << "Usage: \n" << arg0 << " [-i <input_file> [-oc <output_file.c>]]+ [-c] [-p <Platform> [-d <Device>]*]  <ALL PASSTHROUGH CL COMPILER OPTIONS>"
		<< "\n\t" << arg0 << " -list" << std::endl;
	}
	void writeback(const std::string& shader,std::string output)
	{
		std::ofstream outfile(output.c_str());
		for(size_t ci=0;ci<output.size();ci++) {
			output[ci]=isalnum(output[ci]) ? output[ci] : '_';
		}
		while(output[0] == '_') output=std::string(output.begin()+1,output.end());
		output=std::string("_")+output;
		outfile << "const char " << output << "[]={";
		for(size_t i=0;i<shader.size();i++)
		{
			if((i & 0xF) == 0) outfile << "\n";
			outfile << (int)((char)shader[i]) << ",";
		}
		outfile << 0;
		outfile << "};\n" << std::endl;
	}
	int do_one_file(const cl::Context& ctx,const std::vector<cl::Device>& devs, const std::string& filein)
	{
		
		std::ostringstream filecontents;
		std::ifstream fo(filein.c_str());
		if(!fo) throw std::runtime_error(std::string("Error opening file: ")+filein);
		filecontents << fo.rdbuf();
		
		cl::Program prog(ctx,filecontents.str());
		int result;
		if(compile_and_link)
		{
			result=prog.build(devs,other_opts.c_str());
		}
		else
		{
			result=prog.compile(other_opts.c_str());
		}
		int allstatus=result;
		for(size_t di=0;di<devs.size();di++)
		{
			int32_t status;
			std::string buildLog=prog.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devs[di],&status);
			ReplaceStringInPlace(buildLog,"<kernel>",filein);
			allstatus|= (status != CL_BUILD_SUCCESS);
			std::cout << buildLog;
		}
		if(allstatus == CL_SUCCESS)
		{
			if(output_files.count(filein))
			{
				writeback(filecontents.str(),output_files[filein]);
			}
		}
		return allstatus;
	}
	int operator()()
	{
		OpenCL_Compiler comp;
		if(list_all) {
			comp.list_all_platforms();
			return 0;
		}
		int v=0;
		OpenCL_Compiler::DeviceSelection dsel=comp.select_devices(platform,devices);
		std::vector<cl::Device> devs;
		for(size_t di=0;di<dsel.devices.size();di++)
		{
			devs.push_back(*dsel.devices[di]);
		}
		cl::Context ctx(devs);
		for(size_t i=0;i<input_files.size();i++)
		{
			v|=do_one_file(ctx,devs,input_files[i]);
		}
		return v;
	}

};

int main(int argc,char** argv)
{
	std::vector<std::string> all_args(argv,argv+argc);
	try
	{
		Options opt(all_args);
		if(opt.help_activated) return 0;
		return opt();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}
}

#ifndef CL_CHOOSER_HPP
#define CL_CHOOSER_HPP

#include<CL/cl2.hpp>
#include<iostream>
#include<cstdlib>
#include<stdexcept>


class OpenCL_Chooser
{
private:
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
	static std::ostream& localPrint(std::ostream& out,const cl::Platform& plat)
	{
		return out << plat.getInfo<CL_PLATFORM_NAME>() << ":" << plat.getInfo<CL_PLATFORM_VENDOR>() << " " << plat.getInfo<CL_PLATFORM_VERSION>();	
	}
	static std::ostream& localPrint(std::ostream& out,const cl::Device& dev)
	{
		return out << dev.getInfo<CL_DEVICE_NAME>() << ":" << dev.getInfo<CL_DEVICE_VENDOR>() << " " << dev.getInfo<CL_DEVICE_VERSION>() << " " << dev.getInfo<CL_DEVICE_TYPE>();	
	}

public:
	struct SelectionResult
	{
	public:
		cl::Platform platform;
		std::vector<cl::Device> devices;
	};
	std::vector<cl::Platform> all_platforms;
	std::vector<std::vector<cl::Device>> all_devices;
	OpenCL_Chooser()
	{
		try
		{
			cl::Platform::get(&all_platforms);
		}
		catch(const cl::Error& err)
		{
			throw std::runtime_error("No OpenCL platforms found or there was an error attempting find any");
		}
		all_devices.resize(all_platforms.size());
		for(size_t i=0;i<all_platforms.size();i++)
		{
			all_platforms[i].getDevices(CL_DEVICE_TYPE_ALL,&all_devices[i]);
		}
	}
	void list_all_platforms(std::ostream& out=std::cout)
	{
		for(size_t i=0;i<all_platforms.size();i++)
		{
			out << "Platform [" << i << "]: "; 
			localPrint(out,all_platforms[i]) << std::endl;
			std::vector<cl::Device>& devices=all_devices[i];
			for(size_t j=0;j<all_devices.size();j++)
			{
				out  << "\tDevice [" << j << "]: ";
				localPrint(out,all_devices[i][j]) << std::endl;
			}
		}
	}
	SelectionResult select_devices(const std::string& p,const std::vector<std::string>& ds)
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
		std::vector<cl::Device> devices;
		if(ds.size()==0)
		{
			for(size_t z=0;z<all_devices[pdex].size();z++)
			{
				devices.push_back(all_devices[pdex][z]);
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
			devices.push_back(all_devices[pdex][sdex]);
		}
		
		return {all_platforms[pdex],devices};
	}
};

#endif

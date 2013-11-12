#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <iterator>

#include <CL/cl.hpp>

const std::string hw = "Hello World\n";
inline void
checkErr(cl_int err, const char * name)
{
    if (err != CL_SUCCESS) 
    {
        std::cerr << "ERROR: " << name
            << " (" << err << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
}

struct platformInfo
{
    platformInfo(cl::Platform &p)
    {
        p.getInfo((cl_platform_info)CL_PLATFORM_VENDOR, &vendor);
        p.getInfo((cl_platform_info)CL_PLATFORM_NAME, &name);
        p.getInfo((cl_platform_info)CL_PLATFORM_PROFILE, &profile);
        p.getInfo((cl_platform_info)CL_PLATFORM_VERSION, &version);
    }
    std::string humanReadable()
    {
        return name + " by " + vendor + "(profile: " +profile+", ver: " + version+ ")";
    }
    std::string name;
    std::string version;
    std::string profile;
    std::string vendor;
};

int main(int argc, char ** argv)
{
    cl_int err;
    std::vector<cl::Platform> platformList;
    cl::Platform::get(&platformList);
    checkErr((platformList.size() != 0) ? CL_SUCCESS: -1, "cl::Platform::get");
    std::cerr << platformList.size() << " platforms found" << std::endl;
    for(int i = 0; i < platformList.size(); i++)
    {
        platformInfo inf(platformList[i]);
        std::cerr << " " << i << ": " << inf.humanReadable() << std::endl;
    }
    int selectedi = 0;
    if(platformList.size() > 1)
    {
        // Let user choose device
        selectedi = -1;
        while(selectedi > (platformList.size() - 1) or selectedi < 0)
        {
            std::cout << "Select a one: ";
            std::cin >> selectedi;
        }
    }
    cl::Platform &selected = platformList[selectedi];
    platformInfo inf(platformList[selectedi]);
    std::cout << "Selected platform " << inf.humanReadable() << std::endl;

    cl_context_properties cprops[3] = {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)(selected)(),
        0
    };

    cl::Context ctxt(
            CL_DEVICE_TYPE_CPU,
            cprops,
            NULL,
            NULL,
            &err
            );
    checkErr(err, "creating context");

    char * outH = new char[hw.length() + 1];
    cl::Buffer outCL(
            ctxt,
            CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
            hw.length() + 1,
            outH,
            &err
            );
    checkErr(err, "Creating buffer");

    std::vector<cl::Device> devices;
    devices = ctxt.getInfo<CL_CONTEXT_DEVICES>();
    checkErr(devices.size() > 0 ? CL_SUCCESS : -1, "No devices found.");

    std::cout << "Found " << devices.size() << " devices" << std::endl;
    std::cout << "Loading program and preparing to compile" << std::endl;
    std::ifstream file("hello.cl");
    checkErr(file.is_open() ? CL_SUCCESS : -1, "Could not open source");
    std::string prog(
            std::istreambuf_iterator<char>(file), 
            (std::istreambuf_iterator<char>())
            );
    cl::Program::Sources source(1,
            std::make_pair(prog.c_str(), prog.length() + 1)
            );
    cl::Program program(ctxt, source);
    err = program.build(devices, "");
    checkErr(err, "Compiling hello.cl");

    cl::Kernel kernel(program, "hello", &err);
    checkErr(err, "Creating kernel 'hello'");

    err = kernel.setArg(0, outCL);
    checkErr(err, "setting Arguments");
    cl::CommandQueue queue(ctxt, devices[0], 0, &err);
    checkErr(err, "Setting up CommandQueue");
    cl::Event event;
    err = queue.enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(hw.length() + 1),
            cl::NDRange(1,1),
            NULL,
            &event
            );
    checkErr(err, "CommandQueue enqueueing");
    event.wait();
    err = queue.enqueueReadBuffer(outCL, 
            CL_TRUE,
            0,
            hw.length() + 1,
            outH);
    checkErr(err, "Reading buffer");
    std::cout << outH;
    return EXIT_SUCCESS;
}

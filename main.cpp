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
    // Have some variable to store the errorcodes in
    cl_int err;

    // // // PLATFORM STUFF // // //

    // Fetch the platformlist
    std::vector<cl::Platform> platformList;
    cl::Platform::get(&platformList);
    // And check if there's more than one.
    checkErr(
            (platformList.size() != 0) ? CL_SUCCESS: -1, 
            "No platforms found.");

    // List the found platforms
    std::cerr << platformList.size() << " platforms found" << std::endl;
    for(int i = 0; i < platformList.size(); i++)
    {
        platformInfo inf(platformList[i]);
        std::cerr << " " << i << ": " << inf.humanReadable() << std::endl;
    }

    // Let user choose the platform, we don't support
    // more than one yet
    int selectedi = 0;
    if(platformList.size() > 1)
    {
        selectedi = -1;
        while(selectedi > (platformList.size() - 1) or selectedi < 0)
        {
            std::cout << "Select a one: ";
            std::cin >> selectedi;
        }
    }
    // Get the selected platform (by reference, 'cause we can)
    cl::Platform &selected = platformList[selectedi];
    // Use our platformInfo struct to fetch the info!
    platformInfo inf(platformList[selectedi]);
    std::cout << "Selected platform " << inf.humanReadable() << std::endl;

    // // // CONTEXT STUFF // // //

    // Create the properties for
    cl_context_properties cprops[3] = {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)(selected)(),
        0
    };

    // Create the context
    cl::Context ctxt(
            CL_DEVICE_TYPE_CPU,
            cprops,
            NULL,
            NULL,
            &err
            );
    checkErr(err, "creating context");

    // Create the output buffers, this is what the OpenCL app
    // shall put it's results in
    
    // Buffer for the hello world kernel
    char * outH = new char[hw.length() + 1];
    cl::Buffer outCL(
            ctxt,
            CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
            hw.length() + 1,
            outH,
            &err
            );
    checkErr(err, "Creating buffer");

    // Buffers for the square kernel
    int _in = 11;
    int _out;
    cl::Buffer in(
            ctxt,
            CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
            sizeof(int),
            &_in,
            &err
            );
    checkErr(err, "Creating buffer");
    cl::Buffer out(
            ctxt,
            CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
            sizeof(int),
            &_out,
            &err
            );
    checkErr(err, "Creating buffer");

    // // // DEVICE STUFF // // //

    // Fetch all devices
    std::vector<cl::Device> devices;
    devices = ctxt.getInfo<CL_CONTEXT_DEVICES>();
    checkErr(devices.size() > 0 ? CL_SUCCESS : -1, "No devices found.");

    // Tell the user about our devices
    std::cout << "Found " << devices.size() << " devices" << std::endl;

    // // // LOAD AND COMPILE SOURCE // // //

    std::cout << "Loading program and preparing to compile" << std::endl;
    // Read the file
    std::ifstream file("hello.cl");
    checkErr(file.is_open() ? CL_SUCCESS : -1, "Could not open source");

    // Put the contents in a string
    std::string prog(
            std::istreambuf_iterator<char>(file), 
            (std::istreambuf_iterator<char>())
            );
    // Load the source
    cl::Program::Sources source(1,
            std::make_pair(prog.c_str(), prog.length() + 1)
            );
    // Apparently, we build the source for an std::vector of 
    // devices, so, for the platform.
    cl::Program program(ctxt, source);
    err = program.build(devices, "");
    checkErr(err, "Compiling hello.cl");


    // // // KERNELS // // //

    // Load the kernels
    cl::Kernel kernel(program, "hello", &err);
    checkErr(err, "Creating kernel 'hello'");

    cl::Kernel kernel_square(program, "square", &err);
    checkErr(err, "Creating kernel 'square'");

    // Add the output string to the kernel
    err = kernel.setArg(0, outCL);
    checkErr(err, "setting Arguments");
    
    // Add the input and output integers to the kernel
    err = kernel_square.setArg(0, in);
    err = kernel_square.setArg(1, out);
    checkErr(err, "setting Arguments");
    
    // Queue the commands!
    cl::CommandQueue queue(ctxt, devices[0], 0, &err);
    checkErr(err, "Setting up CommandQueue");

    // Setup the events that will be waited upon.
    cl::Event event1, event2;
    // Enqueue the work
    err = queue.enqueueNDRangeKernel(
            kernel, // the Helloworld kernel
            cl::NullRange, // 
            cl::NDRange(hw.length() + 1), //The amount of 'threads' workers
            cl::NDRange(1,1),
            NULL,
            &event1
            );
    err = queue.enqueueNDRangeKernel(
            kernel_square,
            cl::NullRange,
            cl::NDRange(1), // just one int
            cl::NDRange(1,1),
            NULL,
            &event2
            );
    checkErr(err, "CommandQueue enqueueing");

    // Wait for the processor to finish it's 
    // very difficult calculations
    event1.wait();
    event2.wait();

    // Read out the values
    err = queue.enqueueReadBuffer(out, 
            CL_TRUE,
            0,
            1, // just one int
            &_out);
    checkErr(err, "Reading buffer");
    err = queue.enqueueReadBuffer(outCL, 
            CL_TRUE,
            0,
            hw.length() + 1,
            outH);
    checkErr(err, "Reading buffer");

    // Print out our precious results
    std::cout << "Hello world kernel returns: " << outH
        << _in << "^2=" << _out << std::endl;
    return EXIT_SUCCESS;
}

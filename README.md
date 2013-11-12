OpenCL test files
=================

Welcome to my * OpenCL test sources. I hope you find them instructive and enjoyable.

Please do not hesitate to fork and to append some extra instructive material to it.

Prerequisites:
--------------
* An OpenCL implementation
* g++ compiler
* stdc++ libraries
* C++ knowlegde if you want to dig deeper
* opencl headers (yum install opencl-headers)
* icd makes a vender independent interface for opencl, highly recommended:
   yum install ocl-icd-devel ocl-icd

The OpenCL implementation:
Intel Core and Xeon:
http://software.intel.com/en-us/vcsource/tools/opencl-sdk-xe
Note: The site states only Xeon and Phi is supported; in the .tgz there is an rpm included
for Core series > 3th iteration. I know, I use it.

AMD: 
http://developer.amd.com/tools-and-sdks/heterogeneous-computing/amd-accelerated-parallel-processing-app-sdk/downloads/
Not yet tested, I've got some AMD cards, so some testing later on.

NVidia:
https://developer.nvidia.com/cuda-downloads
No NVidia card to test out, can't tell anything. Should work pretty good though.

Compiling
---------

Linux: Read the compile.sh script. It's a oneliner.
MacOS X: Not tested, though I think the linux script should work.
Windows: No idea, find it out. If you did, feel free to edit this readme.

Acknowledgements
================
*
I based this sources intensively on the following URLs; as I didn't find any licensing policy nor
other copyright statements, I will be trying to contact AMD for guidelines on the topic.
http://developer.amd.com/tools-and-sdks/heterogeneous-computing/amd-accelerated-parallel-processing-app-sdk/introductory-tutorial-to-opencl/

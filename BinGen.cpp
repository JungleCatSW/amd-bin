//
// Created by jungl on 12/12/2018.
//

#include "BinGen.hpp"

BinGen::BinGen() {
    cl_int status = 0;
    size_t deviceListSize;

    cl_uint numPlatforms;
  
    status = clGetPlatformIDs(0, NULL, &numPlatforms);
    if(status != CL_SUCCESS)
    {
        std::cout << "Error: Getting Platforms. (clGetPlatformsIDs)\n";
        return;
    }

    if(numPlatforms > 0)
    {
        cl_platform_id* platforms = new cl_platform_id[numPlatforms];
        status = clGetPlatformIDs(numPlatforms, platforms, NULL);
        if(status != CL_SUCCESS)
        {
            std::cout << "Error: Getting Platform Ids. (clGetPlatformsIDs)\n";
            return;
        }
        for(unsigned int i=0; i < numPlatforms; ++i)
        {
            char pbuff[100];
            status = clGetPlatformInfo(
                    platforms[i],
                    CL_PLATFORM_VENDOR,
                    sizeof(pbuff),
                    pbuff,
                    NULL);
            if(status != CL_SUCCESS)
            {
                std::cout << "Error: Getting Platform Info.(clGetPlatformInfo)\n";
                return;
            }
            m_platform = platforms[i];
            if(!strcmp(pbuff, "Advanced Micro Devices, Inc."))
            {
                break;
            }
        }
     //   delete platforms;
    }

    if(NULL == m_platform)
    {
		//TODO fixup
        std::cout << "NULL platform found so Exiting Application." << std::endl;
        return;
    }
}

void BinGen::createContext() {

	cl_int res;

	char exts[128];
	res = clGetPlatformInfo(m_platform, CL_PLATFORM_EXTENSIONS, 128, exts, 0);
	if (res != CL_SUCCESS)
		std::cout << "error" << res << std::endl;

	std::cout << "Platform extensions:" << exts << std::endl;


	cl_context_properties props[] = {
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)m_platform,
		CL_CONTEXT_OFFLINE_DEVICES_AMD,
		(cl_context_properties)1,
	0
	};
	
	cl_context m_context = clCreateContextFromType(props, CL_DEVICE_TYPE_ALL, NULL, NULL, &res);
	//cl_context m_context = createContext(&cprops, num_gpus, TempDeviceList, nullptr, nullptr, &ret);
	std::cout << CL_SUCCESS << "   " << res <<  std::endl;

}

bool BinGen::generateBinary(std::string kernel_path, std::string bin_name) {
	createContext();
	return false;

}

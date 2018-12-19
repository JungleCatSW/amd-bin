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

	cl_context_properties props[] = {
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)m_platform,
		CL_CONTEXT_OFFLINE_DEVICES_AMD,
		(cl_context_properties)1,
	0
	};
	
	m_context = clCreateContextFromType(props, CL_DEVICE_TYPE_ALL, NULL, NULL, &res);	

}

void BinGen::getDevices(){
    size_t deviceListSize;
    int res;
    res = clGetContextInfo(m_context,
                              CL_CONTEXT_DEVICES,
                              0,
                              NULL,
                              &deviceListSize);
    if(res != CL_SUCCESS)
        std::cout << "Error: Getting Context " << std::endl;

    m_devices;
    m_devices = (cl_device_id *)malloc(deviceListSize);

    if(m_devices == 0)
        std::cout << "Error: No devices found.\n";

    // Now, get the device list data
    res = clGetContextInfo(
            m_context,
            CL_CONTEXT_DEVICES,
            deviceListSize,
            m_devices,
            NULL);
    if(res != CL_SUCCESS)
        std::cout << "Error: Getting Context Info (device list, clGetContextInfo)\n" << std::endl;

	m_device = m_devices[0];

}

void BinGen::loadSourceCode() {
	const char *cryptonightCL =
#include "./opencl/cryptonight.cl"
		;
	const char *blake256CL =
#include "./opencl/blake256.cl"
		;
	const char *groestl256CL =
#include "./opencl/groestl256.cl"
		;
	const char *jhCL =
#include "./opencl/jh.cl"
		;
	const char *wolfAesCL =
#include "./opencl/wolf-aes.cl"
		;
	const char *wolfSkeinCL =
#include "./opencl/wolf-skein.cl"
		;
	const char *fastIntMathV2CL =
#include "./opencl/fast_int_math_v2.cl"
		;
	const char *fastDivHeavyCL =
#include "./opencl/fast_div_heavy.cl"
		;

	std::string source_code(cryptonightCL);
	m_source_code = source_code;
	m_source_code = std::regex_replace(m_source_code, std::regex("XMRIG_INCLUDE_WOLF_AES"), wolfAesCL);
	m_source_code = std::regex_replace(m_source_code, std::regex("XMRIG_INCLUDE_WOLF_SKEIN"), wolfSkeinCL);
	m_source_code = std::regex_replace(m_source_code, std::regex("XMRIG_INCLUDE_JH"), jhCL);
	m_source_code = std::regex_replace(m_source_code, std::regex("XMRIG_INCLUDE_BLAKE256"), blake256CL);
	m_source_code = std::regex_replace(m_source_code, std::regex("XMRIG_INCLUDE_GROESTL256"), groestl256CL);
	m_source_code = std::regex_replace(m_source_code, std::regex("XMRIG_INCLUDE_FAST_INT_MATH_V2"), fastIntMathV2CL);
	m_source_code = std::regex_replace(m_source_code, std::regex("XMRIG_INCLUDE_FAST_DIV_HEAVY"), fastDivHeavyCL);
}

void BinGen::buildProgram() {
	size_t deviceListSize;
	int res;
	res = clGetContextInfo(m_context,
		CL_CONTEXT_DEVICES,
		0,
		NULL,
		&deviceListSize);
	if (res != CL_SUCCESS)
		std::cout << "Error: Getting Context " << std::endl;

	m_devices;
	m_devices = (cl_device_id *)malloc(deviceListSize);

	if (m_devices == 0)
		std::cout << "Error: No devices found.\n";

	// Now, get the device list data
	res = clGetContextInfo(
		m_context,
		CL_CONTEXT_DEVICES,
		deviceListSize,
		m_devices,
		NULL);
	if (res != CL_SUCCESS)
		std::cout << "Error: Getting Context Info (device list, clGetContextInfo)\n" << std::endl;

	//m_device = m_devices[0];

	char options[512] = { 0 };
	snprintf(options, sizeof(options), "-DITERATIONS=%u -DMASK=%u -DWORKSIZE=%zu -DSTRIDED_INDEX=%d -DMEM_CHUNK_EXPONENT=%d -DCOMP_MODE=%d -DMEMORY=%zu "
									   "-DALGO=%d -DUNROLL_FACTOR=%d -DOPENCL_DRIVER_MAJOR=%d",
			 0x80000, //iter
			 0x1FFFF0,
			 8,
			 2,
			 static_cast<int>(1u << m_ctx->memChunk),
			 m_ctx->compMode,
			 xmrig::cn_select_memory(algo),
			 static_cast<int>(algo),
			 m_ctx->unrollFactor,
			 amdDriverMajorVersion()
	);

	//cl_int res;
	const char* src = m_source_code.c_str();
	cl_program m_program = clCreateProgramWithSource(m_context, 1, reinterpret_cast<const char**>(&src), nullptr, &res);
	if (res != CL_SUCCESS)
		std::cout << "error" << res << std::endl;

	res = clBuildProgram(m_program, 1, m_devices, NULL, NULL, NULL);
	if(res !=CL_SUCCESS)
		std::cout << "error" << res << std::endl;

}

bool BinGen::generateBinary(std::string kernel_path, std::string bin_name) {
	loadSourceCode();
	createContext();
	getDevices();
	buildProgram();
	return false;

}

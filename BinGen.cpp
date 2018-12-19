//
// Created by jungl on 12/12/2018.
//

#include "BinGen.hpp"

BinGen::BinGen() {
    cl_int status = 0;
    size_t deviceListSize;

    cl_uint numPlatforms;

    status = clGetPlatformIDs(0, NULL, &numPlatforms);
    if (status != CL_SUCCESS) {
        std::cout << "Error: Getting Platforms. (clGetPlatformsIDs)\n";
        return;
    }

    if (numPlatforms > 0) {
        cl_platform_id *platforms = new cl_platform_id[numPlatforms];
        status = clGetPlatformIDs(numPlatforms, platforms, NULL);
        if (status != CL_SUCCESS) {
            std::cout << "Error: Getting Platform Ids. (clGetPlatformsIDs)\n";
            return;
        }
        for (unsigned int i = 0; i < numPlatforms; ++i) {
            char pbuff[100];
            status = clGetPlatformInfo(
                    platforms[i],
                    CL_PLATFORM_VENDOR,
                    sizeof(pbuff),
                    pbuff,
                    NULL);
            if (status != CL_SUCCESS) {
                std::cout << "Error: Getting Platform Info.(clGetPlatformInfo)\n";
                return;
            }
            m_platform = platforms[i];
            if (!strcmp(pbuff, "Advanced Micro Devices, Inc.")) {
                break;
            }
        }
        //   delete platforms;
    }

    if (NULL == m_platform) {
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
            (cl_context_properties) m_platform,
            CL_CONTEXT_OFFLINE_DEVICES_AMD,
            (cl_context_properties) 1,
            0
    };

    m_context = clCreateContextFromType(props, CL_DEVICE_TYPE_ALL, NULL, NULL, &res);

}

void BinGen::getDevices() {
    size_t deviceListSize;
    int res;
    res = clGetContextInfo(m_context,
                           CL_CONTEXT_DEVICES,
                           0,
                           NULL,
                           &deviceListSize);
    if (res != CL_SUCCESS)
        std::cout << "Error: Getting Context " << std::endl;
    m_device_list_size = deviceListSize;

    m_devices = (cl_device_id *) malloc(deviceListSize);

    cl_device_id devices[deviceListSize];

    if (m_devices == 0)
        std::cout << "Error: No devices found.\n";

    // Now, get the device list data
    res = clGetContextInfo(
            m_context,
            CL_CONTEXT_DEVICES,
            deviceListSize,
            &devices,
            NULL);
    if (res != CL_SUCCESS)
        std::cout << "Error: Getting Context Info (device list, clGetContextInfo)\n" << std::endl;


    /*Print these badboys*/
    std::cout << "Found the following virtual devices" << std::endl;
    int validDevices = 0;
    try {
        for (unsigned int i = 0; i < deviceListSize; i++) {
            char name[128] = {0};
            res = clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(name), &name, NULL);
            if (res == CL_SUCCESS) {
                std::cout << name << std::endl;
                validDevices++;
            }
        }
    }catch(std::exception &e){
        std::cout << e.what() << std::endl;
    }
    std::cout << "Found " << validDevices << "/" << deviceListSize << " valid devices" << std::endl;
    m_device = m_devices[0];

}

void BinGen::loadSourceCode() {
    //@formatter:off
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
    //@formatter:on
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
    m_devices = (cl_device_id *) malloc(deviceListSize);

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

    char options[512] = {0};

    snprintf(options, sizeof(options),
             "-DITERATIONS=524288 -DMASK=2097136 -DWORKSIZE=8 -DSTRIDED_INDEX=2 -DMEM_CHUNK_EXPONENT=4 -DCOMP_MODE=1 -DMEMORY=2097152 -DALGO=0 -DUNROLL_FACTOR=8");

    //cl_int res;
    const char *src = m_source_code.c_str();
    m_program = clCreateProgramWithSource(m_context, 1, reinterpret_cast<const char **>(&src), nullptr, &res);
    if (res != CL_SUCCESS)
        std::cout << "error" << res << std::endl;


    res = clBuildProgram(m_program, 1, m_devices, options, NULL, NULL);
    if (res != CL_SUCCESS)
        std::cout << "error" << res << std::endl;

    size_t len = 0;
    res = clGetProgramBuildInfo(m_program, m_devices[0], CL_PROGRAM_BUILD_LOG, 0, nullptr, &len);


    int status = 10;
    res = clGetProgramBuildInfo(m_program, m_devices[0], CL_PROGRAM_BUILD_STATUS, sizeof(int), &status, NULL);
    //CL_BUILD_SUCCESS
    //res == CL_BUILD_SUCCESS

}

void BinGen::programInfo() {
//	int len = 0;
//	int res = clgetProgramBuildInfo(m_program, device, , CL_PROGRAM_BUILD_LOG, 0, nullptr, &len);
//	clGetProgramBuildInfo ( 	cl_program  program,
//			cl_device_id  device,
//			cl_program_build_info  param_name,
//			size_t  param_value_size,
//			void  *param_value,
//			size_t  *param_value_size_ret)
//	char *buildLog = new char[len + 1]();
//
//	if (OclLib::getProgramBuildInfo(m_ctx->Program, m_ctx->DeviceID, CL_PROGRAM_BUILD_LOG, len, buildLog, nullptr) != CL_SUCCESS) {
//		delete [] buildLog;
//		return false;
//	}
}

void BinGen::save() {
//	int num_devices
//	std::vector<size_t> binary_sizes(num_devices);
//	OclLib::getProgramInfo(m_ctx->Program, CL_PROGRAM_BINARY_SIZES, sizeof(size_t) * binary_sizes.size(), binary_sizes.data());
//
//	std::vector<char*> all_programs(num_devices);
//	std::vector<std::vector<char>> program_storage;
//
//	size_t mem_size = 0;
//	for (size_t i = 0; i < all_programs.size(); ++i) {
//		program_storage.emplace_back(std::vector<char>(binary_sizes[i]));
//		all_programs[i] = program_storage[i].data();
//		mem_size += binary_sizes[i];
//	}
//
//	if (OclLib::getProgramInfo(m_ctx->Program, CL_PROGRAM_BINARIES, num_devices * sizeof(char*), all_programs.data()) != CL_SUCCESS) {
//		return false;
//	}
//
//	std::ofstream file_stream;
//	file_stream.open(m_fileName, std::ofstream::out | std::ofstream::binary);
//	file_stream.write(all_programs[dev_id], binary_sizes[dev_id]);
//	file_stream.close();
}

bool BinGen::generateBinary(std::string kernel_path, std::string bin_name) {
    loadSourceCode();
    createContext();
    getDevices();
    buildProgram();
    return false;

}

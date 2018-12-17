//
// Created by jungl on 12/12/2018.
//

#ifndef AMD_BIN_BINGEN_HPP
#define AMD_BIN_BINGEN_HPP

#include <CL/cl_ext.h>
#include <iostream>
#include <string>

class BinGen {

public:
    BinGen();
    bool generateBinary(std::string kernel_path, std::string bin_name);

private:
	void createContext();
	void buildProgram();
	cl_platform_id m_platform = NULL;
};


#endif //AMD_BIN_BINGEN_HPP

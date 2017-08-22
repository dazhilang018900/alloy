/*
 * Copyright(C) 2016, Blake C. Lucas, Ph.D. (img.science@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "ocl/ocl_runtime_error.h"
#include "ocl/ComputeCL.h"
#include "ocl/ProgramCL.h"
#include <CL/cl_gl.h>
#include <AlloyCommon.h>
#include <AlloyFileUtil.h>
#include <AlloyContext.h>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <memory>
#include <array>
namespace aly {
std::shared_ptr<ComputeCL> ComputeCL::computeInstance;
std::shared_ptr<ComputeCL> ComputeCL::Instance() {
	if (computeInstance.get() == nullptr) {
		computeInstance.reset(new ComputeCL());
	}
	return computeInstance;
}
void ComputeCL::flush() const {
	clFlush(getCommandQueue());
}

int ComputeCL::finish() const {
	int err = clFinish(getCommandQueue());
	PrintCLError(err);
	return err;
}
void ComputeCL::Flush() {
	clFlush(Instance()->getCommandQueue());
}

int ComputeCL::Finish() {
	int err = clFinish(Instance()->getCommandQueue());
	PrintCLError(err);
	return err;
}
void ComputeCL::enumeratePlatforms() {
	cl_uint numPlatforms = 0;
	platforms.clear();
	int ret = clGetPlatformIDs(NULL, NULL, &numPlatforms);
	if (ret == CL_SUCCESS) {
		std::vector<cl_platform_id> pids(numPlatforms);
		ret = clGetPlatformIDs(numPlatforms, pids.data(), &numPlatforms);
		if (ret == CL_SUCCESS) {
			for (unsigned int i = 0; i < numPlatforms; i++) {
				platforms.push_back(pids[i]);
			}
		}
	}
}
std::vector<cl_device_id> ComputeCL::enumerateDevices(cl_platform_id id) const {
	cl_uint numDevices = 0;
	std::vector<cl_device_id> devices;
	int ret = clGetDeviceIDs(id, CL_DEVICE_TYPE_ALL, NULL, NULL, &numDevices);
	if (ret == CL_SUCCESS) {
		std::vector<cl_device_id> pids(numDevices);
		ret = clGetDeviceIDs(id, CL_DEVICE_TYPE_ALL, numDevices, pids.data(),
				&numDevices);
		if (ret == CL_SUCCESS) {
			for (unsigned int i = 0; i < numDevices; i++) {
				devices.push_back(pids[i]);
			}
		}
	}
	return devices;
}
bool ComputeCL::hasExtension(cl_device_id id, const std::string& name) const {
	const int MAX_LENGTH = 4096;
	std::array<char, MAX_LENGTH> data;
	clGetDeviceInfo(id, CL_DEVICE_EXTENSIONS, MAX_LENGTH, (void*) data.data(),
			NULL);
	std::string strExtension(data.data());
	return (Contains(strExtension, name));
}
bool ComputeCL::isVendor(cl_platform_id id, const std::string& vendor) const {
	const int MAX_LENGTH = 512;
	std::array<char, MAX_LENGTH> name;
	name.fill(0);
	clGetPlatformInfo(id, CL_PLATFORM_VENDOR, MAX_LENGTH, (void*) name.data(),
			NULL);
	return (ToLower(std::string(name.data())) == ToLower(vendor));
}
bool ComputeCL::isVendor(cl_device_id id, const std::string& vendor) const {
	const int MAX_LENGTH = 512;
	std::array<char, MAX_LENGTH> name;
	name.fill(0);
	clGetDeviceInfo(id, CL_DEVICE_VENDOR, MAX_LENGTH, (void*) name.data(),
			NULL);
	return (ToLower(std::string(name.data())) == ToLower(vendor));
}
bool ComputeCL::lockGL(std::vector<cl_mem>& memobjs) {
	glFinish();
	if (memobjs.size() > 0) {
		int err = clEnqueueAcquireGLObjects(commandQueue,
				static_cast<cl_uint>(memobjs.size()), &memobjs[0], 0, NULL,
				NULL);
		PrintCLError(err);
		return (CL_SUCCESS == err);
	} else {
		return true;
	}
}
bool ComputeCL::unlockGL(std::vector<cl_mem>& memobjs) {
	if (memobjs.size() > 0) {
		int err = clEnqueueReleaseGLObjects(commandQueue,
				static_cast<cl_uint>(memobjs.size()), &memobjs[0], 0, NULL,
				NULL);
		PrintCLError(err);
		return (CL_SUCCESS == err);
	} else {
		return true;
	}
}
bool ComputeCL::lockGL(std::initializer_list<cl_mem>& memobjs) {
	std::vector<cl_mem> tmp(memobjs.begin(), memobjs.end());
	return lockGL(tmp);
}
bool ComputeCL::unlockGL(std::initializer_list<cl_mem>& memobjs) {
	std::vector<cl_mem> tmp(memobjs.begin(), memobjs.end());
	return unlockGL(tmp);
}
ComputeCL::ComputeCL() :
		platformId(0), deviceId(0), context(0), commandQueue(0), initialized(
				false), deviceType(Device::ALL) {
	//glewInit(); Don't call twice!
	CHECK_GL_ERROR();
	buildOptions = "-cl-mad-enable -cl-std=CL1.1 -cl-kernel-arg-info";
}
ComputeCL::~ComputeCL() {
	if (commandQueue != nullptr) {
		clReleaseCommandQueue(commandQueue);
		commandQueue = nullptr;
	}
	if (context != nullptr) {
		clReleaseContext(context);
		context = nullptr;
	}
}

std::string ComputeCL::getDeviceInfo(const cl_device_id& id) const {
	std::string strTmp;
	const int MAX_LENGTH = 4096;
	std::array<char, MAX_LENGTH> data;
	data.fill(0);
	clGetDeviceInfo(id, CL_DEVICE_NAME, MAX_LENGTH, (void*) data.data(), NULL);
	std::string name(data.data());
	clGetDeviceInfo(id, CL_DEVICE_VENDOR, MAX_LENGTH, (void*) data.data(),
			NULL);
	std::string vendor(data.data());
	clGetDeviceInfo(id, CL_DEVICE_VERSION, MAX_LENGTH, (void*) data.data(),
			NULL);
	std::string version(data.data());
	Device type;
	clGetDeviceInfo(id, CL_DEVICE_TYPE, MAX_LENGTH, (void*) &type, NULL);
	clGetDeviceInfo(id, CL_DEVICE_EXTENSIONS, MAX_LENGTH, (void*) data.data(),
			NULL);
	std::string extension(data.data());
	return aly::MakeString() << "Device: " << vendor << ":" << name << " ("
			<< type << ") " << version << "\nExtensions=" << extension;
}
std::shared_ptr<ProgramCL> CLCompile(const std::string& name,
		const std::vector<std::string>& fileNames,
		const std::vector<ConstantCL>& constants) {
	return CLInstance()->compileFiles(name, fileNames, constants);
}
std::shared_ptr<ProgramCL> CLCompile(const std::string& name,
		const std::initializer_list<std::string>& fileNames,
		const std::vector<ConstantCL>& constants) {
	std::vector<std::string> tmp(fileNames.begin(), fileNames.end());
	return CLInstance()->compileFiles(name, tmp, constants);
}
std::shared_ptr<ProgramCL> CLCompile(const std::string& fileName,
		const std::initializer_list<ConstantCL>& constants) {
	std::vector<ConstantCL> tmp;
	tmp.assign(constants.begin(), constants.end());
	return CLCompile(fileName, tmp);
}
std::shared_ptr<ProgramCL> CLCompile(const std::string& fileName,
		const std::vector<ConstantCL>& constants) {
	std::vector<std::string> tmp = { fileName };
	return CLInstance()->compileFiles(GetFileNameWithoutExtension(fileName),
			tmp, constants);
}
void CLInitialize(const std::string& vendor, const ComputeCL::Device& device) {
	if(!ComputeCL::Instance()->isInitialized()){
		ComputeCL::Instance()->initialize(vendor, device);
	}
}
void CLDestroy() {
	ComputeCL::Destroy();
}
void ComputeCL::Destroy() {
	computeInstance.reset();
}
cl_context CLContext() {
	if (!ComputeCL::Instance()->isInitialized()) {
		throw ocl_runtime_error(
				"CL context referenced before being initialized.");
	}
	return ComputeCL::Instance()->getContext();
}
std::string ConstantCL::toCode() const {
	return MakeString() << "#define " << name << " (" << value.toString()
			<< ")\n";
}
std::string ConstantCL::toString() const {
	return MakeString() << name << "=" << value.toString();
}
std::string ComputeCL::getPlatformInfo(const cl_platform_id& id) const {
	std::string strTmp;
	const int MAX_LENGTH = 4096;
	std::array<char, MAX_LENGTH> data;
	data.fill(0);
	clGetPlatformInfo(id, CL_PLATFORM_NAME, MAX_LENGTH, (void*) data.data(),
			NULL);
	std::string name(data.data());
	clGetPlatformInfo(id, CL_PLATFORM_VENDOR, MAX_LENGTH, (void*) data.data(),
			NULL);
	std::string vendor(data.data());
	clGetPlatformInfo(id, CL_PLATFORM_VERSION, MAX_LENGTH, (void*) data.data(),
			NULL);
	std::string version(data.data());
	clGetPlatformInfo(id, CL_PLATFORM_EXTENSIONS, MAX_LENGTH,
			(void*) data.data(), NULL);
	std::string extension(data.data());
	return aly::MakeString() << "Platform: " << vendor << ":" << name << ":"
			<< version << "\nExtensions=" << extension;
}

std::string ComputeCL::getPlatformInfo() const {
	std::stringstream ss;
	for (int i = 0; i < (int) platforms.size(); ++i) {
		std::string strInfo;
		ss << getPlatformInfo(platforms[i]) << std::endl;
		std::vector<cl_device_id> devices = enumerateDevices(platforms[i]);
		for (cl_device_id id : devices) {
			ss << getDeviceInfo(id) << std::endl;
		}
	}
	return ss.str();
}
void ComputeCL::setDevice(const Device& cd, const std::string& vendor) {
	int ret = -1;
	cl_device_type devicetype = CL_DEVICE_TYPE_CPU;
	if (cd == Device::GPU)
		devicetype = CL_DEVICE_TYPE_GPU;
	else if (cd == Device::CPU)
		devicetype = CL_DEVICE_TYPE_CPU;
	else if (cd == Device::ALL)
		devicetype = CL_DEVICE_TYPE_ALL;
	for (int i = 0; i < (int) platforms.size(); ++i) {
		cl_device_id deviceid = 0;
		cl_uint iNumDevices = 0;
		ret = clGetDeviceIDs(platforms[i], devicetype, 1, &deviceid,
				&iNumDevices);
		if (ret == CL_SUCCESS && 0 != deviceid
				&& (vendor == "" || isVendor(deviceid, vendor))) {
			platformId = platforms[i];
			deviceId = deviceid;
			break;
		}
	}
	if (ret != CL_SUCCESS || 0 == deviceId) {
		throw ocl_runtime_error("Could not find CL device for vendor " + vendor,
				ret);
	}
}
void ComputeCL::initialize(const std::string& vendor,
		const ComputeCL::Device& device) {
	initialized = false;
	enumeratePlatforms();
	int err = CL_SUCCESS;
	if (platforms.size() != 0) {
		std::cout << getPlatformInfo();
		bool foundDevice = false;
		try {
			setDevice(deviceType = device, vendor);
			foundDevice = true;
		} catch (const ocl_runtime_error&) {
			std::cerr << "Could not find GPU device for vendor " << vendor
					<< "." << std::endl;
		}
		//GPU with vendor name not found, try to find any GPU
		if (!foundDevice) {
			try {
				setDevice(deviceType = device);
				foundDevice = true;
			} catch (const ocl_runtime_error&) {
				std::cerr << "Could not find GPU device." << std::endl;
			}
		}
		if (!foundDevice) {
			throw ocl_runtime_error(
					aly::MakeString()
							<< "Could not initialize OpenCL with vendor "
							<< vendor << " and device " << device);
		}
		std::string deviceInfo = getDeviceInfo(deviceId);
		if (deviceType == Device::GPU
				&& hasExtension(deviceId, "cl_khr_gl_sharing")) {
			std::cout << "Initializing GL" << std::endl;
			cl_context_properties properties[] =
					{
#ifdef ALY_WINDOWS
							CL_GL_CONTEXT_KHR, (cl_context_properties) wglGetCurrentContext(),
							CL_WGL_HDC_KHR, (cl_context_properties) wglGetCurrentDC(),
#else
							CL_GL_CONTEXT_KHR,
							(cl_context_properties) glXGetCurrentContext(), // GLX Context
							CL_GLX_DISPLAY_KHR,
							(cl_context_properties) glXGetCurrentDisplay(), // GLX Display
#endif
							CL_CONTEXT_PLATFORM,
							(cl_context_properties) platformId, 0 };

			context = clCreateContext(properties, 1, &deviceId, NULL, NULL,
					&err);
		} else {
			cl_context_properties properties[] = {
			CL_CONTEXT_PLATFORM, (cl_context_properties) platformId, 0 };
			context = clCreateContext(properties, 1, &deviceId, NULL, NULL,
					&err);
		}
		if (err != CL_SUCCESS)
			throw ocl_runtime_error("Could not create context.", err);
		commandQueue = clCreateCommandQueue(context, deviceId, 0, &err);
		if (err != CL_SUCCESS)
			throw ocl_runtime_error("Could not create command queue.", err);
	} else {
		throw ocl_runtime_error("No OpenCL platforms found.");
	}
	initialized = true;
}
std::string ComputeCL::getBuildLog(cl_program program) const {
	size_t len = 0;
	int err = clGetProgramBuildInfo(program, deviceId, CL_PROGRAM_BUILD_LOG, 0,
			NULL, &len);
	if (err == CL_SUCCESS) {
		if (len != 0) {
			std::vector<char> logInfo(len + 1);
			err = clGetProgramBuildInfo(program, deviceId, CL_PROGRAM_BUILD_LOG,
					len + 1, logInfo.data(), &len);
			PrintCLError(err);
			return std::string(logInfo.begin(), logInfo.end());
		}
	}
	return "";
}
std::shared_ptr<ProgramCL> ComputeCL::compileSources(
		const std::vector<std::string>& srcCodes,
		const std::vector<ConstantCL>& constants) {
	std::vector<const char*> ptrs;
	std::string constantStr;
	if (constants.size() > 0) {
		ptrs.resize(srcCodes.size() + 1);
		std::stringstream ss;
		for (ConstantCL c : constants) {
			ss << c.toCode();
		}
		constantStr = ss.str();
		ptrs[0] = constantStr.c_str();
		for (int k = 0; k < (int) srcCodes.size(); k++) {
			ptrs[k + 1] = srcCodes[k].c_str();
		}
	} else {
		ptrs.resize(srcCodes.size());
		for (int k = 0; k < (int) srcCodes.size(); k++) {
			ptrs[k] = srcCodes[k].c_str();
		}
	}
	int err = -1;
	cl_program program = clCreateProgramWithSource(context,
			static_cast<cl_uint>(ptrs.size()), (const char**) ptrs.data(),
			nullptr, &err);
	if (err != CL_SUCCESS)
		throw ocl_runtime_error("Could not create OpenCL programs.", err);
	if (program != nullptr) {
		err = clBuildProgram(program, 1, &deviceId, buildOptions.c_str(), NULL,
				NULL);
		if (err != CL_SUCCESS) {
			std::cerr << "Source Code [" << srcCodes[0] << "]" << std::endl;
			std::cerr << "Build Log:\n" << getBuildLog(program) << std::endl;
			std::cerr.flush();
			clReleaseProgram(program);
			program = 0;
			throw ocl_runtime_error(
					"Could not compile OpenCL program from source.", err);
		}
	}
	return std::shared_ptr<ProgramCL>(new ProgramCL(program));
}
std::shared_ptr<ProgramCL> ComputeCL::compileSource(const std::string& srcCode,
		const std::vector<ConstantCL>& constants) {
	std::string srcStr;
	if (constants.size() > 0) {
		std::stringstream ss;
		for (ConstantCL c : constants) {
			ss << c.toCode();
		}
		srcStr = ss.str() + srcCode;
	} else {
		srcStr = srcCode;
	}
	const char* ptrs[1];
	ptrs[0] = srcStr.c_str();
	int err = -1;
	cl_program program = clCreateProgramWithSource(context, 1,
			(const char**) &ptrs[0], nullptr, &err);
	if (err != CL_SUCCESS)
		throw ocl_runtime_error("Could not create OpenCL programs.", err);
	if (program != nullptr) {
		err = clBuildProgram(program, 1, &deviceId, buildOptions.c_str(), NULL,
				NULL);
		if (err != CL_SUCCESS) {
			std::cerr << "Source Code [" << srcCode << "]" << std::endl;
			std::cerr << "Build Log:\n" << getBuildLog(program) << std::endl;
			std::cerr.flush();
			clReleaseProgram(program);
			program = 0;
			throw ocl_runtime_error(
					"Could not compile OpenCL program from source.", err);
		}
	}
	return std::shared_ptr<ProgramCL>(new ProgramCL(program));
}
std::shared_ptr<ProgramCL> ComputeCL::compileFiles(
		const std::vector<std::string>& fileNames,
		const std::vector<ConstantCL>& constants) {
	std::vector<std::string> srcCodes;
	std::vector<const char*> ptrs;
	std::string constantStr;
	if (constants.size() > 0) {
		ptrs.resize(fileNames.size() + 1);
		srcCodes.resize(fileNames.size() + 1);
		std::stringstream ss;
		for (ConstantCL c : constants) {
			ss << c.toCode();
		}
		constantStr = ss.str();
		srcCodes[0] = constantStr;
		ptrs[0] = srcCodes[0].c_str();
		for (int k = 0; k < (int) fileNames.size(); k++) {
			std::string textFile = ReadTextFile(
					AlloyDefaultContext()->getFullPath(fileNames[k]));
			srcCodes[k + 1] = textFile;
			ptrs[k + 1] = srcCodes[k+1].c_str();
		}
	} else {
		ptrs.resize(fileNames.size());
		srcCodes.resize(fileNames.size());
		for (int k = 0; k < (int) fileNames.size(); k++) {
			std::string textFile = ReadTextFile(AlloyDefaultContext()->getFullPath(fileNames[k]));
			srcCodes[k] = textFile;
			ptrs[k] = srcCodes[k].c_str();
		}
	}
	int err = -1;
	cl_program program = clCreateProgramWithSource(context,static_cast<cl_uint>(ptrs.size()), (const char**) ptrs.data(),nullptr, &err);
	if (err != CL_SUCCESS)
		throw ocl_runtime_error("Could not create OpenCL programs.", err);
	if (program != nullptr) {
		err = clBuildProgram(program, 1, &deviceId, buildOptions.c_str(), NULL,
				NULL);
		if (err != CL_SUCCESS) {
			std::cerr << "Source Code [" << srcCodes[0] << "]" << std::endl;
			std::cerr << "Build Log:\n" << getBuildLog(program) << std::endl;
			std::cerr.flush();
			clReleaseProgram(program);
			program = 0;
			throw ocl_runtime_error(
					"Could not compile OpenCL program from source.", err);
		}
	}
	return std::shared_ptr<ProgramCL>(new ProgramCL(program));
}
std::shared_ptr<ProgramCL> ComputeCL::compileFile(const std::string& fileName,
		const std::vector<ConstantCL>& constants) {
	std::string constantStr;
	if (constants.size() > 0) {
		std::stringstream ss;
		for (ConstantCL c : constants) {
			ss << c.toCode();
		}
		constantStr = ss.str();
	}
	std::string srcCodes[1];
	const char* ptrs[1];
	std::string textFile = constantStr+ ReadTextFile(AlloyDefaultContext()->getFullPath(fileName));
	if(textFile.size()==0)throw ocl_runtime_error("Could not read OpenCL program.");
	srcCodes[0] = textFile;
	ptrs[0] = srcCodes[0].c_str();
	int err = -1;
	cl_program program = clCreateProgramWithSource(context, 1,
			(const char**) &ptrs[0], nullptr, &err);
	if (err != CL_SUCCESS)
		throw ocl_runtime_error("Could not create OpenCL programs.", err);
	if (program != nullptr) {
		err = clBuildProgram(program, 1, &deviceId, buildOptions.c_str(), NULL,
				NULL);
		if (err != CL_SUCCESS) {
			std::cerr << "Source Code [" << srcCodes[0] << "]" << std::endl;
			std::cerr << "Build Log:\n" << getBuildLog(program) << std::endl;
			std::cerr.flush();
			clReleaseProgram(program);
			program = 0;
			throw ocl_runtime_error(
					"Could not compile OpenCL program from source.", err);
		}
	}
	return std::shared_ptr<ProgramCL>(new ProgramCL(program));
}
std::vector<std::string> ComputeCL::computeHashCodes(
		const std::vector<std::string>& fileNames) const {
	std::vector<std::string> hashCode(fileNames.size());
	for (int k = 0; k < (int) fileNames.size(); k++) {
		std::string file = AlloyDefaultContext()->getFullPath(fileNames[k]);
		std::string txtFile = ReadTextFile(file);
		std::vector<char> hashData(txtFile.begin(), txtFile.end());
		hashCode[k] = HashCode(hashData, HashMethod::SHA256);
	}
	return hashCode;
}
std::shared_ptr<ProgramCL> ComputeCL::compileFiles(const std::string& name,
		const std::vector<std::string>& fileNames,
		const std::vector<ConstantCL>& constants) {
	int err = CL_SUCCESS;
	cl_program program = nullptr;
	std::stringstream ss;
	for (ConstantCL c : constants) {
		ss << c.toCode();
	}
	for (int k = 0; k < (int) fileNames.size(); k++) {
		std::string file = AlloyDefaultContext()->getFullPath(fileNames[k]);
		std::string txtFile = ReadTextFile(file);
		ss << txtFile;
	}
	std::string hashString = ss.str();
	if(hashString.size()==0)throw ocl_runtime_error("Could not read OpenCL program.");
	std::vector<char> hashData(hashString.begin(), hashString.end());
	std::string hashCode = aly::HashCode(hashData, HashMethod::SHA256);
	std::string binaryName = aly::MakeString() << name << "."
			<< hashCode.substr(0, 16) + ".bin";
	std::string binaryPath = GetCurrentWorkingDirectory() + ALY_PATH_SEPARATOR+binaryName;
	if (FileExists(binaryPath)) {
		std::vector<char> fileData = ReadBinaryFile(binaryPath);
		size_t dataLength[1];
		char* ptrs[1];
		ptrs[0] = fileData.data();
		if (fileData.size() > 0) {
			dataLength[0] = fileData.size();
			program = clCreateProgramWithBinary(context, 1, &deviceId,(const size_t*) &dataLength,
					(const unsigned char**) &ptrs[0], NULL, &err);
			if (err != CL_SUCCESS)
				throw ocl_runtime_error(
						"Could not load pre-compiled OpenCL program.", err);
			if (program != nullptr) {
				err = clBuildProgram(program, 1, &deviceId,
						buildOptions.c_str(), NULL, NULL);
				if (err != CL_SUCCESS) {
					std::cerr << "Build Log:\n" << getBuildLog(program)
							<< std::endl;
					std::cerr.flush();
					clReleaseProgram(program);
					throw ocl_runtime_error(
							"Could not load pre-compiled OpenCL program.", err);
				}
				std::cout << "Read Binary " << binaryName << std::endl;
			}
		}
	}
	if (program == nullptr) {
		program = compileFiles(fileNames, constants)->getHandle();
		if (program != nullptr) {
			const int N = 1;
			std::array<size_t, N> programBinarySizes;
			programBinarySizes.fill(0);
			err = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES,
					N * sizeof(size_t), &programBinarySizes[0], NULL);
			if (err == CL_SUCCESS && programBinarySizes[0] > 0) {
				std::vector<char> dataFile(programBinarySizes[0]);
				std::array<char*, N> dataArray;
				dataArray[0] = dataFile.data();
				err = clGetProgramInfo(program, CL_PROGRAM_BINARIES,N * sizeof(char*), &dataArray[0], NULL);
				if (err != CL_SUCCESS)
					throw ocl_runtime_error("Could not get pre-compiled binary.", err);
				WriteBinaryFile(binaryName, dataFile);
				std::cout << "Wrote Binary " << binaryName << std::endl;
			} else {
				throw ocl_runtime_error("Could not get binary size.", err);
			}
		}
	}
	return std::shared_ptr<ProgramCL>(new ProgramCL(program, name));
}

}


/*
 * AlloyCompression.cpp
 *
 *  Created on: Nov 1, 2018
 *      Author: blake
 */

#include "AlloyCompression.h"
#include "lzf.h"
#include "miniz.h"
#include "lz4/lz4.h"
#include "zstd.h"
#include <cstring>
#include <exception>
#include <stdexcept>
namespace aly {
size_t EstimateOutputSize(size_t sz, CompressionType type) {
	switch (type) {
	case CompressionType::ZIP:
		return mz_compressBound(sz);
	case CompressionType::LFW:
		return EstimateLZF(sz);
	case CompressionType::ZSTD:
		return ZSTD_compressBound(sz);
	case CompressionType::LZ4:
		return LZ4_compressBound(sz);
	default:
		return sz;
	}
}
size_t Compress(const unsigned char* input, size_t input_sz,
		unsigned char* output, size_t output_sz, CompressionType type) {
	switch (type) {
	case CompressionType::ZIP: {
		mz_ulong outSize = output_sz;
		int ret = mz_compress(output, &outSize, input, input_sz);
		assert(ret == MZ_OK);
		return outSize;
	}
	case CompressionType::LFW:
		return CompressLZF(input, input_sz, output, output_sz);
	case CompressionType::ZSTD:
		return ZSTD_compress(output, output_sz, input, input_sz, 1);
	case CompressionType::LZ4:
	{
		size_t outSize = output_sz;
		return LZ4_compress_fast((const char*) input, (char*) output, input_sz,
				output_sz, 1);
	}
	case CompressionType::NONE:
	default:
		std::memcpy(output,input,input_sz);
		return input_sz;
	}
}
void Decompress(const unsigned char* input, size_t input_sz,
		unsigned char* output, size_t output_sz, CompressionType type) {
	switch (type) {
		case CompressionType::ZIP:
		{
			mz_ulong outSize = output_sz;
			int ret = mz_uncompress(output, &outSize, input, input_sz);
			assert(ret == MZ_OK);
			break;
		}
		case CompressionType::LFW:
			DecompressLZF(input, input_sz, output, output_sz);
			break;
		case CompressionType::ZSTD:
			ZSTD_decompress(output, output_sz, input, input_sz);
			break;
		case CompressionType::LZ4:
			LZ4_decompress_fast((const char*) input, (char*) output, output_sz);
			break;
		case CompressionType::NONE:
		default:
			std::memcpy(output,input,input_sz);
			break;
	}
}
size_t Compress(const std::string& input,std::string& output,CompressionType type){
	size_t est=EstimateOutputSize(input.size(),type);
	output.resize(est);
	size_t sz=Compress((const unsigned char*)input.c_str(),input.size(),(unsigned char*)output.c_str(),sz,type);
	if(est!=sz)output.erase(sz,est-sz);
	return sz;
}
void Decompress(const std::string& input,std::string& output,CompressionType type){
	if(output.size()==0){
		throw std::runtime_error("Must pre-allocate output size before decompression.");
	}
	Decompress((const unsigned char*)input.c_str(),input.size(),(unsigned char*)output.c_str(),output.size(),type);
}

size_t Compress(const std::vector<uint8_t>& input,std::vector<uint8_t>& output,CompressionType type){
	size_t est=EstimateOutputSize(input.size(),type);
	output.resize(est);
	size_t sz=Compress((const unsigned char*)input.data(),input.size(),(unsigned char*)output.data(),sz,type);
	if(est!=sz)output.erase(output.begin()+sz,output.end());
	return sz;
}
void Decompress(const std::vector<uint8_t>& input,std::vector<uint8_t>& output,CompressionType type){
	if(output.size()==0){
		throw std::runtime_error("Must pre-allocate output size before decompression.");
	}
	Decompress((const unsigned char*)input.data(),input.size(),(unsigned char*)output.data(),output.size(),type);
}
}

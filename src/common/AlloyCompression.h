/*
 * AlloyCompression.h
 *
 *  Created on: Nov 1, 2018
 *      Author: blake
 */

#ifndef SRC_COMMON_ALLOYCOMPRESSION_H_
#define SRC_COMMON_ALLOYCOMPRESSION_H_

#include <stddef.h>
#include <string>
#include <vector>
namespace aly {
	enum class CompressionType{
		NONE=0,ZIP=1,LFW=2,LZ4=3,ZSTD=4
	};
	size_t EstimateOutputSize(size_t sz,CompressionType type);
	size_t Compress(unsigned char* input, size_t input_sz, unsigned char* output, size_t output_sz,CompressionType type);
	void Decompress(unsigned char* input, size_t input_sz, unsigned char* output, size_t output_sz,CompressionType type);

	size_t Compress(const std::string& input,std::string& output,CompressionType type);
	void Decompress(const std::string& input,std::string& output,CompressionType type);

	size_t Compress(const std::vector<uint8_t>& input,std::vector<uint8_t>& output,CompressionType type);
	void Decompress(const std::vector<uint8_t>& input,std::vector<uint8_t>& output,CompressionType type);

}
#endif /* SRC_COMMON_ALLOYCOMPRESSION_H_ */

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
#include <ostream>
namespace aly {
	enum class CompressionType{
		NONE=0,ZIP=1,LFW=2,LZ4=3,ZSTD=4
	};
	template<class C, class R> std::basic_ostream<C, R> & operator <<(
			std::basic_ostream<C, R> & ss, const CompressionType& type) {
		switch (type) {
		case CompressionType::NONE:
			return ss << "None";
		case CompressionType::ZIP:
			return ss << "ZIP";
		case CompressionType::LFW:
			return ss << "LFW";
		case CompressionType::LZ4:
			return ss << "LZ4";
		case CompressionType::ZSTD:
			return ss << "ZSTD";
		}
		return ss;
	}
	size_t EstimateOutputSize(size_t sz,CompressionType type);
	size_t Compress(const unsigned char* input, size_t input_sz, unsigned char* output, size_t output_sz,CompressionType type);
	void Decompress(const unsigned char* input, size_t input_sz, unsigned char* output, size_t output_sz,CompressionType type);

	size_t Compress(const std::string& input,std::string& output,CompressionType type);
	void Decompress(const std::string& input,std::string& output,CompressionType type);

	size_t Compress(const std::vector<uint8_t>& input,std::vector<uint8_t>& output,CompressionType type);
	void Decompress(const std::vector<uint8_t>& input,std::vector<uint8_t>& output,CompressionType type);

}
#endif /* SRC_COMMON_ALLOYCOMPRESSION_H_ */

/*
 * HoudiniReaderWriter.cpp
 *
 *  Created on: Feb 15, 2019
 *      Author: blake
 */

#include "HoudiniReaderWriter.h"
#include "houdini/houdini_json.h"
#include "system/AlloyFileUtil.h"
#include <fstream>
using namespace aly;
namespace intel {

void WriteMeshToHoudini(const std::string& file, const aly::Mesh& mesh,
		bool binary) {
	std::string ext = GetFileExtension(file);
	int flags = std::ios_base::out;
	std::ofstream out;
	std::shared_ptr<houio::json::Writer> writer;
	if (binary) {
		out.open(GetFileNameWithoutExtension(file) + ".bgeo",
				std::ios_base::out | std::ios_base::binary);
		writer = std::shared_ptr<houio::json::Writer>(
				new houio::json::BinaryWriter(&out));
	} else {
		out.open(GetFileNameWithoutExtension(file) + ".geo",
				std::ios_base::out);
		writer = std::shared_ptr<houio::json::Writer>(
				new houio::json::ASCIIWriter(&out));
	}
	bool hasPrimitives = (mesh.triIndexes.size() > 0
			|| mesh.quadIndexes.size() > 0);
	size_t primCount = mesh.triIndexes.size() + mesh.quadIndexes.size();
	size_t vertexCount = mesh.triIndexes.size() * 3
			+ mesh.quadIndexes.size() * 4;
	writer->jsonBeginArray();

	writer->jsonString("fileversion");
	writer->jsonString("16.5.496");

	writer->jsonString("hasindex");
	writer->jsonBool(false);

	writer->jsonString("pointcount");
	writer->jsonInt(mesh.vertexLocations.size());

	writer->jsonString("vertexcount");
	writer->jsonInt(vertexCount);

	writer->jsonString("primitivecount");
	writer->jsonInt(vertexCount);

	writer->jsonString("info");
	writer->jsonBeginMap();
	writer->jsonKey("software");
	writer->jsonString("Houdini 16.5.496");
	writer->jsonKey("artist");
	writer->jsonString("produser");
	writer->jsonEndMap();

	writer->jsonString("topology");
	writer->jsonBeginArray();
	writer->jsonString("pointref");
	writer->jsonBeginArray();
	writer->jsonString("indices");
	std::vector<int> buffer;
	writer->jsonBeginArray();
	for (uint3 tri : mesh.triIndexes) {
		writer->jsonInt32(tri.x);
		writer->jsonInt32(tri.y);
		writer->jsonInt32(tri.z);
	}
	for (uint4 quad : mesh.quadIndexes) {
		writer->jsonInt32(quad.x);
		writer->jsonInt32(quad.y);
		writer->jsonInt32(quad.z);
		writer->jsonInt32(quad.w);
	}
	writer->jsonEndArray();
	writer->jsonEndArray();
	writer->jsonEndArray();

	writer->jsonString("attributes");

}

void ReadMeshToHoudini(const std::string& file, aly::Mesh& mesh) {

}

}

/*
 * Copyright(C) 2019, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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
#include "HoudiniReaderWriter.h"
#include "houdini/houdini_json.h"
#include "system/AlloyFileUtil.h"
#include "system/gzstream.h"
#include <fstream>
namespace aly {
void SANITY_CHECK_HOUDINI() {
	Mesh monkey;
	ReadMeshFromFile(
			"/home/blake/workspace/studio/alloy/assets/models/monkey.ply",
			monkey);
	std::cout << "Write Monkey Geo" << std::endl;
	WriteMeshToHoudini(MakeDesktopFile("monkey.geo"), monkey, false, false);
	std::cout << "Write Monkey BGeo" << std::endl;
	WriteMeshToHoudini(MakeDesktopFile("monkey.bgeo"), monkey, true, false);
	std::cout << "Done" << std::endl;
	monkey.clear();
	ReadMeshFromHoudini(MakeDesktopFile("monkey.geo"), monkey);
	std::cout << "Monkey " << monkey << std::endl;
	WriteMeshToFile(MakeDesktopFile("monkey.ply"), monkey);

	Mesh eagle;
	ReadMeshFromFile(
			"/home/blake/workspace/studio/alloy/assets/models/eagle.ply",
			eagle);
	std::cout << "Write Eagle Geo" << std::endl;
	WriteMeshToHoudini(MakeDesktopFile("eagle.bgeo.gz"), eagle, true, true);
	std::cout << "Done" << std::endl;
	eagle.clear();
	ReadMeshFromHoudini(MakeDesktopFile("eagle.bgeo.gz"), eagle);
	std::cout << "Eagle " << eagle << std::endl;
	WriteMeshToFile(MakeDesktopFile("eagle.ply"), eagle);

}
void WriteMeshToHoudini(const std::string& file, const aly::Mesh& mesh,
		bool binary, bool compress) {
	using namespace aly::houdini;
	int flags = std::ios_base::out;
	std::shared_ptr<std::ostream> out;
	std::shared_ptr<HJSONWriter> writer;
	std::string tmpFile = file;
	std::string ext = GetFileExtension(file);
	if (ext == "gz") {
		tmpFile = GetFileWithoutExtension(file);
	}
	tmpFile = GetFileWithoutExtension(tmpFile);
	if (compress) {
		if (binary) {
			out = std::shared_ptr<std::ostream>(
					new ogzstream(tmpFile + ".bgeo.gz",
							std::ios_base::out | std::ios_base::binary));
			writer = std::shared_ptr<HJSONWriter>(
					new HoudiniBinaryWriter(out.get()));
		} else {
			out = std::shared_ptr<std::ostream>(
					new ogzstream(tmpFile + ".geo.gz", std::ios_base::out));
			writer = std::shared_ptr<HJSONWriter>(
					new HoudiniTextWriter(out.get()));
		}
	} else {
		if (binary) {
			out = std::shared_ptr<std::ostream>(
					new std::ofstream(tmpFile + ".bgeo",
							std::ios_base::out | std::ios_base::binary));
			writer = std::shared_ptr<HJSONWriter>(
					new HoudiniBinaryWriter(out.get()));
		} else {
			out = std::shared_ptr<std::ostream>(
					new std::ofstream(tmpFile + ".geo", std::ios_base::out));
			writer = std::shared_ptr<HJSONWriter>(
					new HoudiniTextWriter(out.get()));
		}
	}

	bool hasPrimitives = (mesh.triIndexes.size() > 0
			|| mesh.quadIndexes.size() > 0);
	size_t primCount = mesh.lineIndexes.size() + mesh.triIndexes.size()
			+ mesh.quadIndexes.size();
	size_t vertexCount = mesh.lineIndexes.size() * 2
			+ mesh.triIndexes.size() * 3 + mesh.quadIndexes.size() * 4;
	{
		writer->beginArray();
		writer->putText("fileversion");
		writer->putText("16.5.496");

		writer->putText("hasindex");
		writer->putBool(false);

		writer->putText("pointcount");
		writer->putInt(mesh.vertexLocations.size());

		writer->putText("vertexcount");
		writer->putInt(vertexCount);

		writer->putText("primitivecount");
		writer->putInt(primCount);

		writer->putText("info");
		writer->beginMap();
		writer->putMapKey("software");
		writer->putText("Houdini 16.5.496");
		writer->putMapKey("artist");
		writer->putText("produser");
		writer->endMap();
		writer->putText("topology");
		{
			writer->beginArray();
			writer->putText("pointref");
			{
				writer->beginArray();
				writer->putText("indices");
				{
					std::vector<int> flatten;
					flatten.reserve(vertexCount);
					for (uint2 lin : mesh.lineIndexes) {
						flatten.push_back(lin.x);
						flatten.push_back(lin.y);
					}
					for (uint3 tri : mesh.triIndexes) {
						flatten.push_back(tri.x);
						flatten.push_back(tri.y);
						flatten.push_back(tri.z);
					}
					for (uint4 quad : mesh.quadIndexes) {
						flatten.push_back(quad.x);
						flatten.push_back(quad.y);
						flatten.push_back(quad.z);
						flatten.push_back(quad.w);
					}
					writer->putUniformArray(flatten);
				}
				writer->endArray();
			}
			writer->endArray();
		}
		writer->putText("attributes");
		{
			writer->beginArray();
			writer->putText("pointattributes");
			{
				writer->beginArray();
				{

					if (mesh.vertexLocations.size() > 0) {
						writer->beginArray();
						{
							writer->beginArray();
							writer->putText("scope");
							writer->putText("public");
							writer->putText("type");
							writer->putText("numeric");
							writer->putText("name");
							writer->putText("P");
							writer->putText("options");
							writer->beginMap();
							writer->putMapKey("type");
							writer->beginMap();
							writer->putMapKey("type");
							writer->putText("string");
							writer->putMapKey("value");
							writer->putText("point");
							writer->endMap();
							writer->endMap();
							writer->endArray();
						}
						{
							writer->beginArray();
							writer->putText("size");
							writer->putInt(3);
							writer->putText("storage");
							writer->putText("fpreal32");
							writer->putText("values");
							{
								writer->beginArray();
								writer->putText("size");
								writer->putInt(3);
								writer->putText("storage");
								writer->putText("fpreal32");
								writer->putText("tuples");
								{
									writer->beginArray();
									for (float3 pt : mesh.vertexLocations) {
										writer->putVec3(pt);
									}
									writer->endArray();
								}
								writer->endArray();
							}
							writer->endArray();
						}
						writer->endArray();
					}
					if (mesh.vertexNormals.size() > 0) {
						writer->beginArray();
						{
							writer->beginArray();
							writer->putText("scope");
							writer->putText("public");
							writer->putText("type");
							writer->putText("numeric");
							writer->putText("name");
							writer->putText("N");
							writer->putText("options");
							writer->beginMap();
							writer->putMapKey("type");
							writer->beginMap();
							writer->putMapKey("type");
							writer->putText("string");
							writer->putMapKey("value");
							writer->putText("normal");
							writer->endMap();
							writer->endMap();
							writer->endArray();
						}
						{
							writer->beginArray();
							writer->putText("size");
							writer->putInt(3);
							writer->putText("storage");
							writer->putText("fpreal32");
							writer->putText("values");
							{
								writer->beginArray();
								writer->putText("size");
								writer->putInt(3);
								writer->putText("storage");
								writer->putText("fpreal32");
								writer->putText("tuples");
								{
									writer->beginArray();
									for (float3 pt : mesh.vertexNormals) {
										writer->putVec3(pt);
									}
									writer->endArray();
								}
								writer->endArray();
							}
							writer->endArray();
						}
						writer->endArray();
					}
					if (mesh.vertexColors.size() > 0) {
						writer->beginArray();
						{
							writer->beginArray();
							writer->putText("scope");
							writer->putText("public");
							writer->putText("type");
							writer->putText("numeric");
							writer->putText("name");
							writer->putText("Cd");
							writer->putText("options");
							writer->beginMap();
							writer->putMapKey("type");
							writer->beginMap();
							writer->putMapKey("type");
							writer->putText("string");
							writer->putMapKey("value");
							writer->putText("color");
							writer->endMap();
							writer->endMap();
							writer->endArray();
						}
						{
							writer->beginArray();
							writer->putText("size");
							writer->putInt(3);
							writer->putText("storage");
							writer->putText("fpreal32");
							writer->putText("values");
							{
								writer->beginArray();
								writer->putText("size");
								writer->putInt(3);
								writer->putText("storage");
								writer->putText("fpreal32");
								writer->putText("tuples");
								{
									writer->beginArray();
									for (float4 pt : mesh.vertexColors) {
										writer->putVec3(pt.xyz());
									}
									writer->endArray();
								}
								writer->endArray();
							}
							writer->endArray();
						}
						writer->endArray();
					}
					writer->endArray();
				}
			}
			writer->endArray();
		}
		writer->putText("primitives");
		writer->beginArray();
		if (primCount > 0) {
			writer->beginArray();
			{
				writer->beginArray();
				writer->putText("type");
				writer->putText("Polygon_run");
				writer->endArray();
				if (mesh.lineIndexes.size() > 0) {
					writer->beginArray();
					writer->putText("startvertex");
					writer->putInt(0);
					writer->putText("nprimitives");
					writer->putInt(mesh.lineIndexes.size());
					writer->putText("nvertices_rle");
					{
						writer->beginArray();
						writer->putInt(2);
						writer->putInt(mesh.lineIndexes.size());
						writer->endArray();
					}
					writer->endArray();
				}
				if (mesh.triIndexes.size() > 0) {
					writer->beginArray();
					writer->putText("startvertex");
					writer->putInt(mesh.lineIndexes.size() * 2);
					writer->putText("nprimitives");
					writer->putInt(mesh.triIndexes.size());
					writer->putText("nvertices_rle");
					{
						writer->beginArray();
						writer->putInt(3);
						writer->putInt(mesh.triIndexes.size());
						writer->endArray();
					}
					writer->endArray();
				}
				if (mesh.quadIndexes.size() > 0) {
					writer->beginArray();
					writer->putText("startvertex");
					writer->putInt(
							mesh.triIndexes.size() * 3
									+ mesh.lineIndexes.size() * 2);
					writer->putText("nprimitives");
					writer->putInt(mesh.quadIndexes.size());
					writer->putText("nvertices_rle");
					{
						writer->beginArray();
						writer->putInt(4);
						writer->putInt(mesh.quadIndexes.size());
						writer->endArray();
					}
					writer->endArray();
				}
			}
			writer->endArray();
		}
		writer->endArray();
	}
	writer->endArray();
}
void ReadMeshFromHoudini(const std::string& file, aly::Mesh& mesh) {
	using namespace aly::houdini;
	mesh.clear();
	std::string ext = GetFileExtension(file);
	std::shared_ptr<std::istream> in;
	HJSONReader reader;
	HJSONParser p;
	if (ext == "gz") {
		in = std::shared_ptr<std::istream>(
				new igzstream(file, std::ios_base::in | std::ios_base::binary));
	} else if (ext == "bgeo") {
		in = std::shared_ptr<std::ifstream>(
				new std::ifstream(file,
						std::ios_base::in | std::ios_base::binary));
	} else if (ext == "geo") {
		in = std::shared_ptr<std::ifstream>(
				new std::ifstream(file, std::ios_base::in));
	} else {
		throw std::runtime_error(
				MakeString() << "Unsupported extension for " << file);
	}
	if (!p.parse(in.get(), &reader)) {
		throw std::runtime_error(MakeString() << "Could not parse " << file);
	}
	// now the reader contains the json data from the complete file
	// we access it via the root json object.

	// For houdini files, the root json object is an array.
	// The reason for that is that the order of items needs
	// to be contained which does not happen for json objects.
	ArrayPtr root = reader.getRoot().asArray();

	// the houdini root array is a flattened json object
	// in order to work with it more conveniently (e.g. be able
	// to query keys for existance etc.), we unflatten the array
	// into an object again
	ObjectPtr obj = ToHoudiniObject(root);
	// now we can start to query the json data for its content. This requires
	// to know the schema of the layout which unfortunately is not documented.
	// The logger can be used to learn the schema from any given file.
	sint64 numVertices = 0;
	sint64 numPoints = 0;
	sint64 numPrimitives = 0;
	if (obj->hasKey("pointcount"))
		numPoints = obj->get<int>("pointcount", 0);
	if (obj->hasKey("vertexcount"))
		numVertices = obj->get<int>("vertexcount", 0);
	if (obj->hasKey("primitivecount"))
		numPrimitives = obj->get<int>("primitivecount", 0);

	std::cout << "#points:" << numPoints << std::endl;
	std::cout << "#verts:" << numVertices << std::endl;
	std::cout << "#prims:" << numPrimitives << std::endl;
	if (obj->hasKey("attributes")) {
		ObjectPtr attributes = ToHoudiniObject(obj->getArray("attributes"));
		if (attributes->hasKey("pointattributes")) {
			ArrayPtr pointAttributes = attributes->getArray("pointattributes");
			bool foundPoints = false;
			bool foundColors = false;
			bool foundNormals = false;
			for (int idx = 0; idx < pointAttributes->size(); idx++) {
				std::map<std::string, Value*> index;
				pointAttributes->getValue(idx).buildIndex(index);
				for (auto pr : index) {
					auto vals = aly::Split(pr.first, '.', false);
					std::cout << "Vals " << pr.first << std::endl;
					if (vals.back() == "P") {
						foundPoints = true;
					} else if (vals.back() == "N") {
						foundNormals = true;
					} else if (vals.back() == "Cd") {
						foundColors = true;
					} else if (vals.back() == "tuples") {
						if (foundPoints) {
							ArrayPtr points = pr.second->asArray();
							sint64 numPointAttributes = points->size();
							mesh.vertexLocations.resize(numPointAttributes);
							for (int i = 0; i < numPointAttributes; i++) {
								ArrayPtr pointAttribute = points->getArray(i);
								mesh.vertexLocations[i] =
										float3(
												pointAttribute->getValue(0).as<
														real32>(),
												pointAttribute->getValue(1).as<
														real32>(),
												pointAttribute->getValue(2).as<
														real32>());
							}
							foundPoints = false;
						}
						if (foundNormals) {
							ArrayPtr points = pr.second->asArray();
							sint64 numPointAttributes = points->size();
							mesh.vertexNormals.resize(numPointAttributes);
							for (int i = 0; i < numPointAttributes; i++) {
								ArrayPtr pointAttribute = points->getArray(i);
								mesh.vertexNormals[i] =
										float3(
												pointAttribute->getValue(0).as<
														real32>(),
												pointAttribute->getValue(1).as<
														real32>(),
												pointAttribute->getValue(2).as<
														real32>());
							}
							foundNormals = false;
						}
						if (foundColors) {
							ArrayPtr points = pr.second->asArray();
							sint64 numPointAttributes = points->size();
							mesh.vertexColors.resize(numPointAttributes);
							for (int i = 0; i < numPointAttributes; i++) {
								ArrayPtr pointAttribute = points->getArray(i);
								mesh.vertexColors[i] =
										float4(
												pointAttribute->getValue(0).as<
														real32>(),
												pointAttribute->getValue(1).as<
														real32>(),
												pointAttribute->getValue(2).as<
														real32>(),
												(pointAttribute->size() == 4) ?
														pointAttribute->getValue(
																3).as<real32>() :
														1.0f);
							}
							foundColors = false;
						}
					}
				}
			}

		}
		if (attributes->hasKey("vertexattributes")) {
			ArrayPtr vertexAttributes = attributes->getArray(
					"vertexattributes");
			sint64 numVertexAttributes = vertexAttributes->size();
			for (int i = 0; i < numVertexAttributes; ++i) {
				ArrayPtr vertexAttribute = vertexAttributes->getArray(i);
				// here we pass loading the attribute from the json object to a seperate function
				//loadAttribute( pointAttribute, numPoints );
			}
		}
		if (attributes->hasKey("primitiveattributes")) {
			ArrayPtr primitiveAttributes = attributes->getArray(
					"primitiveattributes");
			sint64 numPrimitiveAttributes = primitiveAttributes->size();
			for (int i = 0; i < numPrimitiveAttributes; ++i) {
				ArrayPtr primitiveAttribute = primitiveAttributes->getArray(i);
				// here we pass loading the attribute from the json object to a seperate function
				//loadAttribute( pointAttribute, numPoints );
			}
		}
		if (attributes->hasKey("globalattributes")) {
			ArrayPtr globalAttributes = attributes->getArray(
					"globalattributes");
			sint64 numGlobalAttributes = globalAttributes->size();
			for (int i = 0; i < numGlobalAttributes; ++i) {
				ArrayPtr globalAttribute = globalAttributes->getArray(i);
				// here we pass loading the attribute from the json object to a seperate function
				//loadAttribute( pointAttribute, numPoints );
			}
		}
	}
	if (obj->hasKey("topology")) {
		//loadTopology( toObject(o->getArray("topology")) );
	}
	if (obj->hasKey("primitives")) {
		ArrayPtr primitives = obj->getArray("primitives");
		int numPrimitives = (int) primitives->size();
		for (int j = 0; j < numPrimitives; ++j) {
			ArrayPtr primitive = primitives->getArray(j);
			//loadPrimitive( primitive );
		}
	}
}

}

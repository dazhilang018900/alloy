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
#include <fstream>
namespace aly {
void SANITY_CHECK_HOUDINI() {
	Mesh monkey;
	ReadMeshFromFile(
			"/home/blake/workspace/studio/alloy/assets/models/monkey.ply",
			monkey);
	std::cout << "Monkey " << monkey << std::endl;
	std::cout << "Write Monkey Geo" << std::endl;
	WriteMeshToHoudini(MakeDesktopFile("monkey.geo"), monkey, false);
	std::cout << "Write Monkey BGeo" << std::endl;
	WriteMeshToHoudini(MakeDesktopFile("monkey.bgeo"), monkey, true);
	std::cout << "Done" << std::endl;

	Mesh eagle;
	ReadMeshFromFile(
			"/home/blake/workspace/studio/alloy/assets/models/eagle.ply",
			eagle);
	std::cout << "Eagle " << eagle << std::endl;
	std::cout << "Write Eagle Geo" << std::endl;
	WriteMeshToHoudini(MakeDesktopFile("eagle.geo"), eagle, false);
	std::cout << "Write Eagle BGeo" << std::endl;
	WriteMeshToHoudini(MakeDesktopFile("eagle.bgeo"), eagle, true);
	std::cout << "Done" << std::endl;

}
void WriteMeshToHoudini(const std::string& file, const aly::Mesh& mesh,
		bool binary) {
	std::string ext = GetFileExtension(file);
	int flags = std::ios_base::out;
	std::ofstream out;
	std::shared_ptr<houio::json::Writer> writer;
	if (binary) {
		out.open(GetFileWithoutExtension(file) + ".bgeo",
				std::ios_base::out | std::ios_base::binary);
		writer = std::shared_ptr<houio::json::Writer>(
				new houio::json::BinaryWriter(&out));
	} else {
		out.open(GetFileWithoutExtension(file) + ".geo", std::ios_base::out);
		writer = std::shared_ptr<houio::json::Writer>(
				new houio::json::ASCIIWriter(&out));
	}
	bool hasPrimitives = (mesh.triIndexes.size() > 0
			|| mesh.quadIndexes.size() > 0);
	size_t primCount = mesh.triIndexes.size() + mesh.quadIndexes.size();
	size_t vertexCount = mesh.triIndexes.size() * 3
			+ mesh.quadIndexes.size() * 4;
	{
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
		writer->jsonInt(primCount);

		writer->jsonString("info");
		writer->jsonBeginMap();
		writer->jsonKey("software");
		writer->jsonString("Houdini 16.5.496");
		writer->jsonKey("artist");
		writer->jsonString("produser");
		writer->jsonEndMap();
		writer->jsonString("topology");
		{
			writer->jsonBeginArray();
			writer->jsonString("pointref");
			{
				writer->jsonBeginArray();
				writer->jsonString("indices");
				{
					writer->jsonBeginArray();
					for (uint3 tri : mesh.triIndexes) {
						writer->jsonInt(tri.x);
						writer->jsonInt(tri.y);
						writer->jsonInt(tri.z);
					}
					for (uint4 quad : mesh.quadIndexes) {
						writer->jsonInt(quad.x);
						writer->jsonInt(quad.y);
						writer->jsonInt(quad.z);
						writer->jsonInt(quad.w);
					}
					writer->jsonEndArray();
				}
				writer->jsonEndArray();
			}
			writer->jsonEndArray();
		}
		writer->jsonString("attributes");
		{
			writer->jsonBeginArray();
			writer->jsonString("pointattributes");
			{
				writer->jsonBeginArray();
				{

					if (mesh.vertexLocations.size() > 0) {
						writer->jsonBeginArray();
						{
							writer->jsonBeginArray();
							writer->jsonString("scope");
							writer->jsonString("public");
							writer->jsonString("type");
							writer->jsonString("numeric");
							writer->jsonString("name");
							writer->jsonString("P");
							writer->jsonString("options");
							writer->jsonBeginMap();
							writer->jsonKey("type");
							writer->jsonBeginMap();
							writer->jsonKey("type");
							writer->jsonString("string");
							writer->jsonKey("value");
							writer->jsonString("point");
							writer->jsonEndMap();
							writer->jsonEndMap();
							writer->jsonEndArray();
						}
						{
							writer->jsonBeginArray();
							writer->jsonString("size");
							writer->jsonInt(3);
							writer->jsonString("storage");
							writer->jsonString("fpreal32");
							writer->jsonString("values");
							{
								writer->jsonBeginArray();
								writer->jsonString("size");
								writer->jsonInt(3);
								writer->jsonString("storage");
								writer->jsonString("fpreal32");
								writer->jsonString("tuples");
								{
									writer->jsonBeginArray();
									for (float3 pt : mesh.vertexLocations) {
										writer->jsonBeginArray();
										writer->jsonReal32(pt.x);
										writer->jsonReal32(pt.y);
										writer->jsonReal32(pt.z);
										writer->jsonEndArray();
									}
									writer->jsonEndArray();
								}
								writer->jsonEndArray();
							}
							writer->jsonEndArray();
						}
						writer->jsonEndArray();
					}
					if (mesh.vertexNormals.size() > 0) {
						writer->jsonBeginArray();
						{
							writer->jsonBeginArray();
							writer->jsonString("scope");
							writer->jsonString("public");
							writer->jsonString("type");
							writer->jsonString("numeric");
							writer->jsonString("name");
							writer->jsonString("N");
							writer->jsonString("options");
							writer->jsonBeginMap();
							writer->jsonKey("type");
							writer->jsonBeginMap();
							writer->jsonKey("type");
							writer->jsonString("string");
							writer->jsonKey("value");
							writer->jsonString("normal");
							writer->jsonEndMap();
							writer->jsonEndMap();
							writer->jsonEndArray();
						}
						{
							writer->jsonBeginArray();
							writer->jsonString("size");
							writer->jsonInt(3);
							writer->jsonString("storage");
							writer->jsonString("fpreal32");
							writer->jsonString("values");
							{
								writer->jsonBeginArray();
								writer->jsonString("size");
								writer->jsonInt(3);
								writer->jsonString("storage");
								writer->jsonString("fpreal32");
								writer->jsonString("tuples");
								{
									writer->jsonBeginArray();
									for (float3 pt : mesh.vertexNormals) {
										writer->jsonBeginArray();
										writer->jsonReal32(pt.x);
										writer->jsonReal32(pt.y);
										writer->jsonReal32(pt.z);
										writer->jsonEndArray();
									}
									writer->jsonEndArray();
								}
								writer->jsonEndArray();
							}
							writer->jsonEndArray();
						}
						writer->jsonEndArray();
					}
					if (mesh.vertexColors.size() > 0) {
						writer->jsonBeginArray();
						{
							writer->jsonBeginArray();
							writer->jsonString("scope");
							writer->jsonString("public");
							writer->jsonString("type");
							writer->jsonString("numeric");
							writer->jsonString("name");
							writer->jsonString("Cd");
							writer->jsonString("options");
							writer->jsonBeginMap();
							writer->jsonKey("type");
							writer->jsonBeginMap();
							writer->jsonKey("type");
							writer->jsonString("string");
							writer->jsonKey("value");
							writer->jsonString("color");
							writer->jsonEndMap();
							writer->jsonEndMap();
							writer->jsonEndArray();
						}
						{
							writer->jsonBeginArray();
							writer->jsonString("size");
							writer->jsonInt(3);
							writer->jsonString("storage");
							writer->jsonString("fpreal32");
							writer->jsonString("values");
							{
								writer->jsonBeginArray();
								writer->jsonString("size");
								writer->jsonInt(3);
								writer->jsonString("storage");
								writer->jsonString("fpreal32");
								writer->jsonString("tuples");
								{
									writer->jsonBeginArray();
									for (float4 pt : mesh.vertexColors) {
										writer->jsonBeginArray();
										writer->jsonReal32(pt.x);
										writer->jsonReal32(pt.y);
										writer->jsonReal32(pt.z);
										writer->jsonEndArray();
									}
									writer->jsonEndArray();
								}
								writer->jsonEndArray();
							}
							writer->jsonEndArray();
						}
						writer->jsonEndArray();
					}
					writer->jsonEndArray();
				}
			}
			writer->jsonEndArray();
		}

		writer->jsonString("primitives");
		writer->jsonBeginArray();
		if (primCount > 0) {
			writer->jsonBeginArray();
			{
				writer->jsonBeginArray();
				writer->jsonString("type");
				writer->jsonString("Polygon_run");
				writer->jsonEndArray();
				if (mesh.triIndexes.size() > 0) {
					writer->jsonBeginArray();
					writer->jsonString("startvertex");
					writer->jsonInt(0);
					writer->jsonString("nprimitives");
					writer->jsonInt(mesh.triIndexes.size());
					writer->jsonString("nvertices_rle");
					{
						writer->jsonBeginArray();
						writer->jsonInt(3);
						writer->jsonInt(mesh.triIndexes.size());
						writer->jsonEndArray();
					}
					writer->jsonEndArray();
				}
				if (mesh.quadIndexes.size() > 0) {
					writer->jsonBeginArray();
					writer->jsonString("startvertex");
					writer->jsonInt(mesh.triIndexes.size() * 3);
					writer->jsonString("nprimitives");
					writer->jsonInt(mesh.quadIndexes.size());
					writer->jsonString("nvertices_rle");
					{
						writer->jsonBeginArray();
						writer->jsonInt(4);
						writer->jsonInt(mesh.quadIndexes.size());
						writer->jsonEndArray();
					}
					writer->jsonEndArray();
				}
			}
			writer->jsonEndArray();
		}
		writer->jsonEndArray();
	}
	writer->jsonEndArray();
}
/*
 void ReadMeshFromHoudini(const std::string& file, aly::Mesh& mesh) {

 }
 */
}

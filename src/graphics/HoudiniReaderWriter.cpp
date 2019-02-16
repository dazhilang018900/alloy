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
	using namespace aly::houdini;
	std::string ext = GetFileExtension(file);
	int flags = std::ios_base::out;
	std::ofstream out;
	std::shared_ptr<HJSONWriter> writer;
	if (binary) {
		out.open(GetFileWithoutExtension(file) + ".bgeo",
				std::ios_base::out | std::ios_base::binary);
		writer = std::shared_ptr<HJSONWriter>(new HoudiniBinaryWriter(&out));
	} else {
		out.open(GetFileWithoutExtension(file) + ".geo", std::ios_base::out);
		writer = std::shared_ptr<HJSONWriter>(new HoudiniTextWriter(&out));
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
					writer->putInt(mesh.lineIndexes.size()*2);
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
					writer->putInt(mesh.triIndexes.size() * 3+mesh.lineIndexes.size()*2);
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
/*
 void ReadMeshFromHoudini(const std::string& file, aly::Mesh& mesh) {

 }
 */
}

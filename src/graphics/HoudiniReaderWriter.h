/*
 * HoudiniReaderWriter.h
 *
 *  Created on: Feb 15, 2019
 *      Author: blake
 */

#ifndef COMMON_HOUDINI_HOUDINIREADERWRITER_H_
#define COMMON_HOUDINI_HOUDINIREADERWRITER_H_
#include "AlloyMesh.h"
namespace aly {

void WriteMeshToHoudini(const std::string& file,const aly::Mesh& mesh,bool binary=true);
void ReadMeshToHoudini(const std::string& file,aly::Mesh& mesh);

}
#endif /* COMMON_HOUDINI_HOUDINIREADERWRITER_H_ */

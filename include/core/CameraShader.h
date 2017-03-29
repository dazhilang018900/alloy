#include "GLTexture.h"
#include "GLShader.h"
#include "GLFrameBuffer.h"
#include "AlloyImage.h"
#include "AlloyUnits.h"
#include "AlloyCamera.h"
#include "AlloyMeshPrimitives.h"
#include <initializer_list>
namespace aly{

class CameraShader: public GLShader {
protected:
	Color ambientColor;
	Color diffuseColor;
	Color selectedCameraColor;
	float3 lightDirection;
public:
	void setAmbientColor(const Color& c){
		ambientColor=c;
	}
	void setDiffuseColor(const Color& c){
		diffuseColor=c;
	}
	void setSelectedColor(const Color& c){
		selectedCameraColor=c;
	}
	void setLightDirection(const float3& dir){
		lightDirection=dir;
	}
	CameraShader(bool onScreen = true,const std::shared_ptr<AlloyContext>& context =AlloyDefaultContext());
	void draw(const std::vector<std::shared_ptr<Frustum>>& cameras,CameraParameters& camera,GLFrameBuffer& frameBuffer,int selected=-1);
	void draw(const std::vector<std::shared_ptr<Frustum>>& cameras,CameraParameters& camera, const box2px& bounds,int selected=-1);
};

}

#include "graphics/GLTexture.h"
#include "graphics/GLShader.h"
#include "graphics/GLFrameBuffer.h"
#include "image/AlloyImage.h"
#include "graphics/AlloyCamera.h"
#include "graphics/AlloyMeshPrimitives.h"
#include <initializer_list>
#include <set>

#include "common/AlloyUnits.h"
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
	void draw(const std::vector<std::shared_ptr<Frustum>>& cameras, CameraParameters& camera, GLFrameBuffer& frameBuffer, const std::set<int>& selected);
	void draw(const std::vector<std::shared_ptr<Frustum>>& cameras,CameraParameters& camera, const box2px& bounds,int selected=-1);
	void draw(const std::vector<std::shared_ptr<Frustum>>& cameras, CameraParameters& camera, const box2px& bounds, const std::set<int>& selected);
};

}

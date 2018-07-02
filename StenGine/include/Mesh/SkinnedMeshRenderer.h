#pragma once

#include "Mesh/MeshRenderer.h"
#include "Math/MathDefs.h"
#include "Scene/Component.h"

namespace StenGine
{

class SkinnedMesh;

class SkinnedMeshRenderer : public MeshRenderer {
public:
	SkinnedMeshRenderer();
	virtual ~SkinnedMeshRenderer();

	virtual void SetMesh(Mesh* mesh) override;
	virtual void GatherDrawCall() override;
	virtual void GatherShadowDrawCall() override;

	virtual void DrawMenu() override {}
private:

	void PrepareMatrixPalette();

	virtual void PrepareGPUBuffer() override;
	virtual void PrepareShadowMapBuffer() override;

	std::vector<Mat4> m_toRootTransform;
	std::vector<Mat4> m_matrixPalette;
	SkinnedMesh* mSkinnedMesh;
};

}
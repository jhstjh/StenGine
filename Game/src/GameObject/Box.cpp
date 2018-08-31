#include "stdafx.h"

#include "GameObject/box.h"
#include "Mesh/MeshRenderer.h"
#include "Resource/ResourceManager.h"

namespace SGGame
{

Box::Box()
{
	Mesh* box0Mesh = ResourceManager::Instance()->GetResource<Mesh>(L"GenerateBox");
	auto box0MeshRenderer = std::make_unique<MeshRenderer>();
	box0MeshRenderer->SetMesh(box0Mesh);
	AddComponent(std::move(box0MeshRenderer));
}

}
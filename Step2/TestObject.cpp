#include "TestObject.h"
#include "BaseComponent.h"
#include "GraphicComponent.h"

void TestObject::Init()
{
	AddComponent<ComTransform>();
	auto mesh = AddComponent<ComMesh>();
	AddComponent<ComRenderer>();
	AddComponent<ComAnimator>();

	std::vector<std::string> names;
	mesh->GetMeshNames(names);

	if (names.size())
	{
		mesh->SelectMesh(names.front());
	}
}

void TestObject::Update()
{

}

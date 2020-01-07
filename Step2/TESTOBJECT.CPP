#include "TestObject.h"
#include "Vertex.h"
#include "GraphicComponent.h"

void TestObject::Update(float delta)
{
}

void TestObject::Init()
{
	auto mesh = AddComponent<ComMesh>();

	std::vector<std::string> testMaterialNames(1);
	std::vector<Material> testMaterials(1);
	testMaterials.front().diffuseMapIndex = 0;
	testMaterialNames.front() = "testMaterial";
	mesh->GetDeviceMeshWorks()->createMaterialfunc(testMaterialNames, testMaterials);

	MeshObject testMesh;
	testMesh.type = CGH::MESH_NORMAL;
	testMesh.primitiveType = 4;
	SubmeshData sub;
	sub.indexOffset = 0;
	sub.vertexOffset = 0;
	sub.numIndex = 24;
	sub.numVertex = 8;
	sub.material = "testMaterial";
	testMesh.subs["base"] = sub;
	std::vector<Vertex> vertices(8);
	std::vector<unsigned int> indices(24);
	mesh->GetDeviceMeshWorks()->createMeshfunc("TestMesh", testMesh, 
		sizeof(Vertex) * vertices.size(), vertices.data(), indices);
}

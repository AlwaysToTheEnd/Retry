#include "TestObject.h"
#include "BaseComponent.h"
#include "GraphicComponent.h"
#include "d3dApp.h"

void TestObject::Init()
{
	m_Transform = AddComponent<ComTransform>();
	AddComponent<ComTransform>();
	auto mesh = AddComponent<ComMesh>();
	AddComponent<ComRenderer>();
	auto ani= AddComponent<ComAnimator>();
	
	std::vector<std::string> names;
	mesh->GetMeshNames(names);

	if (names.size())
	{
		mesh->SelectMesh(names.back());
	}

	names.clear();
	ani->GetSkinNames(names);

	if (names.size())
	{
		ani->SelectSkin(names.back());
	}
	
	names.clear();
	ani->GetAniNames(names);

	if (names.size())
	{
		ani->SelectAnimation(names.front());
	}

	ani->SetRoof(true);
}

void TestObject::Update()
{
	static int index = 0;

	if (GMOUSE.leftButton== MOUSEState::RELEASED)
	{
		auto ani = GetComponent<ComAnimator>();
		
		std::vector<std::string> names;
		ani->GetAniNames(names);

		if (names.size())
		{
			ani->SelectAnimation(names[index]);

			index = (index + 1) % names.size();
		}
	}

	if (GKEYBOARD.lastState.Right)
	{
		m_Transform->m_Pos.x += 0.001f;
	}


	if (GKEYBOARD.lastState.Left)
	{
		m_Transform->m_Pos.x -= 0.001f;
	}
}

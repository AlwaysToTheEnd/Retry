#pragma once
#include <functional>
#include <vector>

class PhysXFunctionalObject
{
public:
	bool IsValideObject()
	{
		return Dogtag == 123456789;
	}

private:
	int Dogtag = 123456789;

public:
	std::vector<std::function<void()>> voidFuncs;
};
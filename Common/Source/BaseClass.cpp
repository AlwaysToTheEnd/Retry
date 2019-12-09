#include "BaseClass.h"
#include <Windows.h>
#include <assert.h>


CGH::MAT16::MAT16()
{
	Identity();
}

CGH::MAT16::MAT16(const float array[])
{
	memcpy(m, array, sizeof(MAT16));
}

CGH::MAT16::MAT16(const MAT16& src)
{
	memcpy(m, src.m, sizeof(MAT16));
}

void CGH::MAT16::Identity()
{
	ZeroMemory(m, sizeof(MAT16));
		
	m[0][0] = 1;
	m[1][1] = 1;
	m[2][2] = 1;
	m[3][3] = 1;
}

float& CGH::MAT16::operator()(unsigned int Row, unsigned int Col)
{
	assert(Row < 4 && Col < 4);

	return m[Row][Col];
}

float CGH::MAT16::operator()(unsigned int Row, unsigned int Col) const
{
	assert(Row < 4 && Col < 4);

	return m[Row][Col];
}

CGH::MAT16::operator float* ()
{
	return *m;
}

CGH::MAT16::operator const float* () const
{
	return *m;
}

CGH::MAT16::operator DirectX::XMFLOAT4X4* ()
{
	return reinterpret_cast<DirectX::XMFLOAT4X4*>(this);
}

CGH::MAT16::operator const DirectX::XMFLOAT4X4* () const
{
	return reinterpret_cast<const DirectX::XMFLOAT4X4*>(this);
}

void CGH::MAT16::operator=(const MAT16& rhs)
{
	memcpy(m, rhs.m, sizeof(MAT16));
}

CGH::MAT16& CGH::MAT16::operator*=(const MAT16& rhs)
{
	MAT16 result;

	for (int i = 0; i < 4; i++)
	{
		float x = m[i][0];
		float y = m[i][1];
		float z = m[i][2];
		float w = m[i][3];

		for (int j = 0; j < 4; j++)
		{
			result.m[i][j] = (rhs.m[0][j] * x) + (rhs.m[1][j] * y) + (rhs.m[2][j] * z) + (rhs.m[3][j] * w);
		}
	}

	*this = result;
	return *this;
}

CGH::MAT16& CGH::MAT16::operator+=(const MAT16& rhs)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			m[i][j] += rhs.m[i][j];
		}
	}

	return *this;
}

CGH::MAT16& CGH::MAT16::operator-=(const MAT16& rhs)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			m[i][j] -= rhs.m[i][j];
		}
	}

	return *this;
}

CGH::MAT16& CGH::MAT16::operator*=(float rhs)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			m[i][j] *= rhs;
		}
	}

	return *this;
}

CGH::MAT16& CGH::MAT16::operator/=(float rhs)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			m[i][j] /= rhs;
		}
	}

	return *this;
}

CGH::MAT16 CGH::MAT16::operator+() const
{
	return *this;
}

CGH::MAT16 CGH::MAT16::operator-() const
{
	return *this;
}

CGH::MAT16 CGH::MAT16::operator*(const MAT16& rhs) const
{
	MAT16 result;

	for (int i = 0; i < 4; i++)
	{
		float x = m[i][0];
		float y = m[i][1];
		float z = m[i][2];
		float w = m[i][3];

		for (int j = 0; j < 4; j++)
		{
			result.m[i][j] = (rhs.m[0][j] * x) + (rhs.m[1][j] * y) + (rhs.m[2][j] * z) + (rhs.m[3][j] * w);
		}
	}

	return result;
}

CGH::MAT16 CGH::MAT16::operator+(const MAT16& rhs) const
{
	MAT16 result;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			result.m[i][j] = m[i][j] + rhs.m[i][j];
		}
	}

	return result;
}

CGH::MAT16 CGH::MAT16::operator-(const MAT16& rhs) const
{
	MAT16 result;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			result.m[i][j] = m[i][j] - rhs.m[i][j];
		}
	}

	return result;
}

CGH::MAT16 CGH::MAT16::operator*(float rhs) const
{
	MAT16 result;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			result.m[i][j] = m[i][j] *rhs;
		}
	}

	return result;
}

CGH::MAT16 CGH::MAT16::operator/(float rhs) const
{
	MAT16 result;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			result.m[i][j] = m[i][j] / rhs;
		}
	}

	return result;
}

bool CGH::MAT16::operator==(const MAT16& rhs) const
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (m[i][j] != rhs.m[i][j])
			{
				return false;
			}
		}
	}

	return true;
}

bool CGH::MAT16::operator!=(const MAT16& rhs) const
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (m[i][j] != rhs.m[i][j])
			{
				return true;
			}
		}
	}

	return false;
}


CGH::MAT16 CGH::operator*(float lhs, const MAT16& rhs)
{
	return rhs * lhs;
}

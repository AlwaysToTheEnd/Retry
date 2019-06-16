#pragma once
#include <DirectXMath.h>

namespace CGH
{
	class MAT16
	{
	public:
		MAT16();
		MAT16(const float array[]);
		MAT16(const MAT16&);

		void Identity();

		float& operator () (unsigned int Row, unsigned int Col);
		float  operator () (unsigned int Row, unsigned int Col) const;

		operator float* ();
		operator const float* () const;
		operator DirectX::XMFLOAT4X4* ();
		operator const DirectX::XMFLOAT4X4* () const;

		MAT16& operator *= (const MAT16&);
		MAT16& operator += (const MAT16&);
		MAT16& operator -= (const MAT16&);
		MAT16& operator *= (float);
		MAT16& operator /= (float);

		MAT16 operator + () const;
		MAT16 operator - () const;

		MAT16 operator * (const MAT16&) const;
		MAT16 operator + (const MAT16&) const;
		MAT16 operator - (const MAT16&) const;
		MAT16 operator * (float) const;
		MAT16 operator / (float) const;

		friend MAT16 operator * (float, const MAT16&);

		bool operator == (const MAT16&) const;
		bool operator != (const MAT16&) const;


	public:
		float m[4][4] = {};
	};
}
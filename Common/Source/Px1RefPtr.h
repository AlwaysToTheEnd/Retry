#pragma once

namespace physx
{
	template<class T>
	class Px1RefPtr final
	{
	public:
		Px1RefPtr() = default;
		Px1RefPtr(T* instance)
		{
			ptr = instance;
		}

		~Px1RefPtr()
		{
			if (ptr)
			{
				ptr->release();
			}
		}

		T** GetAddressOf()
		{
			(*this) = reinterpret_cast<T*>(nullptr);
			return &ptr;
		}

		T* Get()
		{
			return ptr;
		}

		void operator=(T* rhs)
		{
			if (ptr)
			{
				ptr->release();
			}

			ptr = rhs;
		}

		T& operator*()
		{
			return *ptr;
		}

		void operator=(Px1RefPtr rhs) = delete;

		T* operator->()
		{
			return (this->ptr);
		}

	private:
		T* ptr = nullptr;
	};
}

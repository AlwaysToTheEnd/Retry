#pragma once

#include <assert.h>
#include <stdlib.h>

class ByteSwap
{
	ByteSwap() {}

public:

	// ----------------------------------------------------------------------
	/** Swap two bytes of data
	 *  @param[inout] _szOut A void* to save the reintcasts for the caller. */
	static inline void Swap2(void* _szOut)
	{
		assert(_szOut);

#if _MSC_VER >= 1400
		uint16_t* const szOut = reinterpret_cast<uint16_t*>(_szOut);
		*szOut = _byteswap_ushort(*szOut);
#else
		uint8_t* const szOut = reinterpret_cast<uint8_t*>(_szOut);
		std::swap(szOut[0],szOut[1]);
#endif
	}

	// ----------------------------------------------------------------------
	/** Swap four bytes of data
	 *  @param[inout] _szOut A void* to save the reintcasts for the caller. */
	static inline void Swap4(void* _szOut)
	{
		assert(_szOut);

#if _MSC_VER >= 1400
		uint32_t* const szOut = reinterpret_cast<uint32_t*>(_szOut);
		*szOut = _byteswap_ulong(*szOut);
#else
		uint8_t* const szOut = reinterpret_cast<uint8_t*>(_szOut);
		std::swap(szOut[0],szOut[3]);
		std::swap(szOut[1],szOut[2]);
#endif
	}

	// ----------------------------------------------------------------------
	/** Swap eight bytes of data
	 *  @param[inout] _szOut A void* to save the reintcasts for the caller. */
	static inline void Swap8(void* _szOut)
	{
		assert(_szOut);

#if _MSC_VER >= 1400
		uint64_t* const szOut = reinterpret_cast<uint64_t*>(_szOut);
		*szOut = _byteswap_uint64(*szOut);
#else
		uint8_t* const szOut = reinterpret_cast<uint8_t*>(_szOut);
		std::swap(szOut[0],szOut[7]);
		std::swap(szOut[1],szOut[6]);
		std::swap(szOut[2],szOut[5]);
		std::swap(szOut[3],szOut[4]);
#endif
	}

	// ----------------------------------------------------------------------
	/** ByteSwap a float. Not a joke.
	 *  @param[inout] fOut ehm. .. */
	static inline void Swap(float* fOut) {
		Swap4(fOut);
	}

	// ----------------------------------------------------------------------
	/** ByteSwap a double. Not a joke.
	 *  @param[inout] fOut ehm. .. */
	static inline void Swap(double* fOut) {
		Swap8(fOut);
	}


	// ----------------------------------------------------------------------
	/** ByteSwap an int16t. Not a joke.
	 *  @param[inout] fOut ehm. .. */
	static inline void Swap(int16_t* fOut) {
		Swap2(fOut);
	}

	static inline void Swap(uint16_t* fOut) {
		Swap2(fOut);
	}

	// ----------------------------------------------------------------------
	/** ByteSwap an int32t. Not a joke.
	 *  @param[inout] fOut ehm. .. */
	static inline void Swap(int32_t* fOut){
		Swap4(fOut);
	}

	static inline void Swap(uint32_t* fOut){
		Swap4(fOut);
	}

	// ----------------------------------------------------------------------
	/** ByteSwap an int64t. Not a joke.
	 *  @param[inout] fOut ehm. .. */
	static inline void Swap(int64_t* fOut) {
		Swap8(fOut);
	}

	static inline void Swap(uint64_t* fOut) {
		Swap8(fOut);
	}
};




// --------------------------------------------------------------------------------------
// ByteSwap macros for BigEndian/LittleEndian support 
// --------------------------------------------------------------------------------------
#if (defined AI_BUILD_BIG_ENDIAN)
#	define AI_LE(t)	(t)
#	define AI_LSWAP2(p)
#	define AI_LSWAP4(p)
#	define AI_LSWAP8(p)
#	define AI_LSWAP2P(p)
#	define AI_LSWAP4P(p)
#	define AI_LSWAP8P(p)
#	define LE_NCONST const
#	define AI_SWAP2(p) ByteSwap::Swap2(&(p))
#	define AI_SWAP4(p) ByteSwap::Swap4(&(p))
#	define AI_SWAP8(p) ByteSwap::Swap8(&(p))
#	define AI_SWAP2P(p) ByteSwap::Swap2((p))
#	define AI_SWAP4P(p) ByteSwap::Swap4((p))
#	define AI_SWAP8P(p) ByteSwap::Swap8((p))
#	define BE_NCONST
#else
#	define AI_BE(t)	(t)
#	define AI_SWAP2(p)
#	define AI_SWAP4(p)
#	define AI_SWAP8(p)
#	define AI_SWAP2P(p)
#	define AI_SWAP4P(p)
#	define AI_SWAP8P(p)
#	define BE_NCONST const
#	define AI_LSWAP2(p)		ByteSwap::Swap2(&(p))
#	define AI_LSWAP4(p)		ByteSwap::Swap4(&(p))
#	define AI_LSWAP8(p)		ByteSwap::Swap8(&(p))
#	define AI_LSWAP2P(p)	ByteSwap::Swap2((p))
#	define AI_LSWAP4P(p)	ByteSwap::Swap4((p))
#	define AI_LSWAP8P(p)	ByteSwap::Swap8((p))
#	define LE_NCONST
#endif
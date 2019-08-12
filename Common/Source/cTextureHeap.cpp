#include "cTextureHeap.h"
#include "d3dx12.h"
#include <wincodec.h>
#include <algorithm>

using namespace DirectX;
using namespace std;


namespace   /*this namesapce code is what need for TextureLoad of DirectXTK_Desktop sln,
			  and little bit revised for that will not use 'ResourceUploadBatch', by CGH*/
{
	//-------------------------------------------------------------------------------------
   // WIC Pixel Format Translation Data
   //-------------------------------------------------------------------------------------
	struct WICTranslate
	{
		GUID                wic;
		DXGI_FORMAT         format;
	};

	const WICTranslate g_WICFormats[] =
	{
		{ GUID_WICPixelFormat128bppRGBAFloat,       DXGI_FORMAT_R32G32B32A32_FLOAT },

		{ GUID_WICPixelFormat64bppRGBAHalf,         DXGI_FORMAT_R16G16B16A16_FLOAT },
		{ GUID_WICPixelFormat64bppRGBA,             DXGI_FORMAT_R16G16B16A16_UNORM },

		{ GUID_WICPixelFormat32bppRGBA,             DXGI_FORMAT_R8G8B8A8_UNORM },
		{ GUID_WICPixelFormat32bppBGRA,             DXGI_FORMAT_B8G8R8A8_UNORM },
		{ GUID_WICPixelFormat32bppBGR,              DXGI_FORMAT_B8G8R8X8_UNORM },

		{ GUID_WICPixelFormat32bppRGBA1010102XR,    DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM },
		{ GUID_WICPixelFormat32bppRGBA1010102,      DXGI_FORMAT_R10G10B10A2_UNORM },

		{ GUID_WICPixelFormat16bppBGRA5551,         DXGI_FORMAT_B5G5R5A1_UNORM },
		{ GUID_WICPixelFormat16bppBGR565,           DXGI_FORMAT_B5G6R5_UNORM },

		{ GUID_WICPixelFormat32bppGrayFloat,        DXGI_FORMAT_R32_FLOAT },
		{ GUID_WICPixelFormat16bppGrayHalf,         DXGI_FORMAT_R16_FLOAT },
		{ GUID_WICPixelFormat16bppGray,             DXGI_FORMAT_R16_UNORM },
		{ GUID_WICPixelFormat8bppGray,              DXGI_FORMAT_R8_UNORM },

		{ GUID_WICPixelFormat8bppAlpha,             DXGI_FORMAT_A8_UNORM },

		{ GUID_WICPixelFormat96bppRGBFloat,         DXGI_FORMAT_R32G32B32_FLOAT },
	};

	//-------------------------------------------------------------------------------------
	// WIC Pixel Format nearest conversion table
	//-------------------------------------------------------------------------------------

	struct WICConvert
	{
		GUID        source;
		GUID        target;
	};

	const WICConvert g_WICConvert[] =
	{
		// Note target GUID in this conversion table must be one of those directly supported formats (above).

		{ GUID_WICPixelFormatBlackWhite,            GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM

		{ GUID_WICPixelFormat1bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
		{ GUID_WICPixelFormat2bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
		{ GUID_WICPixelFormat4bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
		{ GUID_WICPixelFormat8bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 

		{ GUID_WICPixelFormat2bppGray,              GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM 
		{ GUID_WICPixelFormat4bppGray,              GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM 

		{ GUID_WICPixelFormat16bppGrayFixedPoint,   GUID_WICPixelFormat16bppGrayHalf }, // DXGI_FORMAT_R16_FLOAT 
		{ GUID_WICPixelFormat32bppGrayFixedPoint,   GUID_WICPixelFormat32bppGrayFloat }, // DXGI_FORMAT_R32_FLOAT 

		{ GUID_WICPixelFormat16bppBGR555,           GUID_WICPixelFormat16bppBGRA5551 }, // DXGI_FORMAT_B5G5R5A1_UNORM

		{ GUID_WICPixelFormat32bppBGR101010,        GUID_WICPixelFormat32bppRGBA1010102 }, // DXGI_FORMAT_R10G10B10A2_UNORM

		{ GUID_WICPixelFormat24bppBGR,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
		{ GUID_WICPixelFormat24bppRGB,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
		{ GUID_WICPixelFormat32bppPBGRA,            GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
		{ GUID_WICPixelFormat32bppPRGBA,            GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 

		{ GUID_WICPixelFormat48bppRGB,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat48bppBGR,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat64bppBGRA,             GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat64bppPRGBA,            GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat64bppPBGRA,            GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM

		{ GUID_WICPixelFormat48bppRGBFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
		{ GUID_WICPixelFormat48bppBGRFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
		{ GUID_WICPixelFormat64bppRGBAFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
		{ GUID_WICPixelFormat64bppBGRAFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
		{ GUID_WICPixelFormat64bppRGBFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
		{ GUID_WICPixelFormat64bppRGBHalf,          GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
		{ GUID_WICPixelFormat48bppRGBHalf,          GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 

		{ GUID_WICPixelFormat128bppPRGBAFloat,      GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 
		{ GUID_WICPixelFormat128bppRGBFloat,        GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 
		{ GUID_WICPixelFormat128bppRGBAFixedPoint,  GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 
		{ GUID_WICPixelFormat128bppRGBFixedPoint,   GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 
		{ GUID_WICPixelFormat32bppRGBE,             GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 

		{ GUID_WICPixelFormat32bppCMYK,             GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
		{ GUID_WICPixelFormat64bppCMYK,             GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat40bppCMYKAlpha,        GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
		{ GUID_WICPixelFormat80bppCMYKAlpha,        GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM

		{ GUID_WICPixelFormat32bppRGB,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
		{ GUID_WICPixelFormat64bppRGB,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat64bppPRGBAHalf,        GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT

		{ GUID_WICPixelFormat96bppRGBFixedPoint,   GUID_WICPixelFormat96bppRGBFloat }, // DXGI_FORMAT_R32G32B32_FLOAT

		// We don't support n-channel formats
	};

	/////////////////////////////

	//---------------------------------------------------------------------------------
	inline DXGI_FORMAT MakeSRGB(_In_ DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM:
			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

		case DXGI_FORMAT_BC1_UNORM:
			return DXGI_FORMAT_BC1_UNORM_SRGB;

		case DXGI_FORMAT_BC2_UNORM:
			return DXGI_FORMAT_BC2_UNORM_SRGB;

		case DXGI_FORMAT_BC3_UNORM:
			return DXGI_FORMAT_BC3_UNORM_SRGB;

		case DXGI_FORMAT_B8G8R8A8_UNORM:
			return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

		case DXGI_FORMAT_B8G8R8X8_UNORM:
			return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;

		case DXGI_FORMAT_BC7_UNORM:
			return DXGI_FORMAT_BC7_UNORM_SRGB;

		default:
			return format;
		}
	}

	//---------------------------------------------------------------------------------
	DXGI_FORMAT _WICToDXGI(const GUID& guid)
	{
		for (size_t i = 0; i < _countof(g_WICFormats); ++i)
		{
			if (memcmp(&g_WICFormats[i].wic, &guid, sizeof(GUID)) == 0)
				return g_WICFormats[i].format;
		}

		return DXGI_FORMAT_UNKNOWN;
	}

	//---------------------------------------------------------------------------------
	IWICImagingFactory2* GetWIC()
	{
		//create WICImagingFactory in where thread safe.
		IWICImagingFactory2* pWICFac = nullptr;
		static INIT_ONCE initOnce = INIT_ONCE_STATIC_INIT;

		InitOnceExecuteOnce(&initOnce,
			reinterpret_cast<PINIT_ONCE_FN>(&[](PINIT_ONCE, PVOID, PVOID* ifactory)->bool
				{
					return SUCCEEDED(CoCreateInstance(CLSID_WICImagingFactory2,
						nullptr, CLSCTX_INPROC_SERVER,
						__uuidof(IWICImagingFactory2), ifactory)) ? TRUE : FALSE;

				})
			, nullptr, reinterpret_cast<LPVOID*>(&pWICFac));

		return pWICFac;
	}

	//---------------------------------------------------------------------------------
	size_t _WICBitsPerPixel(REFGUID targetGuid)
	{
		auto pWIC = GetWIC();
		if (!pWIC)
			return 0;

		ComPtr<IWICComponentInfo> cinfo;
		if (FAILED(pWIC->CreateComponentInfo(targetGuid, cinfo.GetAddressOf())))
			return 0;

		WICComponentType type;
		if (FAILED(cinfo->GetComponentType(&type)))
			return 0;

		if (type != WICPixelFormat)
			return 0;

		ComPtr<IWICPixelFormatInfo> pfinfo;
		if (FAILED(cinfo.As(&pfinfo)))
			return 0;

		UINT bpp;
		if (FAILED(pfinfo->GetBitsPerPixel(&bpp)))
			return 0;

		return bpp;
	}

	//---------------------------------------------------------------------------------
	uint32_t CountMips(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0)
			return 0;

		uint32_t count = 1;
		while (width > 1 || height > 1)
		{
			width >>= 1;
			height >>= 1;
			count++;
		}
		return count;
	}

	//---------------------------------------------------------------------------------
	bool FormatIsUAVCompatible(_In_ ID3D12Device* device, bool typedUAVLoadAdditionalFormat, DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_R32_FLOAT:
		case DXGI_FORMAT_R32_UINT:
		case DXGI_FORMAT_R32_SINT:
			// Unconditionally supported.
			return true;

		case DXGI_FORMAT_R32G32B32A32_FLOAT:
		case DXGI_FORMAT_R32G32B32A32_UINT:
		case DXGI_FORMAT_R32G32B32A32_SINT:
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
		case DXGI_FORMAT_R16G16B16A16_UINT:
		case DXGI_FORMAT_R16G16B16A16_SINT:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_R8G8B8A8_SINT:
		case DXGI_FORMAT_R16_FLOAT:
		case DXGI_FORMAT_R16_UINT:
		case DXGI_FORMAT_R16_SINT:
		case DXGI_FORMAT_R8_UNORM:
		case DXGI_FORMAT_R8_UINT:
		case DXGI_FORMAT_R8_SINT:
			// All these are supported if this optional feature is set.
			return typedUAVLoadAdditionalFormat;

		case DXGI_FORMAT_R16G16B16A16_UNORM:
		case DXGI_FORMAT_R16G16B16A16_SNORM:
		case DXGI_FORMAT_R32G32_FLOAT:
		case DXGI_FORMAT_R32G32_UINT:
		case DXGI_FORMAT_R32G32_SINT:
		case DXGI_FORMAT_R10G10B10A2_UNORM:
		case DXGI_FORMAT_R10G10B10A2_UINT:
		case DXGI_FORMAT_R11G11B10_FLOAT:
		case DXGI_FORMAT_R8G8B8A8_SNORM:
		case DXGI_FORMAT_R16G16_FLOAT:
		case DXGI_FORMAT_R16G16_UNORM:
		case DXGI_FORMAT_R16G16_UINT:
		case DXGI_FORMAT_R16G16_SNORM:
		case DXGI_FORMAT_R16G16_SINT:
		case DXGI_FORMAT_R8G8_UNORM:
		case DXGI_FORMAT_R8G8_UINT:
		case DXGI_FORMAT_R8G8_SNORM:
		case DXGI_FORMAT_R8G8_SINT:
		case DXGI_FORMAT_R16_UNORM:
		case DXGI_FORMAT_R16_SNORM:
		case DXGI_FORMAT_R8_SNORM:
		case DXGI_FORMAT_A8_UNORM:
		case DXGI_FORMAT_B5G6R5_UNORM:
		case DXGI_FORMAT_B5G5R5A1_UNORM:
		case DXGI_FORMAT_B4G4R4A4_UNORM:
			// Conditionally supported by specific devices.
			if (typedUAVLoadAdditionalFormat)
			{
				D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = { format, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE };
				if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport))))
				{
					const DWORD mask = D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD | D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE;
					return ((formatSupport.Support2 & mask) == mask);
				}
			}
			return false;

		default:
			return false;
		}
	}

	bool FormatIsBGR(DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			return true;
		default:
			return false;
		}
	}

	bool FormatIsSRGB(DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			return true;
		default:
			return false;
		}
	}

	//---------------------------------------------------------------------------------
	bool IsSupportedForGenerateMips(_In_ ID3D12Device* device, DXGI_FORMAT format)
	{
		D3D12_FEATURE_DATA_D3D12_OPTIONS option = {};

		if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS,
			&option, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS))));

		if (FormatIsUAVCompatible(device, option.TypedUAVLoadAdditionalFormats, format))
			return true;

		if (FormatIsBGR(format))
		{
#if defined(_XBOX_ONE) && defined(_TITLE)
			// We know the RGB and BGR memory layouts match for Xbox One
			return true;
#else
			// BGR path requires DXGI_FORMAT_R8G8B8A8_UNORM support for UAV load/store plus matching layouts
			return option.TypedUAVLoadAdditionalFormats && option.StandardSwizzle64KBSupported;
#endif
		}

		if (FormatIsSRGB(format))
		{
			// sRGB path requires DXGI_FORMAT_R8G8B8A8_UNORM support for UAV load/store
			return option.TypedUAVLoadAdditionalFormats;
		}

		return false;
	}


	//---------------------------------------------------------------------------------
	HRESULT CreateTextureFromWIC(_In_ ID3D12Device* d3dDevice,
		_In_ IWICBitmapFrameDecode* frame,
		size_t maxsize,
		D3D12_RESOURCE_FLAGS resFlags,
		unsigned int loadFlags,
		_Outptr_ ID3D12Resource** texture,
		std::unique_ptr<uint8_t[]>& decodedData,
		D3D12_SUBRESOURCE_DATA& subresource)
	{
		UINT width, height;
		HRESULT hr = frame->GetSize(&width, &height);
		if (FAILED(hr))
			return hr;

		assert(width > 0 && height > 0);

		if (maxsize > UINT32_MAX)
			return E_INVALIDARG;

		if (!maxsize)
		{
			maxsize = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
		}

		UINT twidth, theight;
		if (width > maxsize || height > maxsize)
		{
			float ar = static_cast<float>(height) / static_cast<float>(width);
			if (width > height)
			{
				twidth = static_cast<UINT>(maxsize);
				theight = std::max<UINT>(1, static_cast<UINT>(static_cast<float>(maxsize) * ar));
			}
			else
			{
				theight = static_cast<UINT>(maxsize);
				twidth = std::max<UINT>(1, static_cast<UINT>(static_cast<float>(maxsize) / ar));
			}
			assert(twidth <= maxsize && theight <= maxsize);
		}
		else
		{
			twidth = width;
			theight = height;
		}

		// Determine format
		WICPixelFormatGUID pixelFormat;
		hr = frame->GetPixelFormat(&pixelFormat);
		if (FAILED(hr))
			return hr;

		WICPixelFormatGUID convertGUID;
		memcpy_s(&convertGUID, sizeof(WICPixelFormatGUID), &pixelFormat, sizeof(GUID));

		size_t bpp = 0;

		DXGI_FORMAT format = _WICToDXGI(pixelFormat);
		if (format == DXGI_FORMAT_UNKNOWN)
		{
			for (size_t i = 0; i < _countof(g_WICConvert); ++i)
			{
				if (memcmp(&g_WICConvert[i].source, &pixelFormat, sizeof(WICPixelFormatGUID)) == 0)
				{
					memcpy_s(&convertGUID, sizeof(WICPixelFormatGUID), &g_WICConvert[i].target, sizeof(GUID));

					format = _WICToDXGI(g_WICConvert[i].target);
					assert(format != DXGI_FORMAT_UNKNOWN);
					bpp = _WICBitsPerPixel(convertGUID);
					break;
				}
			}

			if (format == DXGI_FORMAT_UNKNOWN)
			{
				DebugTrace("ERROR: WICTextureLoader does not support all DXGI formats (WIC GUID {%8.8lX-%4.4X-%4.4X-%2.2X%2.2X-%2.2X%2.2X%2.2X%2.2X%2.2X%2.2X}). Consider using DirectXTex.\n",
					pixelFormat.Data1, pixelFormat.Data2, pixelFormat.Data3,
					pixelFormat.Data4[0], pixelFormat.Data4[1], pixelFormat.Data4[2], pixelFormat.Data4[3],
					pixelFormat.Data4[4], pixelFormat.Data4[5], pixelFormat.Data4[6], pixelFormat.Data4[7]);
				return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
			}
		}
		else
		{
			bpp = _WICBitsPerPixel(pixelFormat);
		}

		if (!bpp)
			return E_FAIL;

		// Handle sRGB formats
		if (loadFlags & WIC_LOADER_FORCE_SRGB)
		{
			format = MakeSRGB(format);
		}
		else if (!(loadFlags & WIC_LOADER_IGNORE_SRGB))
		{
			ComPtr<IWICMetadataQueryReader> metareader;
			if (SUCCEEDED(frame->GetMetadataQueryReader(metareader.GetAddressOf())))
			{
				GUID containerFormat;
				if (SUCCEEDED(metareader->GetContainerFormat(&containerFormat)))
				{
					// Check for sRGB colorspace metadata
					bool sRGB = false;

					PROPVARIANT value;
					PropVariantInit(&value);

					if (memcmp(&containerFormat, &GUID_ContainerFormatPng, sizeof(GUID)) == 0)
					{
						// Check for sRGB chunk
						if (SUCCEEDED(metareader->GetMetadataByName(L"/sRGB/RenderingIntent", &value)) && value.vt == VT_UI1)
						{
							sRGB = true;
						}
					}
#if defined(_XBOX_ONE) && defined(_TITLE)
					else if (memcmp(&containerFormat, &GUID_ContainerFormatJpeg, sizeof(GUID)) == 0)
					{
						if (SUCCEEDED(metareader->GetMetadataByName(L"/app1/ifd/exif/{ushort=40961}", &value)) && value.vt == VT_UI2 && value.uiVal == 1)
						{
							sRGB = true;
						}
					}
					else if (memcmp(&containerFormat, &GUID_ContainerFormatTiff, sizeof(GUID)) == 0)
					{
						if (SUCCEEDED(metareader->GetMetadataByName(L"/ifd/exif/{ushort=40961}", &value)) && value.vt == VT_UI2 && value.uiVal == 1)
						{
							sRGB = true;
						}
					}
#else
					else if (SUCCEEDED(metareader->GetMetadataByName(L"System.Image.ColorSpace", &value)) && value.vt == VT_UI2 && value.uiVal == 1)
					{
						sRGB = true;
					}
#endif

					(void)PropVariantClear(&value);

					if (sRGB)
						format = MakeSRGB(format);
				}
			}
		}

		// Allocate memory for decoded image
		uint64_t rowBytes = (uint64_t(twidth) * uint64_t(bpp) + 7u) / 8u;
		uint64_t numBytes = rowBytes * uint64_t(height);

		if (rowBytes > UINT32_MAX || numBytes > UINT32_MAX)
			return HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);

		auto rowPitch = static_cast<size_t>(rowBytes);
		auto imageSize = static_cast<size_t>(numBytes);

		decodedData.reset(new (std::nothrow) uint8_t[imageSize]);
		if (!decodedData)
			return E_OUTOFMEMORY;

		// Load image data
		if (memcmp(&convertGUID, &pixelFormat, sizeof(GUID)) == 0
			&& twidth == width
			&& theight == height)
		{
			// No format conversion or resize needed
			hr = frame->CopyPixels(nullptr, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), decodedData.get());
			if (FAILED(hr))
				return hr;
		}
		else if (twidth != width || theight != height)
		{
			// Resize
			auto pWIC = GetWIC();
			if (!pWIC)
				return E_NOINTERFACE;

			ComPtr<IWICBitmapScaler> scaler;
			hr = pWIC->CreateBitmapScaler(scaler.GetAddressOf());
			if (FAILED(hr))
				return hr;

			hr = scaler->Initialize(frame, twidth, theight, WICBitmapInterpolationModeFant);
			if (FAILED(hr))
				return hr;

			WICPixelFormatGUID pfScaler;
			hr = scaler->GetPixelFormat(&pfScaler);
			if (FAILED(hr))
				return hr;

			if (memcmp(&convertGUID, &pfScaler, sizeof(GUID)) == 0)
			{
				// No format conversion needed
				hr = scaler->CopyPixels(nullptr, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), decodedData.get());
				if (FAILED(hr))
					return hr;
			}
			else
			{
				ComPtr<IWICFormatConverter> FC;
				hr = pWIC->CreateFormatConverter(FC.GetAddressOf());
				if (FAILED(hr))
					return hr;

				BOOL canConvert = FALSE;
				hr = FC->CanConvert(pfScaler, convertGUID, &canConvert);
				if (FAILED(hr) || !canConvert)
				{
					return E_UNEXPECTED;
				}

				hr = FC->Initialize(scaler.Get(), convertGUID, WICBitmapDitherTypeErrorDiffusion, nullptr, 0, WICBitmapPaletteTypeMedianCut);
				if (FAILED(hr))
					return hr;

				hr = FC->CopyPixels(nullptr, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), decodedData.get());
				if (FAILED(hr))
					return hr;
			}
		}
		else
		{
			// Format conversion but no resize
			auto pWIC = GetWIC();
			if (!pWIC)
				return E_NOINTERFACE;

			ComPtr<IWICFormatConverter> FC;
			hr = pWIC->CreateFormatConverter(FC.GetAddressOf());
			if (FAILED(hr))
				return hr;

			BOOL canConvert = FALSE;
			hr = FC->CanConvert(pixelFormat, convertGUID, &canConvert);
			if (FAILED(hr) || !canConvert)
			{
				return E_UNEXPECTED;
			}

			hr = FC->Initialize(frame, convertGUID, WICBitmapDitherTypeErrorDiffusion, nullptr, 0, WICBitmapPaletteTypeMedianCut);
			if (FAILED(hr))
				return hr;

			hr = FC->CopyPixels(nullptr, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), decodedData.get());
			if (FAILED(hr))
				return hr;
		}

		// Count the number of mips
		uint32_t mipCount = (loadFlags & (WIC_LOADER_MIP_AUTOGEN | WIC_LOADER_MIP_RESERVE)) ? CountMips(twidth, theight) : 1;

		// Create texture
		D3D12_RESOURCE_DESC desc = {};
		desc.Width = twidth;
		desc.Height = theight;
		desc.MipLevels = static_cast<UINT16>(mipCount);
		desc.DepthOrArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Flags = resFlags;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

		CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);

		ID3D12Resource* tex = nullptr;
		hr = d3dDevice->CreateCommittedResource(
			&defaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&tex));

		if (FAILED(hr))
		{
			return hr;
		}

		_Analysis_assume_(tex != nullptr);

		subresource.pData = decodedData.get();
		subresource.RowPitch = static_cast<LONG>(rowPitch);
		subresource.SlicePitch = static_cast<LONG>(imageSize);

		*texture = tex;
		return hr;
	}

	//--------------------------------------------------------------------------------------
	void SetDebugTextureInfo(
		_In_z_ const wchar_t* fileName,
		_In_ ID3D12Resource** texture)
	{
#if !defined(NO_D3D12_DEBUG_NAME) && ( defined(_DEBUG) || defined(PROFILE) )
		if (texture && *texture)
		{
			const wchar_t* pstrName = wcsrchr(fileName, '\\');
			if (!pstrName)
			{
				pstrName = fileName;
			}
			else
			{
				pstrName++;
			}

			(*texture)->SetName(pstrName);
		}
#else
		UNREFERENCED_PARAMETER(fileName);
		UNREFERENCED_PARAMETER(texture);
#endif
	}

	//--------------------------------------------------------------------------------------
	DXGI_FORMAT GetPixelFormat(_In_ IWICBitmapFrameDecode* frame)
	{
		WICPixelFormatGUID pixelFormat;
		if (FAILED(frame->GetPixelFormat(&pixelFormat)))
			return DXGI_FORMAT_UNKNOWN;

		DXGI_FORMAT format = _WICToDXGI(pixelFormat);
		if (format == DXGI_FORMAT_UNKNOWN)
		{
			for (size_t i = 0; i < _countof(g_WICConvert); ++i)
			{
				if (memcmp(&g_WICConvert[i].source, &pixelFormat, sizeof(WICPixelFormatGUID)) == 0)
				{
					return _WICToDXGI(g_WICConvert[i].target);
				}
			}
		}

		return format;
	}
}

cTextureHeap::cTextureHeap(ID3D12Device* device, UINT maxTexture)
	: m_SrvDescriptorSize(0)
	, m_isBeging(false)
{
	assert(device && maxTexture);

	m_SrvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvHeapDesc.NumDescriptors = maxTexture;

	ThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(m_SrvHeap.GetAddressOf())));
}

void cTextureHeap::Begin(ID3D12Device* device)
{
	assert(!m_isBeging && "TextureHeap aleady beging state");

	ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(m_comAlloc.GetAddressOf())));

	ThrowIfFailed(device->CreateCommandList(1, D3D12_COMMAND_LIST_TYPE_DIRECT, m_comAlloc.Get(),
		nullptr, IID_PPV_ARGS(m_commandList.GetAddressOf())));

	m_isBeging = true;
}

void cTextureHeap::AddTexture(ID3D12Device* device, ID3D12CommandQueue* cmdqueue, const string& name, const wstring& filename)
{
	auto it = m_Textures.find(name);
	assert(it == m_Textures.end() && "This name is overlapping name");

	TEXTURENUM addedTexture;
	addedTexture.num = (UINT)m_Textures.size();
	addedTexture.tex.name = name;

	if (IsDDSTextureFile(filename))
	{

	}
	else
	{

	}

	auto srvHeapHandle = (CD3DX12_CPU_DESCRIPTOR_HANDLE)m_SrvHeap->GetCPUDescriptorHandleForHeapStart();
	srvHeapHandle.Offset(addedTexture.num, m_SrvDescriptorSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = addedTexture.tex.resource->GetDesc().Format;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.MipLevels = addedTexture.tex.resource->GetDesc().MipLevels;

	device->CreateShaderResourceView(addedTexture.tex.resource.Get(), &srvDesc, srvHeapHandle);
}

void cTextureHeap::AddCubeMapTexture(ID3D12Device* device, ID3D12CommandQueue* cmdqueue, const string& name, const wstring& filename)
{
	auto it = m_Textures.find(name);
	assert(it == m_Textures.end() && "This name is overlapping name");

	TEXTURENUM addedTexture;
	addedTexture.num = (UINT)m_Textures.size();
	addedTexture.tex.name = name;

	if (IsDDSTextureFile(filename))
	{

	}
	else
	{
		assert(false && "Cube Texture is supported only dds extension");
	}

	auto srvHeapHandle = (CD3DX12_CPU_DESCRIPTOR_HANDLE)m_SrvHeap->GetCPUDescriptorHandleForHeapStart();
	srvHeapHandle.Offset(addedTexture.num, m_SrvDescriptorSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Format = addedTexture.tex.resource->GetDesc().Format;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.MipLevels = addedTexture.tex.resource->GetDesc().MipLevels;

	device->CreateShaderResourceView(addedTexture.tex.resource.Get(), &srvDesc, srvHeapHandle);
}

void cTextureHeap::AddNullTexture(ID3D12Device* device, const string& name, DXGI_FORMAT srvFormat,
	const D3D12_RESOURCE_DESC* resourceDesc, const D3D12_CLEAR_VALUE* optClear)
{
	auto it = m_Textures.find(name);
	assert(it == m_Textures.end() && "This name is overlapping name");

	TEXTURENUM addTexture;
	addTexture.num = (UINT)m_Textures.size();
	addTexture.tex.name = name;

	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE, resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, optClear,
		IID_PPV_ARGS(addTexture.tex.resource.GetAddressOf()));

	m_Textures[name] = addTexture;

	auto srvHeapHandle = (CD3DX12_CPU_DESCRIPTOR_HANDLE)m_SrvHeap->GetCPUDescriptorHandleForHeapStart();
	srvHeapHandle.Offset(addTexture.num, m_SrvDescriptorSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = srvFormat;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.PlaneSlice = 0;

	device->CreateShaderResourceView(m_Textures[name].tex.resource.Get(), &srvDesc, srvHeapHandle);
}

void cTextureHeap::End(ID3D12CommandQueue* queue, void(*flushCommandQueueFunc)())
{
	assert(m_isBeging && "TextureHeap already closed");

	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* commandLists[] = { m_commandList.Get() };
	queue->ExecuteCommandLists(1, commandLists);

	flushCommandQueueFunc();

	m_commandList.Reset();
	m_comAlloc.Reset();
}

ComPtr<ID3D12Resource> cTextureHeap::GetTexture(const string& name)
{
	auto it = m_Textures.find(name);
	assert(it != m_Textures.end() && "can not find this name");

	return it->second.tex.resource;
}

UINT cTextureHeap::GetTextureIndex(const string& name) const
{
	auto it = m_Textures.find(name);
	assert(it != m_Textures.end() && "can not find this name");

	return it->second.num;
}

HRESULT cTextureHeap::LoadWICTexture(ID3D12Device* device, const std::wstring& filename, size_t maxsize,
	D3D12_RESOURCE_FLAGS resFlags, WIC_LOADER_FLAGS loadflags, ID3D12Resource** texture)
{
	assert(m_isBeging && "can not call to create texture on a closed TextureHeap");

	IWICImagingFactory2* pWICFac = GetWIC();

	ComPtr<IWICBitmapDecoder> decoder;
	HRESULT hr = pWICFac->CreateDecoderFromFilename(filename.c_str(), nullptr,
		GENERIC_READ, WICDecodeOptions::WICDecodeMetadataCacheOnDemand, decoder.GetAddressOf());
	if (FAILED(hr)) return hr;

	ComPtr<IWICBitmapFrameDecode> frame;
	hr = decoder->GetFrame(0, frame.GetAddressOf());
	if (FAILED(hr)) return hr;

	if (loadflags & WIC_LOADER_MIP_AUTOGEN)
	{
		DXGI_FORMAT fmt = GetPixelFormat(frame.Get());
		if (!IsSupportedForGenerateMips(device, fmt))
		{
			DebugTrace("WARNING: This device does not support autogen mips for this format (%d)\n", static_cast<int>(fmt));
			uint32_t currflag = loadflags;
			currflag &= ~WIC_LOADER_MIP_AUTOGEN;
			loadflags = (WIC_LOADER_FLAGS)currflag;
		}
	}

	std::unique_ptr<uint8_t[]> decodedData;
	D3D12_SUBRESOURCE_DATA initData;
	hr = CreateTextureFromWIC(device, frame.Get(), maxsize,
		resFlags, loadflags,
		texture, decodedData, initData);

	if (SUCCEEDED(hr))
	{
		SetDebugTextureInfo(filename.c_str(), texture);

		UINT64 uploadSize = GetRequiredIntermediateSize(*texture, 0, 1);

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadSize);

		// Create a temporary buffer
		ComPtr<ID3D12Resource> scratchResource = nullptr;
		ThrowIfFailed(device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, // D3D12_CLEAR_VALUE* pOptimizedClearValue
			IID_PPV_ARGS(scratchResource.GetAddressOf())));

		// Submit resource copy to command list
		UpdateSubresources(mList.Get(), resource, scratchResource.Get(), 0, subresourceIndexStart, numSubresources, subRes);

		// Remember this upload object for delayed release
		resourceUpload.Transition(
			*texture,
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		// Generate mips?
		if (loadflags & WIC_LOADER_MIP_AUTOGEN)
		{
			resourceUpload.GenerateMips(*texture);
		}
	}

	return hr;
}

bool cTextureHeap::IsDDSTextureFile(const std::wstring& filename)
{
	size_t index = filename.find('.') + 1;
	wstring extension;
	extension.assign(&filename[index], filename.size() - index);

	return extension == L"dds";
}

#include "IGraphicDevice.h"

void IGraphicDevice::ReservedWorksClear()
{
	m_ReservedAniBones.clear();
	m_ReservedRenderInfos.clear();
	m_ReservedFonts.clear();
}

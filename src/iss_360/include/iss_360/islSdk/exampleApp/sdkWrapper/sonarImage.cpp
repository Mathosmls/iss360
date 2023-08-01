//------------------------------------------ Includes ----------------------------------------------

#include "sonarImage.h"
#include "platform/mem.h"

using namespace IslSdk;

//--------------------------------------------------------------------------------------------------
Palette::Palette()
{
	m_palette = islSdkPaletteCreate();
}
//--------------------------------------------------------------------------------------------------
Palette::~Palette()
{
	if (m_palette != nullptr)
	{
		islSdkPaletteDelete(m_palette);
	}
}
//--------------------------------------------------------------------------------------------------
void Palette::setToDefault()
{
	const real_t PALETTE_SIZE = 65536;
	gradientValue_t palette[5];

	palette[0].colour = 0xff000000;
	palette[0].position = (uint16_t)((PALETTE_SIZE - 1) * 0);
	palette[1].colour = 0xff205aab;
	palette[1].position = (uint16_t)((PALETTE_SIZE - 1) * 0.22);
	palette[2].colour = 0xff27ce3d;
	palette[2].position = (uint16_t)((PALETTE_SIZE - 1) * 0.4);
	palette[3].colour = 0xffc2ce23;
	palette[3].position = (uint16_t)((PALETTE_SIZE - 1) * 0.62);
	palette[4].colour = 0xffce1200;
	palette[4].position = (uint16_t)((PALETTE_SIZE - 1) * 1.0);

	set(&palette[0], 5, 0xff000000);
}
//--------------------------------------------------------------------------------------------------
void Palette::set(const gradientValue_t* gradientList, uint32_t count, uint32_t nullColour)
{
	if (gradientList != nullptr)
	{
		islSdkPaletteSet(m_palette, gradientList, count, nullColour);
	}
}
//--------------------------------------------------------------------------------------------------
void Palette::render(uint32_t* buf, uint32_t width, uint32_t height, bool_t horizontal)
{
	islSdkPaletteRender(m_palette, buf, width, height, horizontal);
}
//==================================================================================================
SonarImageBuf::SonarImageBuf()
{
	m_refresh = false;
	m_buf.mem = nullptr;
	m_buf.width = 0;
	m_buf.height = 0;
	m_buf.size = 0;
	m_buf.area.minRangeMm = 0;
	m_buf.area.maxRangeMm = 0;
	m_buf.area.sector.start = 0;
	m_buf.area.sector.size = 0;
	m_buf.useBilinerInterpolation = true;
	m_buf.use32BitBuffer = true;
}
//--------------------------------------------------------------------------------------------------
SonarImageBuf::~SonarImageBuf()
{
	if (m_buf.mem != nullptr)
	{
		memFree(m_buf.mem);
	}
}
//--------------------------------------------------------------------------------------------------
void SonarImageBuf::setBuffer(uint32_t width, uint32_t height, bool_t use32BitsPerPixel)
{
	if (m_buf.width != width || m_buf.height != height || m_buf.use32BitBuffer != use32BitsPerPixel)
	{
		m_buf.width = width;
		m_buf.height = height;
		m_buf.use32BitBuffer = use32BitsPerPixel;
		m_buf.size = width * height * 2;
		m_refresh = true;

		if (use32BitsPerPixel)
		{
			m_buf.size *= 2;
		}

		uint8_t* mem = (uint8_t*)memRealloc(m_buf.mem, m_buf.size);

		if (mem)
		{
			m_buf.mem = mem;
		}
		else
		{
			free(m_buf.mem);
			m_buf.width = 0;
			m_buf.height = 0;
			m_buf.size = 0;
		}
	}
}
//--------------------------------------------------------------------------------------------------
void SonarImageBuf::setSectorArea(uint32_t minRangeMm, uint32_t maxRangeMm, uint32_t sectorStart, uint32_t sectorSize)
{
	if (sectorSize == 0)
	{
		sectorSize = 12800;
	}
	if (m_buf.area.minRangeMm != minRangeMm || m_buf.area.maxRangeMm != maxRangeMm || m_buf.area.sector.start != sectorStart || m_buf.area.sector.size != sectorSize)
	{
		m_buf.area.minRangeMm = minRangeMm;
		m_buf.area.maxRangeMm = maxRangeMm;
		m_buf.area.sector.start = sectorStart;
		m_buf.area.sector.size = sectorSize;
		m_refresh = true;
	}
}
//--------------------------------------------------------------------------------------------------
SonarImage::SonarImage()
{
	m_image = islSdkSonarImageCreate();
}
//--------------------------------------------------------------------------------------------------
SonarImage::~SonarImage()
{
	islSdkSonarImageDelete(m_image);
}
//--------------------------------------------------------------------------------------------------
void SonarImage::addData(iss360Ping_t& data, uint32_t txPulseLengthMm)
{
	islSdkSonarImageAddPingData(m_image, &data, txPulseLengthMm);
}
//--------------------------------------------------------------------------------------------------
void SonarImage::clear()
{
	islSdkSonarImageClearSection(m_image, 0, 12800);
}
//--------------------------------------------------------------------------------------------------
void SonarImage::clear(uint32_t sectorStart, uint32_t sectorSize)
{
	islSdkSonarImageClearSection(m_image, sectorStart, sectorSize);
}
//--------------------------------------------------------------------------------------------------
void SonarImage::setDefaultStepSize(int32_t stepSize)
{
	islSdkSonarImageSetDefaultStepSize(m_image, stepSize);
}
//--------------------------------------------------------------------------------------------------
void SonarImage::render(SonarImageBuf& imageBuf, Palette* palette, bool_t reDraw)
{
	islSdkpalette_t* pal = nullptr;

	if (imageBuf.m_buf.mem)
	{
		if (palette != nullptr)
		{
			pal = palette->m_palette;
		}

		if (imageBuf.m_refresh || reDraw)
		{
			imageBuf.m_refresh = false;
			islSdkSonarImageRender(m_image, &imageBuf.m_buf, pal, &imageBuf.m_buf.area);
		}
		else
		{
			islSdkSonarImageRender(m_image, &imageBuf.m_buf, pal, nullptr);
		}
	}
}
//--------------------------------------------------------------------------------------------------
void SonarImage::renderTexture(SonarImageBuf& imageBuf, Palette* palette, pixelArea_t* updatedArea, bool_t reDraw)
{
	islSdkpalette_t* pal = nullptr;

	if (imageBuf.m_buf.mem)
	{
		if (palette != nullptr)
		{
			pal = palette->m_palette;
		}

		if (imageBuf.m_refresh || reDraw)
		{
			imageBuf.m_refresh = false;
			islSdkSonarImageRenderTexture(m_image, &imageBuf.m_buf, pal, &imageBuf.m_buf.area, updatedArea);
		}
		else
		{
			islSdkSonarImageRenderTexture(m_image, &imageBuf.m_buf, pal, nullptr, updatedArea);
		}
	}
}
//--------------------------------------------------------------------------------------------------

#ifndef SONARIMAGECPP_H_
#define SONARIMAGECPP_H_

//------------------------------------------ Includes ----------------------------------------------

#include "helpers/sonarImage.h"

//--------------------------------------- Class Definition -----------------------------------------

namespace IslSdk
{
	class Palette
	{
	private:	
		islSdkpalette_t* m_palette;

	public:
		Palette();
		~Palette();
		void setToDefault();
		void set(const gradientValue_t* gradientList, uint32_t count, uint32_t nullColour);
		void render(uint32_t* buf, uint32_t width, uint32_t height, bool_t horizontal);

		friend class SonarImage;
	};

	class SonarImageBuf
	{
	private:
		sonarImageBuf_t m_buf;
		bool_t m_refresh;

	public:
		const sonarImageBuf_t& buf = m_buf;
		SonarImageBuf();
		~SonarImageBuf();
		void setBuffer(uint32_t width, uint32_t height, bool_t use32BitsPerPixel);
		void setSectorArea(uint32_t minRangeMm, uint32_t maxRangeMm, uint32_t sectorStart, uint32_t sectorSize);
		void setBilinerInterpolation(bool_t state) { m_buf.useBilinerInterpolation = state; }

		friend class SonarImage;
	};

	class SonarImage
	{
	private:
		islSdkSonarImage_t* m_image;

	public:
		SonarImage();
		~SonarImage();
		void addData(iss360Ping_t& data, uint32_t txPulseBlankMm);
		void clear();
		void clear(uint32_t sectorStart, uint32_t sectorSize);
		void setDefaultStepSize(int32_t stepSize);
		void render(SonarImageBuf& buf, Palette* palette, bool_t reDraw = false);
		void renderTexture(SonarImageBuf& buf, Palette* palette, pixelArea_t* updatedArea = nullptr, bool_t reDraw = false);
	};
}

//--------------------------------------------------------------------------------------------------
#endif

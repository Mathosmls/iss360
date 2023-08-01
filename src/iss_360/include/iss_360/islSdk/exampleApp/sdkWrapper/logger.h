#ifndef LOGGER_H_
#define LOGGER_H_

//------------------------------------------ Includes ----------------------------------------------

#include "islSdkLogging.h"
#include "sigSlot.h"
#include <vector>
#include <string>

//--------------------------------------- Class Definition -----------------------------------------

namespace IslSdk
{
	class Logger
	{
	private:
		uint32_t m_id;
		uint32_t m_totalRecordCount;
		uint64_t m_startTime;
		uint32_t m_totalDurationMs;
		uint32_t m_trackCount;
		logTrackInfo_t m_tracks[32];
		uint32_t m_maxFileSize;
		bool_t m_ready;
		std::string m_filename;

	public:
		const uint32_t& id = m_id;
		const bool_t& ready = m_ready;
		
		Signal<Logger&, uint32_t, const utf8_t*> onError;
		Signal<Logger&> onFileOpen;
		Signal<Logger&, const utf8_t*> onNewFile;
		Signal<Logger&> onLogFileMaxSize;
		Signal<Logger&, const logDataRecord_t&, bool_t, bool_t> onLogPlayback;

		static Logger* findById(uint32_t id);
		static Logger* findByFilename(const str_t* filename, uint32_t id);
		static void deleteAll();
		Logger(const str_t* filename, bool_t open);
		~Logger();
		void handleEvent(uint64_t timeMs, uint32_t eventId, const void* data);
		bool_t getTrackInfo(uint32_t trackId, logTrackInfo_t& logTrackInfo);
		void startNewFile(uint32_t maxSize);
		uint32_t addTrack(uint32_t appId, uint32_t partNumber, uint16_t pid);
		void addData(uint32_t trackId, bool_t isMeta, bool_t canSkip, uint32_t dataType, const void* data, uint32_t dataSize);
		void play(real_t speed);
		void seek(uint32_t index);
		void getInfo();
	};
}

//--------------------------------------------------------------------------------------------------

#endif

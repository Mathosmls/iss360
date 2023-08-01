//------------------------------------------ Includes ----------------------------------------------

#include "logger.h"
#include "device.h"
#include "helpers/logHelper.h"
#include "platform/mem.h"

using namespace IslSdk;

//------------------------------------------- Globals ----------------------------------------------

static std::vector<Logger*> loggerList;

//--------------------------------------------------------------------------------------------------
Logger* Logger::findById(uint32_t id)
{
	Logger* logger;

	logger = nullptr;
	for (size_t i = 0; i < loggerList.size(); i++)
	{
		if (loggerList[i]->m_id == id)
		{
			logger = loggerList[i];
			break;
		}
	}

	return logger;
}
//--------------------------------------------------------------------------------------------------
Logger* Logger::findByFilename(const str_t* filename, uint32_t id)
{
	Logger* logger;

	logger = nullptr;
	for (size_t i = 0; i < loggerList.size(); i++)
	{
		if (loggerList[i]->m_filename == filename)
		{
			logger = loggerList[i];
			if (logger->m_id == 0)
			{
				logger->m_id = id;
			}
			break;
		}
	}

	return logger;
}
//--------------------------------------------------------------------------------------------------
void Logger::deleteAll()
{
	while (loggerList.size())
	{
		delete loggerList[0];
	}
}
//--------------------------------------------------------------------------------------------------
Logger::Logger(const str_t* filename, bool_t open)
{
	m_id = 0;
	m_ready = false;
	m_totalRecordCount = 0;
	m_startTime = 0;
	m_totalDurationMs = 0;
	m_trackCount = 0;
	m_maxFileSize = 1024 * 1024 * 2000;
	m_filename.assign((const char*)filename);
	memSet(m_tracks, 0, sizeof(m_tracks));
	
	if (open)
	{
		islSdkLogOpen(filename);
	}
	else
	{
		islSdkLogCreate(filename);
	}

	loggerList.push_back(this);
}
//--------------------------------------------------------------------------------------------------
Logger::~Logger()
{
	islSdkLogClose(m_id);
	Device::removeLoggerInst(*this);

	for (size_t i = 0; i < loggerList.size(); i++)
	{
		if (loggerList[i]->m_id == m_id)
		{
			loggerList.erase(loggerList.begin() + i);
			break;
		}
	}
}
//--------------------------------------------------------------------------------------------------
void Logger::handleEvent(uint64_t timeMs, uint32_t eventId, const void* data)
{
	logInfo_t* info;
	logDataRecord_t* logDataRecord;
	bool_t processed;

	switch (eventId)
	{
	case EVENT_LOG_ERROR:
		onError(*this, ((sdkError_t*)data)->errorCode, ((sdkError_t*)data)->msg);
		break;

	case EVENT_LOG_CREATED:
		m_ready = true;
		break;

	case EVENT_LOG_OPENED:
	case EVENT_LOG_INFO:
		info = (logInfo_t*)data;
		if (info->trackCount > countof(m_tracks))
		{
			info->trackCount = countof(m_tracks);
		}
		m_totalRecordCount = info->totalRecordCount;
		m_startTime = info->startTime;
		m_totalDurationMs = info->totalDurationMs;
		m_trackCount = info->trackCount;
		memCopy(&m_tracks[0], info->logTrackInfo, sizeof(logTrackInfo_t) * m_trackCount);

		if (eventId == EVENT_LOG_OPENED)
		{
			m_ready = true;
			onFileOpen(*this);
		}
		break;

	case EVENT_LOG_CLOSED:
		break;

	case EVENT_LOG_TRACK_ADDED:
		getInfo();
		break;

	case EVENT_LOG_NEW_FILE:
		onNewFile(*this, (utf8_t*)data);
		break;

	case EVENT_LOG_FILE_SIZE:
		onLogFileMaxSize(*this);
		break;

	case EVENT_LOG_META_RECORD:
	case EVENT_LOG_DATA_RECORD:
		logDataRecord = (logDataRecord_t*)data;
		processed = logHelperProcessRecord(logDataRecord);
		onLogPlayback(*this, *logDataRecord, processed, eventId == EVENT_LOG_META_RECORD);
		break;

	default:
		break;
	}
}
//---------------------------------------------------------------------------------------------------
bool_t Logger::getTrackInfo(uint32_t trackId, logTrackInfo_t& logTrackInfo)
{
	uint32_t i;

	if (m_ready)
	{
		for (i = 0; i < m_trackCount; i++)
		{
			if (m_tracks[i].trackId == trackId)
			{
				logTrackInfo = m_tracks[i];
				return true;
			}
		}
	}
	return false;
}
//---------------------------------------------------------------------------------------------------
void Logger::startNewFile(uint32_t maxSize)
{
	m_maxFileSize = maxSize;
	islSdkLogStartNewFile(m_id, maxSize);
}
//---------------------------------------------------------------------------------------------------
uint32_t Logger::addTrack(uint32_t appId, uint32_t partNumber, uint16_t pid)
{
	m_trackCount++;
	islSdkLogAddTrack(m_id, m_trackCount, appId, partNumber, pid);
	return m_trackCount;
}
//---------------------------------------------------------------------------------------------------
void Logger::addData(uint32_t trackId, bool_t isMeta, bool_t canSkip, uint32_t dataType, const void* data, uint32_t dataSize)
{
	islSdkLogAddData(m_id, trackId, isMeta, canSkip, dataType, data, dataSize);
}
//---------------------------------------------------------------------------------------------------
void Logger::play(real_t speed)
{
	islSdkLogPlay(m_id, speed);
}
//---------------------------------------------------------------------------------------------------
void Logger::seek(uint32_t index)
{
	islSdkLogSeek(m_id, index);
}
//---------------------------------------------------------------------------------------------------
void Logger::getInfo()
{
	islSdkLogGetInfo(m_id);
}
//---------------------------------------------------------------------------------------------------

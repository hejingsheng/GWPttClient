#include "PcAudioDevice.h"
#include "GWLog.h"
#include "GWPttEngine.h"
#include <mutex>
#include <list>

#ifdef USE_SDL_AUDIO_PLAY
#include "SDL.h"
#pragma comment(lib, "SDL2.lib")
#endif

#include <QThread>

static void startRecord();
static void stopRecord();
static void startPlay();
static void stopPlay();
static void playbackPlayData(char *pcm, int len);
static void muteSpeaker(char mute);
static void muteRecorder(char mute);

static GWPttAudioModule pttAudioDevice = {
		startPlay,
		stopPlay,
		playbackPlayData,
		startRecord,
		stopRecord,
		muteSpeaker,
		muteRecorder,
};

struct PcmData
{
	unsigned char pcm[640];
	int len;
};

static std::mutex pcmDataMutex_;
static int packetSize_;
static std::list<PcmData> pcmDataList_;
static char jitterFull = 0;
static int jitterCache = 15; // 15*20ms = 300ms

qint64 AudioPcmBuffer::readData(char *stream, qint64 len)
{
#ifdef USE_SDL_AUDIO_PLAY
	return -1;
#else
	if (dir == 1)
	{
		struct PcmData *data = NULL;
		if (jitterFull)
		{
			std::lock_guard<std::mutex> lock(pcmDataMutex_);
			if (pcmDataList_.size() <= 0)
			{
				memset(stream, 0, len);
				pttUpdateLeftVoicePacket(0);
				return 640;
			}
			else
			{
				struct PcmData data = pcmDataList_.front();
				int length = data.len;
				unsigned char *pcm = data.pcm;
				if (length < len)
				{
					memcpy(stream, pcm, length);
				}
				else
				{
					memcpy(stream, pcm, len);
				}
				pcmDataList_.pop_front();
				pttUpdateLeftVoicePacket(pcmDataList_.size());
				return length;
			}
		}
		else
		{
			memset(stream, 0, len);
			return 640;
		}
	}
	else
	{
		return 640;
	}
#endif
}

qint64 AudioPcmBuffer::writeData(const char* data, qint64 len)
{
	//qDebug() << "Got data:" << len;
	if (dir == 0)
	{
		pttOnPcmData((char*)data, 640);
		return len;
	}
	else
	{
		return -1;
	}
}

QThread *recordThread = nullptr;
AudioRecordWorker *recordWorker = nullptr;
AudioRecordWorker::AudioRecordWorker()
{
	recordDevice = nullptr;
	recordBuffer = nullptr;
}

AudioRecordWorker::~AudioRecordWorker()
{
	stopRecord();
}

void AudioRecordWorker::startRecord() 
{
	QAudioFormat format;
	format.setSampleRate(GW_PTT_AUDIO_SAMPLERATE);      // 16kHz采样率
	format.setChannelCount(GW_PTT_AUDIO_CHANNELS);        // 单声道
	format.setSampleSize(GW_PTT_AUDIO_BITS);         // 16位采样
	format.setCodec("audio/pcm");     // PCM编码
	format.setByteOrder(QAudioFormat::LittleEndian);
	format.setSampleType(QAudioFormat::SignedInt);
	QAudioDeviceInfo inputDevice = QAudioDeviceInfo::defaultInputDevice();
	if (!inputDevice.isFormatSupported(format)) {
		format = inputDevice.nearestFormat(format);
	}
	recordDevice = new QAudioInput(inputDevice, format);
	recordDevice->setBufferSize(GW_PTT_AUDIO_BYTES_PER_FRAME * 5);
	recordBuffer = new AudioPcmBuffer;
	recordBuffer->open(QIODevice::WriteOnly);
	recordBuffer->setDir(0);
	recordDevice->start(recordBuffer);
}

void AudioRecordWorker::stopRecord() 
{
	if (recordDevice != nullptr)
	{
		recordDevice->stop();
		delete recordDevice;
	}
	recordDevice = nullptr;
	if (recordBuffer != nullptr)
	{
		recordBuffer->close();
		delete recordBuffer;
	}
	recordBuffer = nullptr;
}
#ifdef USE_SDL_AUDIO_PLAY
static SDL_AudioDeviceID playDevice = 0;
void onNeedPlayData(void *userdata, Uint8 * stream, int len)
{
	struct PcmData *data = NULL;
	if (jitterFull)
	{
		std::lock_guard<std::mutex> lock(pcmDataMutex_);
		if (pcmDataList_.size() <= 0)
		{
			memset(stream, 0, len);
			pttUpdateLeftVoicePacket(0);
		}
		else
		{
			struct PcmData data = pcmDataList_.front();
			int length = data.len;
			unsigned char *pcm = data.pcm;
			if (length < len)
			{
				memcpy(stream, pcm, length);
			}
			else
			{
				memcpy(stream, pcm, len);
			}
			pcmDataList_.pop_front();
			pttUpdateLeftVoicePacket(pcmDataList_.size());
		}
	}
	else
	{
		memset(stream, 0, len);
	}
}
#else
QThread *playThread = nullptr;
AudioPlayWorker *playWorker = nullptr;
AudioPlayWorker::AudioPlayWorker()
{
	playDevice = nullptr;
	playBuffer = nullptr;
}

AudioPlayWorker::~AudioPlayWorker()
{
	stopPlay();
}

void AudioPlayWorker::startPlay()
{
	QAudioFormat format;
	format.setSampleRate(GW_PTT_AUDIO_SAMPLERATE);      // 16kHz采样率
	format.setChannelCount(GW_PTT_AUDIO_CHANNELS);        // 单声道
	format.setSampleSize(GW_PTT_AUDIO_BITS);         // 16位采样
	format.setCodec("audio/pcm");     // PCM编码
	format.setByteOrder(QAudioFormat::LittleEndian);
	format.setSampleType(QAudioFormat::SignedInt);
	QAudioDeviceInfo outputDevice = QAudioDeviceInfo::defaultOutputDevice();
	if (!outputDevice.isFormatSupported(format)) {
		format = outputDevice.nearestFormat(format);
	}
	playDevice = new QAudioOutput(outputDevice, format);
	playDevice->setBufferSize(GW_PTT_AUDIO_BYTES_PER_FRAME * 5);
	playBuffer = new AudioPcmBuffer;
	playBuffer->open(QIODevice::ReadOnly);
	playBuffer->setDir(1);
	playDevice->start(playBuffer);
}

void AudioPlayWorker::stopPlay()
{
	if (playDevice != nullptr)
	{
		playDevice->stop();
		delete playDevice;
	}
	playDevice = nullptr;
	if (playBuffer != nullptr)
	{
		playBuffer->close();
		delete playBuffer;
	}
	playBuffer = nullptr;
}
#endif

GWPttAudioModule* pcInitAudioDevice(int sampleRate, int bits, int channels)
{
	recordThread = new QThread();
	recordWorker = new AudioRecordWorker();
	recordWorker->moveToThread(recordThread);
	QObject::connect(recordThread, &QThread::started, recordWorker, &AudioRecordWorker::startRecord);
	QObject::connect(recordThread, &QThread::finished, recordWorker, &AudioRecordWorker::stopRecord);
#ifdef USE_SDL_AUDIO_PLAY
#else
	playThread = new QThread();
	playWorker = new AudioPlayWorker();
	playWorker->moveToThread(playThread);
	QObject::connect(playThread, &QThread::started, playWorker, &AudioPlayWorker::startPlay);
	QObject::connect(playThread, &QThread::finished, playWorker, &AudioPlayWorker::stopPlay);
#endif
	packetSize_ = ((sampleRate / (1000 / GW_PTT_AUDIO_DURATION_MS_PER_FRAME)*bits / 8)*channels);
	return &pttAudioDevice;
}

static void startRecord()
{
	recordThread->start();
}

static void stopRecord()
{
	recordThread->quit();
}

static void startPlay()
{
#ifdef USE_SDL_AUDIO_PLAY
	SDL_AudioSpec want, have;
	SDL_Init(SDL_INIT_AUDIO);
	SDL_zero(want);
	want.freq = GW_PTT_AUDIO_SAMPLERATE;
	want.format = AUDIO_S16SYS;
	want.channels = GW_PTT_AUDIO_CHANNELS;
	want.samples = GW_PTT_AUDIO_BYTES_PER_FRAME / 2;
	want.callback = onNeedPlayData;
	want.size = GW_PTT_AUDIO_BYTES_PER_FRAME;
	want.userdata = NULL;
	playDevice = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
	if (playDevice == 0)
	{
		GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "Failed to open audio play device: %s\n", SDL_GetError());
		return;
	}
	else
	{
		printf("Buffer size in samples: %u\n", have.samples);
		printf("Callback will receive approx %u bytes per call\n", have.samples * have.channels * (SDL_AUDIO_BITSIZE(have.format) / 8));
	}
	SDL_PauseAudioDevice(playDevice, 0);
#else
	playThread->start();
#endif
}

static void stopPlay()
{
	// stop play non block
#ifdef USE_SDL_AUDIO_PLAY
	SDL_CloseAudioDevice(playDevice);
	SDL_Quit();
#else
	playThread->quit();
#endif
}

static void playbackPlayData(char *pcm, int len)
{
	int idx = 0;

	std::lock_guard<std::mutex> lock(pcmDataMutex_);
	while (idx < len)
	{
		size_t length = ((len - idx) > packetSize_) ? packetSize_ : (len - idx);
		struct PcmData pcmData;
		memset(&pcmData, 0, sizeof(struct PcmData));
		memcpy(pcmData.pcm, pcm + idx, length);
		pcmData.len = length;
		pcmDataList_.push_back(pcmData);
		idx += length;
	}
	if (pcmDataList_.size() > jitterCache && jitterFull == 0)
	{
		GWLOG_PRINT(GW_LOG_LEVEL_INFO, "pkg cache finish start play");
		jitterFull = 1;
	}
}

static void muteSpeaker(char mute)
{

}

static void muteRecorder(char mute)
{
}
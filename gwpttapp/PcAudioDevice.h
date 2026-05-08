#ifndef _PC_AUDIODEVICE_H_
#define _PC_AUDIODEVICE_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "GWPttAudioDevice.h"
#ifdef __cplusplus
}
#endif

#define USE_SDL_AUDIO_PLAY

#include <QAudioFormat>
#include <QAudioInput>
#ifdef USE_SDL_AUDIO_PLAY
#else
#include <QAudioOutput>
#endif
#include <QAudioDeviceInfo>
#include <QObject>

GWPttAudioModule* pcInitAudioDevice(int sampleRate, int bits, int channels);

class AudioPcmBuffer : public QIODevice {
public:
	explicit AudioPcmBuffer(QObject* parent = nullptr) : QIODevice(parent) { ; }

	bool isSequential() {
		return true;
	}

	void setDir(int dir) {
		this->dir = dir;
	}

protected:
	qint64 readData(char *stream, qint64 len) override;

	qint64 writeData(const char* data, qint64 len) override;

private:
	bool dir; // 0 record  1 play

};

class AudioRecordWorker : public QObject {

	Q_OBJECT

public:
	AudioRecordWorker();
	~AudioRecordWorker();

public slots:
	void startRecord();
	void stopRecord();

private:
	QAudioInput *recordDevice = nullptr;
	AudioPcmBuffer *recordBuffer = nullptr;
};

#ifdef USE_SDL_AUDIO_PLAY
#else
class AudioPlayWorker : public QObject {

	Q_OBJECT

public:
	AudioPlayWorker();
	~AudioPlayWorker();

public slots:
	void startPlay();
	void stopPlay();

private:
	QAudioOutput *playDevice = nullptr;
	AudioPcmBuffer *playBuffer = nullptr;
};

#endif

#endif
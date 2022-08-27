#include "AudioInputControl.h"

#include <stdlib.h>
#include <math.h>
#include <QDateTime>
#include <QDebug>
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <qendian.h>
#include "UserSettings.h"
#include "../Logging.h"

AudioInputControl::AudioInputControl(){
}

void AudioInputControl::setVolume(const QString &deviceDesc, int volume){
    auto index = audioDeviceIndex(deviceDesc);
    if(index==-1){
        nhlog::dev()->warn("Device description not found: {}" , deviceDesc.toStdString());
        return;
    }
    _inputDevices.setVolume(index, volume);
}

QAudioDeviceInfo AudioInputControl::audioDeviceInfo(const QString &deviceDesc){
    QString deviceName;
    auto sources = _inputDevices.sources();
    for(auto &source: sources.toStdMap()){
        if(source.second.desc == deviceDesc){
            deviceName = source.second.name;
            break;
        }
    }
    if(deviceName.isEmpty()){
        return QAudioDeviceInfo();
    }

    for (auto &deviceInfo: QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
        if (deviceName == deviceInfo.deviceName())
            return deviceInfo;
    }
    return QAudioDeviceInfo();
}

int32_t AudioInputControl::audioDeviceIndex(const QString &deviceDesc){
    auto sources = _inputDevices.sources();
    for(auto &source: sources.toStdMap()){
        if(source.second.desc == deviceDesc){
            return source.second.index;
        }
    }
    return -1;
}

void AudioInputControl::initializeAudio(const QString &deviceDesc)
{
    QAudioDeviceInfo deviceInfo = audioDeviceInfo(deviceDesc);
    if(deviceInfo.isNull()){
        nhlog::dev()->warn("Device description not found: {}" , deviceDesc.toStdString());
        return;
    }
    QAudioFormat format;
    format.setSampleRate(8000);
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setCodec("audio/pcm");

    if (!deviceInfo.isFormatSupported(format)) {
        nhlog::dev()->warn("Default format not supported - trying to use nearest");
        format = deviceInfo.nearestFormat(format);
    }

    m_audioInfo.reset(new AudioInfo(format));
    connect(m_audioInfo.data(), &AudioInfo::update, [this]() {
        emit levelChanged(m_audioInfo->level());
    });

    m_audioInput.reset(new QAudioInput(deviceInfo, format));
    m_audioInfo->start();
    m_audioInput->start(m_audioInfo.data());
}

void AudioInputControl::deviceChanged(const QString &deviceDesc)
{
    if(!m_audioInfo.isNull())
        m_audioInfo->stop();
    if(!m_audioInput.isNull()){
        m_audioInput->stop();
        m_audioInput->disconnect(this);
    }
    initializeAudio(deviceDesc);
}

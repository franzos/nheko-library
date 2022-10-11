#include "AudioDeviceControl.h"
#ifndef Q_OS_ANDROID
#include <stdlib.h>
#include <math.h>
#include <QDateTime>
#include <QDebug>
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <qendian.h>
#include "UserSettings.h"
#include "../Logging.h"

AudioDeviceControl::AudioDeviceControl(){
    connect(&_audioDevices, &AudioDevices::newInputDeviceStatus,[&](uint32_t index) {
        emit newInputDeviceStatus(index);
    });
    connect(&_audioDevices, &AudioDevices::newOutputDeviceStatus,[&](uint32_t index) {
        emit newOutputDeviceStatus(index);
    });
}

qreal AudioDeviceControl::getMicrophoneVolume(const QString &deviceDesc){
    auto index = audioDeviceIndex(deviceDesc);
    if(index==-1){
        nhlog::dev()->warn("Device description not found: {}" , deviceDesc.toStdString());
        return 0;
    }
    return _audioDevices.getMicrophoneVolume(index);
}

void AudioDeviceControl::setMicrophoneVolume(const QString &deviceDesc, qreal volume){
    auto index = audioDeviceIndex(deviceDesc);
    if(index==-1){
        nhlog::dev()->warn("Device description not found: {}" , deviceDesc.toStdString());
        return;
    }
    _audioDevices.setMicrophoneVolume(index, volume);
}

void AudioDeviceControl::setSpeakerVolume(qreal volume){
    _audioDevices.setSpeakerVolume(volume);
}

qreal AudioDeviceControl::getSpeakerVolume(){
    return _audioDevices.getSpeakerVolume();
}

QAudioDeviceInfo AudioDeviceControl::audioDeviceInfo(const QString &deviceDesc){
    QString deviceName;
    auto sources = _audioDevices.sources();
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

int32_t AudioDeviceControl::audioDeviceIndex(const QString &deviceDesc){
    auto sources = _audioDevices.sources();
    for(auto &source: sources.toStdMap()){
        if(source.second.desc == deviceDesc){
            return source.second.index;
        }
    }
    return -1;
}

void AudioDeviceControl::initializeAudio(const QString &deviceDesc)
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

void AudioDeviceControl::deviceChanged(const QString &deviceDesc)
{
    if(!m_audioInfo.isNull())
        m_audioInfo->stop();
    if(!m_audioInput.isNull()){
        m_audioInput->stop();
        m_audioInput->disconnect(this);
    }
    initializeAudio(deviceDesc);
}

AudioDeviceInfo AudioDeviceControl::deviceInfo(qint32 index){
    AudioDeviceInfo info;
    if(_audioDevices.sources().count(index)){
        info =_audioDevices.sources()[index];
    }
    return info;
}

#else

AudioDeviceControl::AudioDeviceControl() {}

qreal AudioDeviceControl::getMicrophoneVolume(const QString &deviceDesc) {
    Q_UNUSED(deviceDesc);
    return 0.0;
}

void AudioDeviceControl::setMicrophoneVolume(const QString &deviceDesc, qreal volume) {
    Q_UNUSED(deviceDesc);
    Q_UNUSED(volume);
}

void AudioDeviceControl::setSpeakerVolume(qreal volume) {
    Q_UNUSED(volume);
}

qreal AudioDeviceControl::getSpeakerVolume() {
    return 0.0;
}

QAudioDeviceInfo AudioDeviceControl::audioDeviceInfo(const QString &deviceDesc) {
    Q_UNUSED(deviceDesc);
    return QAudioDeviceInfo {};
}

int32_t AudioDeviceControl::audioDeviceIndex(const QString &deviceDesc) {
    Q_UNUSED(deviceDesc);
    return -1;
}

void AudioDeviceControl::initializeAudio(const QString &deviceDesc) {
    Q_UNUSED(deviceDesc);
}

void AudioDeviceControl::deviceChanged(const QString &deviceDesc) {
    Q_UNUSED(deviceDesc);
}

AudioDeviceInfo AudioDeviceControl::deviceInfo(qint32 index) {
    Q_UNUSED(index);
    return AudioDeviceInfo {};
}
#endif
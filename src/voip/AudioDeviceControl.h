#pragma once 
#include <QtGlobal>
#ifndef Q_OS_ANDROID
#include <QAudioInput>
#include <QByteArray>
#include <QObject>
#include <QPixmap>
#include <QScopedPointer>
#include "AudioInfo.h"
#include "InputDevices.h"
#endif
class AudioDeviceControl : public QObject
{
    Q_OBJECT
#ifndef Q_OS_ANDROID
public:
    AudioDeviceControl();

public slots:
    void initializeAudio(const QString &deviceDesc);
    void deviceChanged(const QString &deviceDesc);
    void setMicrophoneVolume(const QString &deviceDesc, qreal volume);
    void setSpeakerVolume(qreal volume);
    qreal getVolume(const QString &deviceDesc);
    InputDeviceInfo deviceInfo(qint32 index);
    
private:
    QAudioDeviceInfo audioDeviceInfo(const QString &deviceDesc);
    int32_t audioDeviceIndex(const QString &deviceDesc);

signals:
    void levelChanged(const qreal &level);
    void newDeviceStatus(qint32 index);

private:
    QScopedPointer<AudioInfo> m_audioInfo;
    QScopedPointer<QAudioInput> m_audioInput;
    InputDevices _inputDevices;
#endif
};
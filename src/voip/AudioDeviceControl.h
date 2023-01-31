#pragma once 
#include <QByteArray>
#include <QObject>
#include <QPixmap>
#include <QScopedPointer>
#include <QtGlobal>

#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
#include <QAudioInput>
#else

struct QAudioDeviceInfo {};
struct QAudioInput {};

#endif

#include "AudioInfo.h"
#include "AudioDevices.h"

class AudioDeviceControl : public QObject
{
    Q_OBJECT
public:
    AudioDeviceControl();

public slots:
    void initializeAudio(const QString &deviceDesc);
    void deviceChanged(const QString &deviceDesc);
    void setMicrophoneVolume(const QString &deviceDesc, qreal volume);
    qreal getMicrophoneVolume(const QString &deviceDesc);
    void setSpeakerVolume(const QString &deviceDesc, qreal volume);
    qreal getSpeakerVolume(const QString &deviceDesc);
    AudioDeviceInfo deviceInfo(qint32 index);
    QStringList audioOutputDevices();
    void setDefaultAudioOutput(const QString &deviceDesc);
    QString defaultAudioOutput();
    
private:
    QAudioDeviceInfo audioDeviceInfo(const QString &deviceDesc);
    int32_t audioDeviceSourceIndex(const QString &deviceDesc);
    int32_t audioDeviceSinkIndex(const QString &desc);

signals:
    void levelChanged(const qreal &level);
    void newInputDeviceStatus(qint32 index);
    void newOutputDeviceStatus(qint32 index);
    void defaultOutputDeviceChanged(qint32 index);

private:
    QScopedPointer<AudioInfo> m_audioInfo;
    QScopedPointer<QAudioInput> m_audioInput;
    AudioDevices _audioDevices;
};
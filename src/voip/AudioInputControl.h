#pragma once 

#include <QAudioInput>
#include <QByteArray>
#include <QObject>
#include <QPixmap>
#include <QScopedPointer>
#include "AudioInfo.h"
#include "InputDevices.h"

class AudioInputControl : public QObject
{
    Q_OBJECT

public:
    AudioInputControl();

public slots:
    void initializeAudio(const QString &deviceDesc);
    void deviceChanged(const QString &deviceDesc);
    void setVolume(const QString &deviceDesc, int volume);

private:
    QAudioDeviceInfo audioDeviceInfo(const QString &deviceDesc);
    int32_t audioDeviceIndex(const QString &deviceDesc);

signals:
    void levelChanged(const qreal &level);

private:
    QScopedPointer<AudioInfo> m_audioInfo;
    QScopedPointer<QAudioInput> m_audioInput;
    InputDevices _inputDevices;
};

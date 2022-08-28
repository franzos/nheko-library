#pragma once 

#include <QtGlobal>
#ifndef Q_OS_ANDROID
#include <QAudioInput>
#include <QByteArray>
#include <QObject>
#endif

class AudioInfo : public QIODevice
{
    Q_OBJECT
#ifndef Q_OS_ANDROID
public:
    AudioInfo(const QAudioFormat &format);

    void start();
    void stop();

    qreal level() const { return m_level; }

    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

private:
    const QAudioFormat m_format;
    quint32 m_maxAmplitude = 0;
    qreal m_level = 0.0; // 0.0 <= m_level <= 1.0

signals:
    void update();
#endif
};

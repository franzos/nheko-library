#pragma once

#include <QObject>
#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>
#include <pulse/ext-stream-restore.h>
#include <pulse/ext-device-manager.h>
#include <QMap>

class InputDeviceInfo {
    public:
        int index;
        QString name;
        QString desc;
};

class InputDevices : public QObject
{
    Q_OBJECT

public:
    InputDevices(QObject *parent = nullptr);
    ~InputDevices(){};
    void removeSource(uint32_t index);
    void updateSource(const pa_source_info &info);
    QMap<uint32_t, InputDeviceInfo> sources() {return _sources;};

signals:
    void newDeviceStatus(uint32_t index);

public slots:
    void setVolume(uint32_t index, int volume);

private:
    QMap<uint32_t, InputDeviceInfo> _sources;
};

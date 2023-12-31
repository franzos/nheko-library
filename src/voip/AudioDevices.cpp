#include "AudioDevices.h"

#include <QtGlobal>

#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
#include <QDebug>
#include <glib.h>
#include "Logging.h"

#define PACKAGE_VERSION "0.1"

static pa_context* context = nullptr;
static pa_mainloop_api* api = nullptr;
static bool retry = false;
static int reconnect_timeout = 1;


/* Forward Declaration */
gboolean connect_to_pulse(gpointer userdata);

void show_error(const char *txt) {
    char buf[256];

    snprintf(buf, sizeof(buf), "%s: %s", txt, pa_strerror(pa_context_errno(context)));

    // QMessageBox::critical(nullptr, QObject::tr("Error"), QString::fromUtf8(buf));
    // qApp->quit();
    // TODO
}

void source_cb(pa_context *, const pa_source_info *i, int eol, void *userdata) {
    AudioDevices *inputDevices = static_cast<AudioDevices*>(userdata);

    if (eol < 0) {
        if (pa_context_errno(context) == PA_ERR_NOENTITY)
            return;

        show_error(QObject::tr("Source callback failure").toUtf8().constData());
        return;
    }

    if (eol > 0) {
        // dec_outstanding(w);
        return;
    }
    inputDevices->updateSource(*i);
}

void sink_cb(pa_context *, const pa_sink_info *i, int eol, void *userdata) {
    AudioDevices *outputDevices = static_cast<AudioDevices*>(userdata);

    if (eol < 0) {
        if (pa_context_errno(context) == PA_ERR_NOENTITY)
            return;

        show_error(QObject::tr("Sink callback failure").toUtf8().constData());
        return;
    }

    if (eol > 0) {
        // dec_outstanding(w);
        return;
    }

#if HAVE_EXT_DEVICE_RESTORE_API
    if (w->updateSink(*i))
        ext_device_restore_subscribe_cb(c, PA_DEVICE_TYPE_SINK, i->index, w);
#else
    outputDevices->updateSink(*i);
#endif
    outputDevices->checkDefaultOutput();
}

void subscribe_cb(pa_context *c, pa_subscription_event_type_t t, uint32_t index, void *userdata) {
    AudioDevices *inputDevice = static_cast<AudioDevices*>(userdata);

    switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) {
        case PA_SUBSCRIPTION_EVENT_SINK:
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE)
                inputDevice->removeSink(index);
            else {
                pa_operation *o;
                if (!(o = pa_context_get_sink_info_by_index(c, index, sink_cb, userdata))) {
                    show_error(QObject::tr("pa_context_get_sink_info_by_index() failed").toUtf8().constData());
                    return;
                }
                pa_operation_unref(o);
            }
            break;

        case PA_SUBSCRIPTION_EVENT_SOURCE:
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
                inputDevice->removeSource(index);
            } else {
                pa_operation *o;
                if (!(o = pa_context_get_source_info_by_index(c, index, source_cb, userdata))) {
                    show_error(QObject::tr("pa_context_get_source_info_by_index() failed").toUtf8().constData());
                    return;
                }
                pa_operation_unref(o);
            }
            break;

        case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
            break;

        case PA_SUBSCRIPTION_EVENT_SOURCE_OUTPUT:
            break;

        case PA_SUBSCRIPTION_EVENT_CLIENT:
            break;

        case PA_SUBSCRIPTION_EVENT_SERVER: 
            break;

        case PA_SUBSCRIPTION_EVENT_CARD:
            break;

    }
}

void context_state_callback(pa_context *c, void *userdata) {
    g_assert(c);

    switch (pa_context_get_state(c)) {
        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
            break;

        case PA_CONTEXT_READY: {
            pa_operation *o;

            reconnect_timeout = 1;

            pa_context_set_subscribe_callback(c, subscribe_cb, userdata);

            if (!(o = pa_context_subscribe(c, (pa_subscription_mask_t)
                                           (PA_SUBSCRIPTION_MASK_SINK|
                                            PA_SUBSCRIPTION_MASK_SOURCE|
                                            PA_SUBSCRIPTION_MASK_SINK_INPUT|
                                            PA_SUBSCRIPTION_MASK_SOURCE_OUTPUT|
                                            PA_SUBSCRIPTION_MASK_CLIENT|
                                            PA_SUBSCRIPTION_MASK_SERVER|
                                            PA_SUBSCRIPTION_MASK_CARD), nullptr, nullptr))) {
                show_error(QObject::tr("pa_context_subscribe() failed").toUtf8().constData());
                return;
            }
            pa_operation_unref(o);

            if (!(o = pa_context_get_source_info_list(c, source_cb, userdata))) {
                show_error(QObject::tr("pa_context_get_source_info_list() failed").toUtf8().constData());
                return;
            }
            pa_operation_unref(o);
            if (!(o = pa_context_get_sink_info_list(c, sink_cb, userdata))) {
                show_error(QObject::tr("pa_context_get_sink_info_list() failed").toUtf8().constData());
                return;
            }
            pa_operation_unref(o);
            break;
        }

        case PA_CONTEXT_FAILED:
            // TODO
            // w->setConnectionState(false);
            // w->removeAllWidgets();
            // w->updateDeviceVisibility();
            pa_context_unref(context);
            context = nullptr;

            if (reconnect_timeout > 0) {
                g_debug("%s", QObject::tr("Connection failed, attempting reconnect").toUtf8().constData());
                g_timeout_add_seconds(reconnect_timeout, connect_to_pulse, userdata);
            }
            return;

        case PA_CONTEXT_TERMINATED:
        default:
            // qApp->quit(); TODO
            return;
    }
}

gboolean connect_to_pulse(gpointer userdata) {
    if (context)
        return false;

    pa_proplist *proplist = pa_proplist_new();
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME, QObject::tr("PulseAudio Volume Control").toUtf8().constData());
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ID, "org.PulseAudio.pavucontrol");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ICON_NAME, "audio-card");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_VERSION, PACKAGE_VERSION);

    context = pa_context_new_with_proplist(api, nullptr, proplist);
    g_assert(context);

    pa_proplist_free(proplist);

    pa_context_set_state_callback(context, context_state_callback, userdata);

    // w->setConnectingMessage(); TODO
    if (pa_context_connect(context, nullptr, PA_CONTEXT_NOFAIL, nullptr) < 0) {
        if (pa_context_errno(context) == PA_ERR_INVALID) {
            // TODO
            // w->setConnectingMessage(QObject::tr("Connection to PulseAudio failed. Automatic retry in 5s.<br><br>"
            //     "In this case this is likely because PULSE_SERVER in the Environment/X11 Root Window Properties"
            //     "or default-server in client.conf is misconfigured.<br>"
            //     "This situation can also arrise when PulseAudio crashed and left stale details in the X11 Root Window.<br>"
            //     "If this is the case, then PulseAudio should autospawn again, or if this is not configured you should"
            //     "run start-pulseaudio-x11 manually.").toUtf8().constData());
            reconnect_timeout = 5;
        }
        else {
            if(!retry) {
                reconnect_timeout = -1;
                // qApp->quit();
                // TODO
            } else {
                g_debug("%s", QObject::tr("Connection failed, attempting reconnect").toUtf8().constData());
                reconnect_timeout = 5;
                g_timeout_add_seconds(reconnect_timeout, connect_to_pulse, userdata);
            }
        }
    }

    return false;
}

void AudioDevices::removeSource(uint32_t index){
    if (!_sources.count(index))
        return;
    _sources.remove(index);
}

void AudioDevices::removeSink(uint32_t index){
    if (!_sinks.count(index))
        return;
    _sinks.remove(index);
}

void AudioDevices::updateSink(const pa_sink_info &info) {
    if(!_sinks.count(info.index)){
        AudioDeviceInfo devInfo;
        devInfo.index = info.index;
        devInfo.name  = QString(info.name);
        devInfo.desc  = QString(info.description);
        _sinks[info.index] = devInfo;
        nhlog::dev()->info("Audio Device Output: {} {} {}", devInfo.index, devInfo.name.toStdString(), devInfo.desc.toStdString());
    }
    _sinks[info.index].volume = (info.volume.channels?(qreal)info.volume.values[0]:0) / 65535;
    emit newOutputDeviceStatus(info.index);
}

void AudioDevices::updateSource(const pa_source_info &info){
    if(!_sources.count(info.index)){
        AudioDeviceInfo devInfo;
        devInfo.index = info.index;
        devInfo.name  = QString(info.name);
        devInfo.desc  = QString(info.description);
        _sources[info.index] = devInfo;
        nhlog::dev()->info("Audio Device Input: {} {} {}", devInfo.index, devInfo.name.toStdString(), devInfo.desc.toStdString());
    }
    _sources[info.index].volume = (info.volume.channels?(qreal)info.volume.values[0]:0) / 65535;
    emit newInputDeviceStatus(info.index);
}

AudioDevices::AudioDevices(QObject *parent):
    QObject(parent){
    pa_glib_mainloop *m = pa_glib_mainloop_new(g_main_context_default());
    g_assert(m);
    api = pa_glib_mainloop_get_api(m);
    g_assert(api);
    connect_to_pulse(this);
}

void AudioDevices::setMicrophoneVolume(uint32_t index, qreal volume){
    if(volume < 0 || volume > 1){
        nhlog::dev()->warn("Volume value is not correct: ({})", volume);
        return;
    } 
    if(!_sources.count(index)){
        nhlog::dev()->warn("Input device index is not valid: ({})", index);
        return;
    }
    nhlog::dev()->info("Set device input \"{}\" volume to {}%", _sources[index].name.toStdString() , std::to_string(int(volume * 100)));
    std::string volumeCommand = "pactl -- set-source-volume " + std::to_string(index) + " " + std::to_string(int(volume * 100)) + "%"; 
    nhlog::dev()->debug("Exec: {}", volumeCommand);
    system(volumeCommand.c_str());
}

qreal AudioDevices::getMicrophoneVolume(uint32_t index){
    if(!_sources.count(index)){
        nhlog::dev()->warn("Input device index is not valid: ({})", index);
        return 0;
    }
    return _sources[index].volume;
}

void AudioDevices::setSpeakerVolume(uint32_t index, qreal volume){
    std::string volumeCommand = "pactl -- set-sink-volume " + std::to_string(index) + " " + std::to_string(int(volume * 100)) + "%"; 
    nhlog::dev()->debug("Exec: {}", volumeCommand);
    system(volumeCommand.c_str());
}

qreal AudioDevices::getSpeakerVolume(uint32_t index){
    if(!_sinks.count(index)){
        nhlog::dev()->warn("Output device index is not valid: ({})", index);
        return 0;
    }
    return _sinks[index].volume;
}

QString AudioDevices::defaultAudioOutput(){
    checkDefaultOutput();
    return _defaultAudioOutput;
}

void AudioDevices::checkDefaultOutput(){
    nhlog::dev()->debug("Checking the default audio output ...");
    for(auto &s: _sinks.toStdMap()){
        std::string volumeCommand = "pactl get-default-sink | grep -e " + s.second.name.toStdString(); 
        auto result = system(volumeCommand.c_str());
        if(result == 0){
            _defaultAudioOutput = s.second.desc;
            nhlog::dev()->debug("Default audio output is " + s.second.name.toStdString() + "(index: " + std::to_string(s.second.index) + ")");
            emit defaultOutputDeviceChanged(s.second.index);
        }
    }
}
#else

AudioDevices::AudioDevices(QObject* parent): QObject(parent) {}

void AudioDevices::removeSource(uint32_t index) {
    Q_UNUSED(index);
}

void AudioDevices::removeSink(uint32_t index) {
    Q_UNUSED(index);
}

void AudioDevices::updateSink(const pa_sink_info &info) {
    Q_UNUSED(info);
}

void AudioDevices::updateSource(const pa_source_info &info) {
    Q_UNUSED(info);
}

void AudioDevices::setMicrophoneVolume(uint32_t index, qreal volume) {
    Q_UNUSED(index);
    Q_UNUSED(volume);
}

qreal AudioDevices::getMicrophoneVolume(uint32_t index) {
    Q_UNUSED(index);
    return 0;
}

void AudioDevices::setSpeakerVolume(uint32_t index, qreal volume) {
    Q_UNUSED(index);
    Q_UNUSED(volume);
}

qreal AudioDevices::getSpeakerVolume(uint32_t index){
    Q_UNUSED(index);
    return 0;
}

QString AudioDevices::defaultAudioOutput(){
    return 0;
}

void AudioDevices::checkDefaultOutput(){
}
#endif
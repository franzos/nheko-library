// SPDX-FileCopyrightText: 2021 Nheko Contributors
// SPDX-FileCopyrightText: 2022 Nheko Contributors
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <memory>

#include <QUrl>

#include "Cache.h"
#include "CallDevices.h"
#include "CallManager.h"
#include "../Client.h"
#include "Logging.h"
#include "MatrixClient.h"
#include "UserSettings.h"
#include "Utils.h"

#include "mtx/responses/turn_server.hpp"

#ifdef XCB_AVAILABLE
#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>
#endif

#ifdef GSTREAMER_AVAILABLE

extern "C"
{
#include "gst/gst.h"
}

#if defined(Q_OS_ANDROID)
extern "C" gboolean gst_qt_android_init (GError ** error);
#elif defined(Q_OS_IOS)
#include "../gstreamer/gst_ios_init.h"
#endif

#endif // GSTREAMER_AVAILABLE

Q_DECLARE_METATYPE(std::vector<mtx::events::voip::CallCandidates::Candidate>)
Q_DECLARE_METATYPE(mtx::events::voip::CallCandidates::Candidate)
Q_DECLARE_METATYPE(mtx::responses::TurnServer)

using namespace mtx::events;
using namespace mtx::events::voip;

using webrtc::CallType;
//! Session Description Object
typedef RTCSessionDescriptionInit SDO;

namespace {
std::vector<std::string>
getTurnURIs(const mtx::responses::TurnServer &turnServer);
}

CallManager::CallManager(QObject *parent)
  : QObject(parent)
  , session_(WebRTCSession::instance())
  , turnServerTimer_(this)
{
    qRegisterMetaType<std::vector<mtx::events::voip::CallCandidates::Candidate>>();
    qRegisterMetaType<mtx::events::voip::CallCandidates::Candidate>();
    qRegisterMetaType<mtx::responses::TurnServer>();

#ifdef GSTREAMER_AVAILABLE
    // TODO: we need to check Nheko's approach that doesn't require the following
    //       workaround to use the `qmlglsink` inside it's QML files
#if defined(Q_OS_ANDROID)
    if (!gst_qt_android_init(NULL)) {
        nhlog::ui()->error("WebRTC: Failed to init gstreamer!");
    }
#elif defined(Q_OS_IOS)
    gst_ios_init();
#else
    gst_init(NULL, NULL);
#endif

    GstElement *sink = gst_element_factory_make("qmlglsink", NULL);
    if (!sink) {
        nhlog::ui()->error("WebRTC: failed to make factory: qmlglsink");
    }
    gst_object_unref(sink);
#endif

    connect(
      &session_,
      &WebRTCSession::offerCreated,
      this,
      [this](const std::string &sdp, const std::vector<CallCandidates::Candidate> &candidates) {
          nhlog::ui()->debug("WebRTC: call id: {} - sending offer", callid_);
          emit newMessage(
            roomid_,
            CallInvite{callid_, partyid_, SDO{sdp, SDO::Type::Offer}, "0", timeoutms_, invitee_});
          emit newMessage(roomid_, CallCandidates{callid_, partyid_, candidates, "0"});
          std::string callid(callid_);
          QTimer::singleShot(timeoutms_, this, [this, callid]() {
              if (session_.state() == webrtc::State::OFFERSENT && callid == callid_) {
                  hangUp(CallHangUp::Reason::InviteTimeOut);
                  emit Client::instance()->showNotification(
                    QStringLiteral("The remote side failed to pick up."));
              }
          });
      });

    connect(
      &session_,
      &WebRTCSession::answerCreated,
      this,
      [this](const std::string &sdp, const std::vector<CallCandidates::Candidate> &candidates) {
          nhlog::ui()->debug("WebRTC: call id: {} - sending answer", callid_);
          emit newMessage(roomid_, CallAnswer{callid_, partyid_, "0", SDO{sdp, SDO::Type::Answer}});
          emit newMessage(roomid_, CallCandidates{callid_, partyid_, candidates, "0"});
      });

    connect(&session_,
            &WebRTCSession::newICECandidate,
            this,
            [this](const CallCandidates::Candidate &candidate) {
                nhlog::ui()->debug("WebRTC: call id: {} - sending ice candidate", callid_);
                emit newMessage(roomid_, CallCandidates{callid_, partyid_, {candidate}, "0"});
            });

    connect(&turnServerTimer_, &QTimer::timeout, this, &CallManager::retrieveTurnServer);

    connect(
      this, &CallManager::turnServerRetrieved, this, [this](const mtx::responses::TurnServer &res) {
          nhlog::net()->info("TURN server(s) retrieved from homeserver:");
          nhlog::net()->info("username: {}", res.username);
          nhlog::net()->info("ttl: {} seconds", res.ttl);
          for (const auto &u : res.uris)
              nhlog::net()->info("uri: {}", u);

          // Request new credentials close to expiry
          // See https://tools.ietf.org/html/draft-uberti-behave-turn-rest-00
          turnURIs_    = getTurnURIs(res);
          uint32_t ttl = std::max(res.ttl, UINT32_C(3600));
          if (res.ttl < 3600)
              nhlog::net()->warn("Setting ttl to 1 hour");
          turnServerTimer_.setInterval(ttl * 1000 * 0.9);
      });

    connect(&session_, &WebRTCSession::stateChanged, this, [this](webrtc::State state) {
        switch (state) {
        case webrtc::State::DISCONNECTED:
            clear();
            break;
        case webrtc::State::ICEFAILED: {
            QString error(QStringLiteral("Call connection failed."));
            if (turnURIs_.empty())
                error += QLatin1String(" Your homeserver has no configured TURN server.");
            nhlog::ui()->error(error.toStdString());
            emit Client::instance()->showNotification(error);
            hangUp(CallHangUp::Reason::ICEFailed);
            break;
        }
        default:
            break;
        }
        emit newCallState();
    });

    connect(
      &CallDevices::instance(), &CallDevices::devicesChanged, this, &CallManager::devicesChanged);
}

bool
CallManager::sendInvite(const QString &roomid, CallType callType, unsigned int windowIndex)
{
    Q_UNUSED(windowIndex)
    if (isOnCall()){
        nhlog::ui()->error("User is already onCall");
        return false;
    }

    auto roomInfo = cache::singleRoomInfo(roomid.toStdString());
    if (roomInfo.member_count != 2) {
        nhlog::ui()->error("Calls are limited to 1:1 rooms.");
        emit Client::instance()->showNotification(
          QStringLiteral("Calls are limited to 1:1 rooms."));
        return false;
    }

    std::string errorMessage;
    if (!session_.havePlugins(false, &errorMessage) ||
        (callType == CallType::VIDEO &&  !session_.havePlugins(true, &errorMessage))) {
        nhlog::ui()->error(errorMessage);
        emit Client::instance()->showNotification(QString::fromStdString(errorMessage));
        return false;
    }

    if (callType == CallType::VIDEO && !CallDevices::instance().haveCamera()) {
        errorMessage = "Camera not connected!"; 
        nhlog::ui()->error(errorMessage);
        emit Client::instance()->showNotification(QString::fromStdString(errorMessage));
        return false;
    }

    if (callType == CallType::VOICE && !CallDevices::instance().haveMic()) {
        errorMessage = "Microphone not connected!"; 
        nhlog::ui()->error(errorMessage);
        emit Client::instance()->showNotification(QString::fromStdString(errorMessage));
        return false;
    }

    callType_ = callType;
    roomid_   = roomid;
    session_.setTurnServers(turnURIs_);
    generateCallID();
    std::string strCallType =
      callType_ == CallType::VOICE ? "voice" : (callType_ == CallType::VIDEO ? "video" : "screen");
    nhlog::ui()->debug("WebRTC: call id: {} - creating {} invite", callid_, strCallType);
    std::vector<RoomMember> members(cache::getMembers(roomid.toStdString()));
    const RoomMember &callee =
      members.front().user_id == utils::localUser() ? members.back() : members.front();
    callParty_            = callee.user_id;
    callPartyDisplayName_ = callee.display_name.isEmpty() ? callee.user_id : callee.display_name;
    callPartyAvatarUrl_   = roomInfo.avatar_url;
    emit newInviteState();
    if (!session_.createOffer(callType, 0)) {
        nhlog::ui()->error("Problem setting up call.");
        emit Client::instance()->showNotification(QStringLiteral("Problem setting up call."));
        endCall();
        return false;
    }
    return true;
}

namespace {
std::string
callHangUpReasonString(CallHangUp::Reason reason)
{
    switch (reason) {
    case CallHangUp::Reason::ICEFailed:
        return "ICE failed";
    case CallHangUp::Reason::InviteTimeOut:
        return "Invite time out";
    case CallHangUp::Reason::ICETimeOut:
        return "ICE time out";
    case CallHangUp::Reason::UserHangUp:
        return "User hung up";
    case CallHangUp::Reason::UserMediaFailed:
        return "User media failed";
    case CallHangUp::Reason::UserBusy:
        return "User busy";
    case CallHangUp::Reason::UnknownError:
        return "Unknown error";
    default:
        return "User";
    }
}
}

void
CallManager::hangUp(CallHangUp::Reason reason)
{
    if (!callid_.empty()) {
        nhlog::ui()->debug(
          "WebRTC: call id: {} - hanging up ({})", callid_, callHangUpReasonString(reason));
        emit newMessage(roomid_, CallHangUp{callid_, partyid_, "0", reason});
        endCall();
    }
}

void
CallManager::syncEvent(const mtx::events::collections::TimelineEvents &event)
{
#ifdef GSTREAMER_AVAILABLE
    if (handleEvent<CallInvite>(event) || handleEvent<CallCandidates>(event) ||
        handleEvent<CallAnswer>(event) || handleEvent<CallHangUp>(event))
        return;
#else
    (void)event;
#endif
}

template<typename T>
bool
CallManager::handleEvent(const mtx::events::collections::TimelineEvents &event)
{
    if (std::holds_alternative<RoomEvent<T>>(event)) {
        handleEvent(std::get<RoomEvent<T>>(event));
        return true;
    }
    return false;
}

void
CallManager::handleEvent(const RoomEvent<CallInvite> &callInviteEvent)
{
    const char video[]     = "m=video";
    const std::string &sdp = callInviteEvent.content.offer.sdp;
    bool isVideo           = std::search(sdp.cbegin(),
                               sdp.cend(),
                               std::cbegin(video),
                               std::cend(video) - 1,
                               [](unsigned char c1, unsigned char c2) {
                                   return std::tolower(c1) == std::tolower(c2);
                               }) != sdp.cend();

    nhlog::ui()->debug("WebRTC: call id: {} - incoming {} CallInvite from {}",
                       callInviteEvent.content.call_id,
                       (isVideo ? "video" : "voice"),
                       callInviteEvent.sender);

    if (callInviteEvent.content.call_id.empty())
        return;

    auto roomInfo = cache::singleRoomInfo(callInviteEvent.room_id);
    if (isOnCall() || roomInfo.member_count != 2) {
        emit newMessage(
          QString::fromStdString(callInviteEvent.room_id),
          CallHangUp{
            callInviteEvent.content.call_id, partyid_, "0", CallHangUp::Reason::InviteTimeOut});
        return;
    }

    const QString &ringtone = UserSettings::instance()->ringtone();
    Q_UNUSED(ringtone);
    roomid_ = QString::fromStdString(callInviteEvent.room_id);
    callid_ = callInviteEvent.content.call_id;
    remoteICECandidates_.clear();

    std::vector<RoomMember> members(cache::getMembers(callInviteEvent.room_id));
    const RoomMember &caller =
      members.front().user_id == utils::localUser() ? members.back() : members.front();
    callParty_            = caller.user_id;
    callPartyDisplayName_ = caller.display_name.isEmpty() ? caller.user_id : caller.display_name;
    callPartyAvatarUrl_   = roomInfo.avatar_url;

    haveCallInvite_ = true;
    callType_       = isVideo ? CallType::VIDEO : CallType::VOICE;
    inviteSDP_      = callInviteEvent.content.offer.sdp;
    emit newInviteState();
}

void
CallManager::acceptInvite()
{
    if (!haveCallInvite_)
        return;

    std::string errorMessage;
    if (!session_.havePlugins(false, &errorMessage) ||
        (callType_ == CallType::VIDEO && !session_.havePlugins(true, &errorMessage))) {
        nhlog::ui()->error(errorMessage);
        emit Client::instance()->showNotification(QString::fromStdString(errorMessage));
        hangUp();
        return;
    }

    session_.setTurnServers(turnURIs_);
    if (!session_.acceptOffer(inviteSDP_)) {
        nhlog::ui()->error("Problem setting up call.");
        emit Client::instance()->showNotification(QStringLiteral("Problem setting up call."));
        hangUp();
        return;
    }
    session_.acceptICECandidates(remoteICECandidates_);
    remoteICECandidates_.clear();
    haveCallInvite_ = false;
    emit newInviteState();
}

void
CallManager::handleEvent(const RoomEvent<CallCandidates> &callCandidatesEvent)
{
    if (callCandidatesEvent.sender == utils::localUser().toStdString())
        return;

    nhlog::ui()->debug("WebRTC: call id: {} - incoming CallCandidates from {}",
                       callCandidatesEvent.content.call_id,
                       callCandidatesEvent.sender);

    if (callid_ == callCandidatesEvent.content.call_id) {
        if (isOnCall())
            session_.acceptICECandidates(callCandidatesEvent.content.candidates);
        else {
            // CallInvite has been received and we're awaiting localUser to accept or
            // reject the call
            for (const auto &c : callCandidatesEvent.content.candidates)
                remoteICECandidates_.push_back(c);
        }
    }
}

void
CallManager::handleEvent(const RoomEvent<CallAnswer> &callAnswerEvent)
{
    nhlog::ui()->debug("WebRTC: call id: {} - incoming CallAnswer from {}",
                       callAnswerEvent.content.call_id,
                       callAnswerEvent.sender);

    if (callAnswerEvent.sender == utils::localUser().toStdString() &&
        callid_ == callAnswerEvent.content.call_id) {
        if (!isOnCall()) {
            nhlog::ui()->debug("Call answered on another device.");
            emit Client::instance()->showNotification(
              QStringLiteral("Call answered on another device."));
            haveCallInvite_ = false;
            emit newInviteState();
        }
        return;
    }

    if (isOnCall() && callid_ == callAnswerEvent.content.call_id) {
        if (!session_.acceptAnswer(callAnswerEvent.content.answer.sdp)) {
            nhlog::ui()->error("Problem setting up call.");
            emit Client::instance()->showNotification(QStringLiteral("Problem setting up call."));
            hangUp();
        }
    }
}

void
CallManager::handleEvent(const RoomEvent<CallHangUp> &callHangUpEvent)
{
    nhlog::ui()->debug("WebRTC: call id: {} - incoming CallHangUp ({}) from {}",
                       callHangUpEvent.content.call_id,
                       callHangUpReasonString(callHangUpEvent.content.reason),
                       callHangUpEvent.sender);

    if (callid_ == callHangUpEvent.content.call_id)
        endCall();
}

void
CallManager::toggleMicMute()
{
    session_.toggleMicMute();
    emit micMuteChanged();
}

bool
CallManager::callsSupported()
{
#ifdef GSTREAMER_AVAILABLE
    return true;
#else
    return false;
#endif
}

bool
CallManager::screenShareSupported()
{
    return std::getenv("DISPLAY") && !std::getenv("WAYLAND_DISPLAY");
}

QStringList
CallManager::devices(bool isVideo) const
{
    QStringList ret;
    const QString &defaultDevice =
      isVideo ? UserSettings::instance()->camera() : UserSettings::instance()->microphone();
    std::vector<std::string> devices =
      CallDevices::instance().names(isVideo, defaultDevice.toStdString());
    ret.reserve(devices.size());
    std::transform(devices.cbegin(), devices.cend(), std::back_inserter(ret), [](const auto &d) {
        return QString::fromStdString(d);
    });

    return ret;
}

void
CallManager::generateCallID()
{
    using namespace std::chrono;
    uint64_t ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    callid_     = "c" + std::to_string(ms);
}

void
CallManager::clear()
{
    roomid_.clear();
    callParty_.clear();
    callPartyDisplayName_.clear();
    callPartyAvatarUrl_.clear();
    callid_.clear();
    callType_       = CallType::VOICE;
    haveCallInvite_ = false;
    emit newInviteState();
    inviteSDP_.clear();
    remoteICECandidates_.clear();
}

void
CallManager::endCall()
{
    session_.end();
    clear();
}

void
CallManager::refreshTurnServer()
{
    turnURIs_.clear();
    turnServerTimer_.start(2000);
}

void
CallManager::retrieveTurnServer()
{
    http::client()->get_turn_server(
      [this](const mtx::responses::TurnServer &res, mtx::http::RequestErr err) {
          if (err) {
              turnServerTimer_.setInterval(5000);
              return;
          }
          emit turnServerRetrieved(res);
      });
}

QStringList
CallManager::windowList()
{
    windows_.clear();
    windows_.push_back({tr("Entire screen"), 0});

#ifdef XCB_AVAILABLE
    std::unique_ptr<xcb_connection_t, std::function<void(xcb_connection_t *)>> connection(
      xcb_connect(nullptr, nullptr), [](xcb_connection_t *c) { xcb_disconnect(c); });
    if (xcb_connection_has_error(connection.get())) {
        nhlog::ui()->error("Failed to connect to X server");
        return {};
    }

    xcb_ewmh_connection_t ewmh;
    if (!xcb_ewmh_init_atoms_replies(
          &ewmh, xcb_ewmh_init_atoms(connection.get(), &ewmh), nullptr)) {
        nhlog::ui()->error("Failed to connect to EWMH server");
        return {};
    }
    std::unique_ptr<xcb_ewmh_connection_t, std::function<void(xcb_ewmh_connection_t *)>>
      ewmhconnection(&ewmh, [](xcb_ewmh_connection_t *c) { xcb_ewmh_connection_wipe(c); });

    for (int i = 0; i < ewmh.nb_screens; i++) {
        xcb_ewmh_get_windows_reply_t clients;
        if (!xcb_ewmh_get_client_list_reply(
              &ewmh, xcb_ewmh_get_client_list(&ewmh, i), &clients, nullptr)) {
            nhlog::ui()->error("Failed to request window list");
            return {};
        }

        for (uint32_t w = 0; w < clients.windows_len; w++) {
            xcb_window_t window = clients.windows[w];

            std::string name;
            xcb_ewmh_get_utf8_strings_reply_t data;
            auto getName = [](xcb_ewmh_get_utf8_strings_reply_t *r) {
                std::string name(r->strings, r->strings_len);
                xcb_ewmh_get_utf8_strings_reply_wipe(r);
                return name;
            };

            xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_name(&ewmh, window);
            if (xcb_ewmh_get_wm_name_reply(&ewmh, cookie, &data, nullptr))
                name = getName(&data);

            cookie = xcb_ewmh_get_wm_visible_name(&ewmh, window);
            if (xcb_ewmh_get_wm_visible_name_reply(&ewmh, cookie, &data, nullptr))
                name = getName(&data);

            windows_.push_back({QString::fromStdString(name), window});
        }
        xcb_ewmh_get_windows_reply_wipe(&clients);
    }
#endif
    QStringList ret;
    ret.reserve(windows_.size());
    for (const auto &w : windows_)
        ret.append(w.first);

    return ret;
}

#ifdef GSTREAMER_AVAILABLE
namespace {

GstElement *pipe_        = nullptr;
unsigned int busWatchId_ = 0;

gboolean
newBusMessage(GstBus *bus G_GNUC_UNUSED, GstMessage *msg, gpointer G_GNUC_UNUSED)
{
    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS:
        if (pipe_) {
            gst_element_set_state(GST_ELEMENT(pipe_), GST_STATE_NULL);
            gst_object_unref(pipe_);
            pipe_ = nullptr;
        }
        if (busWatchId_) {
            g_source_remove(busWatchId_);
            busWatchId_ = 0;
        }
        break;
    default:
        break;
    }
    return TRUE;
}
}
#endif

void
CallManager::previewWindow(unsigned int index) const
{
#ifdef GSTREAMER_AVAILABLE
    if (windows_.empty() || index >= windows_.size() || !gst_is_initialized())
        return;

    GstElement *ximagesrc = nullptr;
#if defined(Q_OS_ANDROID)
    ximagesrc = gst_element_factory_make("ahcsrc", nullptr);
#elif defined(Q_OS_IOS)
    ximagesrc = gst_element_factory_make("avfvideosrc", nullptr);
#else 
    ximagesrc = gst_element_factory_make("ximagesrc", nullptr);
#endif
    if (!ximagesrc) {
        nhlog::ui()->error("Failed to create ximagesrc");
        return;
    }
    GstElement *videoconvert = gst_element_factory_make("videoconvert", nullptr);
    GstElement *capsfilter   = gst_element_factory_make("capsfilter", nullptr);
    GstElement *ximagesink   = gst_element_factory_make("ximagesink", nullptr);

    g_object_set(ximagesrc, "use-damage", FALSE, nullptr);
    g_object_set(ximagesrc, "show-pointer", FALSE, nullptr);
    g_object_set(ximagesrc, "xid", windows_[index].second, nullptr);

    GstCaps *caps = gst_caps_new_simple(
      "video/x-raw", "width", G_TYPE_INT, 480, "height", G_TYPE_INT, 360, nullptr);
    g_object_set(capsfilter, "caps", caps, nullptr);
    gst_caps_unref(caps);

    pipe_ = gst_pipeline_new(nullptr);
    gst_bin_add_many(
      GST_BIN(pipe_), ximagesrc, videoconvert, capsfilter, ximagesink, nullptr);
    if (!gst_element_link_many(
          ximagesrc, videoconvert, capsfilter, ximagesink, nullptr)) {
        nhlog::ui()->error("Failed to link preview window elements");
        gst_object_unref(pipe_);
        pipe_ = nullptr;
        return;
    }
    if (gst_element_set_state(pipe_, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) {
        nhlog::ui()->error("Unable to start preview pipeline");
        gst_object_unref(pipe_);
        pipe_ = nullptr;
        return;
    }

    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipe_));
    busWatchId_ = gst_bus_add_watch(bus, newBusMessage, nullptr);
    gst_object_unref(bus);
#else
    (void)index;
#endif
}

namespace {
std::vector<std::string>
getTurnURIs(const mtx::responses::TurnServer &turnServer)
{
    // gstreamer expects: turn(s)://username:password@host:port?transport=udp(tcp)
    // where username and password are percent-encoded
    std::vector<std::string> ret;
    for (const auto &uri : turnServer.uris) {
        if (auto c = uri.find(':'); c == std::string::npos) {
            nhlog::ui()->error("Invalid TURN server uri: {}", uri);
            continue;
        } else {
            std::string scheme = std::string(uri, 0, c);
            if (scheme != "turn" && scheme != "turns") {
                nhlog::ui()->error("Invalid TURN server uri: {}", uri);
                continue;
            }

            QString encodedUri =
              QString::fromStdString(scheme) + "://" +
              QUrl::toPercentEncoding(QString::fromStdString(turnServer.username)) + ":" +
              QUrl::toPercentEncoding(QString::fromStdString(turnServer.password)) + "@" +
              QString::fromStdString(std::string(uri, ++c));
            ret.push_back(encodedUri.toStdString());
        }
    }
    return ret;
}
}

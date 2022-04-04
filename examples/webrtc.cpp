
#include <gst/gst.h>

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <mtx/responses/common.hpp>
#include <mtx/responses/turn_server.hpp>

#include "Client.h"
#include "voip/CallDevices.h"
#include "voip/CallManager.h"

class WebRTCHandler : public QObject {
    Q_OBJECT
   public:
    explicit WebRTCHandler(Client *client, QObject *parent = nullptr)
        : QObject(parent),
          client_(client) {}

    Q_INVOKABLE void hangup() {
        client_->callManager()->hangUp();
    }

   private:
    Client *client_;
};
#include "webrtc.moc"

int main(int argc, char *argv[]) {
    QApplication app{argc, argv};

    QCoreApplication::setApplicationName("WebRTC example");
    QCoreApplication::setApplicationVersion("1.0.0");
    QCommandLineParser parser;
    parser.setApplicationDescription("Example Application to perform video/voice calls using Nheko's WebRTC facilities");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({{"w", "wait-for-call"}, "Wait for an incoming call"});
    parser.addOption({"video", "perform video call"});
    parser.process(app);
    auto waitForCall = parser.isSet("wait-for-call");
    auto callType = parser.isSet("video") ? webrtc::CallType::VIDEO : webrtc::CallType::VOICE;

    // Following section fixed the GstGLVideoItem import issue in QML file
    gst_init(NULL, NULL);
    GstElement *sink = gst_element_factory_make("qmlglsink", NULL);
    gst_object_unref(sink);

    QString deviceName = "voip-test";
    QString userId = "@reza_test02:pantherx.org";
    QString password = "98KoWG2KUjsuPcyvvnjKhd92";
    QString serverAddress = "https://matrix.pantherx.org";
    QString targetUserId = "@r.majd:pantherx.org";
    QString targetRoomId = "!LBljXrKlFDSGQDadbK:pantherx.org";

    auto *client = Client::instance();
    WebRTCHandler webrtc{client};
    auto *session = &WebRTCSession::instance();
    if (client == nullptr) {
        qCritical() << "Client is not initiated";
        return -1;
    }
    // client->enableLogger(true, true);
    auto *callMgr = client->callManager();
    if (callMgr == nullptr) {
        qCritical() << "Call manager is not initialized";
        return -1;
    }

    UserSettings::instance()->clear();
    UserSettings::instance()->setUseStunServer(true);

    QQmlApplicationEngine engine;
    qmlRegisterSingletonInstance<WebRTCHandler>("examples.webrtc", 1, 0, "WebRTCHandler", &webrtc);
    QObject::connect(&engine, &QQmlEngine::quit, [&app, client]() {
        client->stop();
        app.exit(0);
    });
    engine.load(QUrl(QStringLiteral("qrc:/webrtc.qml")));

    auto *video = engine.rootObjects().first()->findChild<QQuickItem *>("videoCallItem");
    nhlog::ui()->info(">>> WebRTC VIDEO: {}", video != nullptr ? std::string("FOUND") : std::string("NULL"));

    QObject::connect(client, &Client::initialSync, [=](const mtx::responses::Sync &sync) {
        Q_UNUSED(sync)
        nhlog::ui()->info(">>> INITIAL SYNC");
    });
    QObject::connect(client, &Client::newUpdated, [=](const mtx::responses::Sync &sync) {
        Q_UNUSED(sync)
        nhlog::ui()->info(">>> NEW UPDATED");
    });
    QObject::connect(client, &Client::newSyncResponse, [=](const mtx::responses::Sync &sync, const QString &prev_token) {
        Q_UNUSED(sync)
        nhlog::ui()->info(">>> NEW SYNC RESPONSE: {}", prev_token.toStdString());
    });
    QObject::connect(client, &Client::dropToLogin, [=](const QString &msg) {
        nhlog::ui()->info(">>> DROP TO LOGIN: (msg: {})", msg.toStdString());
        client->loginWithPassword(deviceName, userId, password, serverAddress);
    });
    QObject::connect(client, &Client::loginErrorOccurred, [=](const QString &err) {
        nhlog::ui()->error(">>> LOGIN ERROR: {}", err.toStdString());
    });
    QObject::connect(client, &Client::loginOk, [=](const UserInformation &user) {
        nhlog::ui()->info(">>> LOGIN OK: {}", user.userId.toStdString());
        client->start();
    });
    QObject::connect(client, &Client::initiateFinished, [=]() {
        nhlog::ui()->info(">>> CLIENT INITIATED");
    });

    QObject::connect(callMgr, &CallManager::turnServerRetrieved, [=](const mtx::responses::TurnServer &turnInfo) {
        nhlog::ui()->info(">>> Turn server retrieved: {}", turnInfo.uris.size());
        auto rooms = client->joinedRoomList();
        nhlog::ui()->info(">>> ROOMS: {}", rooms.size());
        bool targetFound = false;
        for (const auto &roomid : rooms.keys()) {
            nhlog::ui()->info("   - {} : {}", roomid.toStdString(), rooms[roomid].name.toStdString());
            if (roomid == targetRoomId) {
                targetFound = true;
            }
        }
        if (targetFound && !waitForCall) {
            // The order is important: 1. send invite 2. set session's video
            callMgr->sendInvite(targetRoomId, callType);
            if (video != nullptr) {
                session->setVideoItem(video);
            }
        }
    });

    QObject::connect(callMgr, &CallManager::newInviteState, [=]() {
        if (waitForCall || true) {
            callMgr->acceptInvite();
            if (video != nullptr) {
                session->setVideoItem(video);
            }
        }
    });

    QObject::connect(callMgr, &CallManager::devicesChanged, [=]() {
        auto defaultMic = UserSettings::instance()->microphone();
        auto defaultCam = UserSettings::instance()->camera();
        auto mics = CallDevices::instance().names(false, defaultMic.toStdString());
        auto cams = CallDevices::instance().names(true, defaultCam.toStdString());
        nhlog::ui()->info(">>> DEVICES CHANGED: mics: {} - cams: {}", mics.size(), cams.size());
        if (mics.size() > 0) {
            for (const auto &mic : mics) {
                auto q_mic = QString::fromStdString(mic);
                if (!q_mic.toLower().startsWith("monitor")) {
                    UserSettings::instance()->setMicrophone(q_mic);
                    nhlog::ui()->info("   - [mic]: {}", mic);
                    break;
                }
            }
        }
        if (cams.size() > 0) {
            UserSettings::instance()->setCamera(QString::fromStdString(cams[0]));
            nhlog::ui()->info("   - [cam]: {}", cams[0]);
        }
    });

    QObject::connect(callMgr,
                     qOverload<const QString &, const mtx::events::msg::CallInvite &>(&CallManager::newMessage),
                     [=](const QString &roomid, const mtx::events::msg::CallInvite &invite) {
                         nhlog::ui()->info(">>> CALL INVITE: callid: {} - room: {}", invite.call_id, roomid.toStdString());
                         if (auto timeline = client->timeline(roomid)) {
                             timeline->sendMessageEvent(invite, mtx::events::EventType::CallInvite);
                         }
                     });

    QObject::connect(callMgr,
                     qOverload<const QString &, const mtx::events::msg::CallCandidates &>(&CallManager::newMessage),
                     [=](const QString &roomid, const mtx::events::msg::CallCandidates &candidate) {
                         nhlog::ui()->info(">>> CALL CANDIDATE: callid: {} - room: {}", candidate.call_id, roomid.toStdString());
                         if (auto timeline = client->timeline(roomid)) {
                             timeline->sendMessageEvent(candidate, mtx::events::EventType::CallCandidates);
                         }
                     });

    QObject::connect(callMgr,
                     qOverload<const QString &, const mtx::events::msg::CallAnswer &>(&CallManager::newMessage),
                     [=](const QString &roomid, const mtx::events::msg::CallAnswer &answer) {
                         nhlog::ui()->info(">>> CALL ANSWER: callid: {} - room: {}", answer.call_id, roomid.toStdString());
                         if (auto timeline = client->timeline(roomid)) {
                             timeline->sendMessageEvent(answer, mtx::events::EventType::CallAnswer);
                         }
                     });

    QObject::connect(callMgr,
                     qOverload<const QString &, const mtx::events::msg::CallHangUp &>(&CallManager::newMessage),
                     [=](const QString &roomid, const mtx::events::msg::CallHangUp &hangup) {
                         nhlog::ui()->info(">>> CALL HANGUP: callid: {} - room: {}", hangup.call_id, roomid.toStdString());
                         if (auto timeline = client->timeline(roomid)) {
                             timeline->sendMessageEvent(hangup, mtx::events::EventType::CallHangUp);
                         }
                     });

    client->start();
    return app.exec();
}
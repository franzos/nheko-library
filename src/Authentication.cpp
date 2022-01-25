#include "Authentication.h"
#include <QDebug>
#include <iostream>
#include "Utils.h"


void Authentication::loginWithPassword(std::string deviceName, std::string userId, std::string password, std::string serverAddress){
    http::client()->set_server(serverAddress);
    mtx::identifiers::User user;
    user = mtx::identifiers::parse<mtx::identifiers::User>(userId);        
    http::client()->login(          
    user.localpart(),
    password,
    deviceName,
    [this](const mtx::responses::Login &res, mtx::http::RequestErr err) {
        if (err) {
            auto s = utils::httpMtxErrorToString(err).toStdString();
            emit loginErrorOccurred(s);
            return;
        }
        if (res.well_known) {
            http::client()->set_server(res.well_known->homeserver.base_url);
        }
        emit loginOk(res);
    });

}


  void Authentication::logout(){
    http::client()->logout([this](const mtx::responses::Logout &, mtx::http::RequestErr err) {
        if (err) {
            auto s = utils::httpMtxErrorToString(err).toStdString();     
            emit logoutErrorOccurred(s);
            return;
        }        
        emit logoutOk();
    });
  }

std::string Authentication::serverDiscovery(std::string userId){
    mtx::identifiers::User user;    
    try {
        user = mtx::identifiers::parse<mtx::identifiers::User>(userId);
    } catch (const std::exception &) {
        // showError(error_matrixid_label_,
        //           tr("You have entered an invalid Matrix ID  e.g @joe:matrix.org"));
        return " ";
    }
  QString homeServer = QString::fromStdString(user.hostname());
  http::client()->set_server(user.hostname());
  http::client()->well_known(   
    [this](const mtx::responses::WellKnown &res, mtx::http::RequestErr err) {             
        if (err) {
            if (err->status_code == 404) {
                nhlog::net()->info("Autodiscovery: No .well-known.");
                // checkHomeserverVersion();
                return;
            }

            if (!err->parse_error.empty()) {
                // emit versionErrorCb(tr("Autodiscovery failed. Received malformed response."));
                nhlog::net()->error("Autodiscovery failed. Received malformed response.");
                return;
            }

            // emit versionErrorCb(tr("Autodiscovery failed. Unknown error when "
            //                        "requesting .well-known."));
            nhlog::net()->error("Autodiscovery failed. Unknown error when "
                                "requesting .well-known. {} {}",
                                err->status_code,
                                err->error_code);
            
            return;
        }

        nhlog::net()->info("Autodiscovery: Discovered '" + res.homeserver.base_url + "'");
        //http::client()->set_server(res.homeserver.base_url);
        //homeServer = res.homeserver.base_url;
    });
   return homeServer.toStdString();

}
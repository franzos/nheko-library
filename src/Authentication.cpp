#include "Authentication.h"
#include <QDebug>
#include <iostream>



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
                  auto error = err->matrix_error.error;
                  if (error.empty()){            
                        error = err->parse_error;
                  }
                  std::string s = std::string (error);
                  emit loginErrorOccurred(s);
                  return;
              }
              if (res.well_known) {
                  http::client()->set_server(res.well_known->homeserver.base_url);
                 // nhlog::net()->info("Login requested to user server: " +
                                     //res.well_known->homeserver.base_url);
              }
              //TODO put data to Database  
              emit loginOk(res);
          });

}

 bool Authentication::hasValidUser(){
     //TODO check database if there is access token ...
    //  if(true){ //there is user
    //         http::client()->set_access_token("token.toStdString()");
    //         http::client()->set_server("home_server.toStdString()");
    //         http::client()->set_device_id("device_id.toStdString()");
    //         return true;
    // }
    return false; 
 }

  mtx::responses::Login Authentication::userInformation(){
    //TODO read data from database and fill mtx::responses::Login
    mtx::responses::Login otput; 
    return otput;
  }

  void Authentication::logout(){
    http::client()->logout([this](const mtx::responses::Logout &, mtx::http::RequestErr err) {
        if (err) {
            // TODO: handle special errors
           // nhlog::net()->warn("failed to logout: {}", err);
           
            auto error = err->matrix_error.error;
            if (error.empty()){            
                error = err->parse_error;
            }
            std::string s = std::string (error);            
           emit logoutErrorOccurred(s);
            return;
        }
        //TODO Delete account from DB via deviceId
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
                // nhlog::net()->info("Autodiscovery: No .well-known.");
                // checkHomeserverVersion();
                std::cout<<"Error 1 "<<std::endl;
                return "";
            }

            if (!err->parse_error.empty()) {
                // emit versionErrorCb(tr("Autodiscovery failed. Received malformed response."));
                // nhlog::net()->error("Autodiscovery failed. Received malformed response.");
                std::cout<<"Error 2"<<std::endl;
                return "";
            }

            // emit versionErrorCb(tr("Autodiscovery failed. Unknown error when "
            //                        "requesting .well-known."));
            // nhlog::net()->error("Autodiscovery failed. Unknown error when "
            //                     "requesting .well-known. {} {}",
            //                     err->status_code,
            //                     err->error_code);
            
            return "";
        }

        // nhlog::net()->info("Autodiscovery: Discovered '" + res.homeserver.base_url + "'");
        //http::client()->set_server(res.homeserver.base_url);
        std::cout<<"Test: "<< res.homeserver.base_url<<std::endl;
        //homeServer = res.homeserver.base_url;
    });
   return homeServer.toStdString();

}
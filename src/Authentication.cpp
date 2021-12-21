#include "Authentication.h"
#include <QDebug>



void Authentication::loginWithUsername(std::string deviceName, std::string userId, std::string password, std::string serverAddress){
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
                  emit errorOccurred(s);
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
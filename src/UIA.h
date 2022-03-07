// SPDX-FileCopyrightText: 2021 Nheko Contributors
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Some more information
//                                                     Library                           GUI
//                                                        |                               |
//  if current_stage == password            --> emit password()           -->       showing a dialog box to get login password  
//                                                      .....             <--       passing the password by calling `continuePassword`
//                                                        |                               |
//  if current_stage == email               --> emit email()              -->       showing a dialog box to get email
//                                                      .....             <--       passing the email by calling `continueEmail`
//                                                        |                               |
//  if current_stage == phone               --> emit phone()              -->       showing a dialog box to get phone
//                                                      .....             <--       passing the phone by calling `continuePhoneNumber`
//                                                        |                               |
//  if current_stage == recaptcha           --> emit captchaDialog()      -->       showing a Captcha Dialog 
//                                                      .....             <--       confirm the recaptcha by calling `recaptchaConfirmation`
//                                                        |                               |
//  if current_stage == registration_token  --> emit registration_token() -->       Showing a Dialog box to get the registration token
//                                                      .....             <--       passing the token by calling `registrationOK
//                                                        |                               |
//  else                                    --> emit fallbackAuth()       -->       Showing a Dialog box to Auth
//                                                      .....             <--       go next by calling `fallbackAuthConfirmation`
//                                                        |                               |
//
// Note: 
//  * `recaptcha`, `registration_token` and `fallbackAuth` updated in the UIA.cpp. In nheko these parts mixed with GUI so we had to seperate them with Signals 
//    and handle them in GUI and then return back the responses to the library.

// Other Signals
//                                              emit prompt3pidToken()    -->       showing a dialog box to get token
//                                                      .....             <--       passing the token by calling `submit3pidToken`
//                                                        |                               |
//                                                 emit error()           -->       showing a dialog box to show error message
//                                                        |                               |

#pragma once

#include <QObject>

#include "MatrixClient.h"

class UIA : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title NOTIFY titleChanged)

public:
    static UIA *instance();

    UIA(QObject *parent = nullptr)
      : QObject(parent)
    {}

    mtx::http::UIAHandler genericHandler(QString context);

    QString title() const { return title_; }

public slots:
    void continuePassword(QString password);
    void continueEmail(QString email);
    void continuePhoneNumber(QString countryCode, QString phoneNumber);
    void submit3pidToken(QString token);
    void continue3pidReceived();

    void recaptchaConfirmation(const mtx::http::UIAHandler &h, const mtx::user_interactive::Unauthorized &u);
    void registrationOK(const QString &token, const mtx::http::UIAHandler &h, const mtx::user_interactive::Unauthorized &u);
    void fallbackAuthConfirmation(const mtx::http::UIAHandler &h, const mtx::user_interactive::Unauthorized &u);
    void cancel();

signals:
    void password();
    void email();
    void phoneNumber();
    void captchaDialog(const QString &context, const mtx::http::UIAHandler &h, const mtx::user_interactive::Unauthorized &u);
    void registrationToken(const QString &context, const mtx::http::UIAHandler &h, const mtx::user_interactive::Unauthorized &u);
    void fallbackAuth(const QString &context, const QString &currentStage, const mtx::http::UIAHandler &h, const mtx::user_interactive::Unauthorized &u);

    void confirm3pidToken();
    void prompt3pidToken();
    void tokenAccepted();

    void titleChanged();
    void error(QString msg);

private:
    std::optional<mtx::http::UIAHandler> currentHandler;
    mtx::user_interactive::Unauthorized currentStatus;
    QString title_;

    // for 3pids like email and phone number
    std::string client_secret;
    std::string sid;
    std::string submit_url;
    bool email_ = true;
};

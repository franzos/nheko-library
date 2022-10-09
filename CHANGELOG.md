# Change Log

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/)

## [0.1.24]
### Fixed

* Fixing issue in device verification status after login.


## [0.1.23]
### Fixed

* Fixing the encryption issue after device verification.


## [0.1.22]
### Changed and Fixed

* User Settings added.
* Fixing an issue to store server url.


## [0.1.21]
### Changed

* Update to send room information changes as signal.


## [0.1.20]
### Changed

* CIBA Login improvment.
* Cancel CIBA Login request.


## [0.1.19]
### Changed

* Add some logs.


## [0.1.18]
### Changed

* Minor change to make `knownUsers` accessible in QML.


## [0.1.17]
### Changed

* Speaker volume control added.


## [0.1.16]
### Fixed

* Fixing crash issue if video device resolution or framerates was empty.


## [0.1.15]
### Fixed

* Set default video framrate and resoultion if they were empty.


## [0.1.14]
### Changed

* Added missed depenecies to CMakeLists.txt.


## [0.1.13]
### Changed

* Support set device input volume.
* Support monitor device input level.


## [0.1.11]
### Fixed

* Fixing crash issue after member leave the direct chat.


## [0.1.10]
### Updated

* Integrated with PantherX Online Accounts.


## [0.1.9]
### Updated

* Merge with upstream@0.10.0.


## [0.1.8]
### Changed

* Improvment: Removing an unneeded method to return timeline object in UserProfile class.


## [0.1.7]
### Changed

* Get list of known users (members of existing group/private chats).
* Remove Image providers class to gui library.


## [0.1.5]
### Changed

* Fixing issue in save settings related to audio/video device inputs.
* Remove unused `curl` inclusion.


## [0.1.4]
### Changed

* Get permissions of timeline.
* Kick and ban user added.
* Sign-out from device added.
* Cleaning the old DB after each successful login if DB is exist.


## [0.1.3]
### Changed

* CIBA Login with access token.


## [0.1.2]
### Changed

* Read events, prepare and emit signals as notification.
* Remove unused rest request files.


## [0.1.1]
### Changed

* Emit error message if there is no Camera/Microphone in Video/Audio calls.
* Remove /user/me codes and using it from Auth library.


## [0.1.0]
### Changed

* Pin/Unpin message.
* Update with upstream.
* ReadReceipts added.
* raw message retrieving.


## [0.0.61]
### Changed

* Update CIBA authentication to re-use it in other places.
* Add get CM user information method.


## [0.0.60]
### Changed

* Login options correction

## [0.0.58]
### Changed

* Adding exteract host name from user Id

## [0.0.57]
### Changed

* Changing inputs of discovery method from userId to hostName.

## [0.0.56]
### Changed

* Adding server discovery.

## [0.0.55]
### Fixed

* Fixing issue in changing the WebRTCSession state when there is no audio stream in other side in Video Call.


## [0.0.54]
### Changed

* Remove the log from current working directory to QStandardPaths::AppDataLocation.


## [0.0.53]
### Changed

* Bug fixing in detecting video and audio devices.
* Bug fixing in retriving the Video devices.
* Bug fixing in reading the `framerate` from video devices.
* Add some warning log messages to `CallDevices.cpp`.


## [0.0.52]
### Changed

* get login Options.

## [0.0.51]
### Changed

* Avatar support.
* Check user presence support.


## [0.0.5]
### Changed

* Check user existence method added.


## [0.0.40]
### Fixed

* Send CallManager errors via showNotification signal.
* Fixing issue in Call just after creating chat and user accepted the invitation.


## [0.0.39]
### Fixed

* re-try login with ciba issue fixing.


## [0.0.37]
### Fixed

* Update Cache DB_SIZE to prevent crash during the start on Android


## [0.0.36]
### Changed

* VOIP Support in Build configuration.


## [0.0.35]
### Changed

* Update UserInformation structure


## [0.0.34]
### Fixed

* Check user id format in login.
* Fixing issue to sync room after accept invitation.


## [0.0.33]
### Changed

* Update CIBA login with new changes in server.
* Update to work with `mtxclient`@0.0.7.


## [0.0.32]
### Changed

* Library version is added.


## [0.0.31]
### Changed

* Support Voice/Video call.


## [0.0.27]
### Changed

* Verification implemented.


## [0.0.26]
### Updated

* Set `disable_certificate_validation` default value (`true`)
  (Initial tests passed in Android and Desktop)

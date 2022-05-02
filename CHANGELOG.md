# Change Log

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/)

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

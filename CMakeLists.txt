cmake_minimum_required(VERSION 3.13)
project(matrix-client-library)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard")
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "Require C++ standard to be supported")
set(CMAKE_POSITION_INDEPENDENT_CODE ON CACHE BOOL "compile as PIC by default")
set(GLIB_MINIMUM_VERSION "2.50.0")

include(CMakeDependentOption)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(GNUInstallDirs)
include_directories(src)

# find_package(GLIB "2.50.0" REQUIRED)

find_package(PkgConfig REQUIRED) 
pkg_check_modules(libcurl REQUIRED IMPORTED_TARGET libcurl)

option(VOIP "Whether to enable the voip support. disable this if you don't have the gstreamer" ON)
if(VOIP)
	message(" + VOIP: enabled.")
	pkg_check_modules(GSTREAMER REQUIRED IMPORTED_TARGET gstreamer-sdp-1.0>=1.18 gstreamer-webrtc-1.0>=1.18)
endif()

option(CIBA_AUTHENTICATION "Whether the CIBA authentication method enabled or not" OFF)
if(CIBA_AUTHENTICATION)
	message(" + CIBA: enabled.")
	FILE(WRITE src/Features.h
	"#define CIBA_AUTHENTICATION 1"
	)
else()
	FILE(WRITE src/Features.h
	"#define CIBA_AUTHENTICATION 0"
	)
endif()
option(PX_ACCOUNTS_INTEGRATION "Whether the integration with PantherX Accounts and Secrets services enabled or not" OFF)

option(BUILD_EXAMPLES "Whether build the examples or not" OFF)
option(BUILD_TESTS "Whether build the tests or not" OFF)

find_package(spdlog 1.0.0 CONFIG REQUIRED)
find_package(LMDB REQUIRED)

find_package(Qt5Network REQUIRED)
find_package(Qt5 5.15 COMPONENTS Gui QuickControls2 QuickWidgets Svg REQUIRED)

set(
	CMAKE_CXX_FLAGS
	"${CMAKE_CXX_FLAGS} \
	-Wall \
	-Wextra \
	-pipe \
	-pedantic \
	-fsized-deallocation \
	-fdiagnostics-color=always \
	-Wunreachable-code \
	-Wno-attributes"
	)
if (NOT CMAKE_COMPILER_IS_GNUCXX)
	# -Wshadow is buggy and broken in GCC, so do not enable it.
	# see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79328
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow")
endif()

#
# Declare source and header files.
#
set(SRC_FILES
	src/Authentication.cpp
	src/timeline/EventStore.cpp
	src/timeline/Permissions.cpp
	src/timeline/Reaction.cpp
	src/timeline/Timeline.cpp
	src/encryption/Olm.cpp
	src/encryption/DeviceVerificationFlow.cpp
	src/encryption/SelfVerificationStatus.cpp
	src/encryption/VerificationManager.cpp
	src/voip/AudioInfo.cpp
	src/voip/AudioDeviceControl.cpp
	src/voip/CallDevices.cpp
	src/voip/CallManager.cpp
	src/voip/AudioDevices.cpp
	src/voip/WebRTCSession.cpp
	src/Cache.cpp
	src/Client.cpp
	src/EventAccessors.cpp
	src/Logging.cpp
	src/MatrixClient.cpp
	src/PresenceEmitter.cpp
	src/UIA.cpp
	src/UserProfile.cpp
	src/UserSettings.cpp
	src/Utils.cpp
	)

include(FeatureSummary)

find_package(OpenSSL 1.1.0 REQUIRED)
find_package(MatrixClient 0.6.0 REQUIRED)
find_package(Olm 3 REQUIRED)
find_package(spdlog 1.0.0 CONFIG REQUIRED)
find_package(cmark REQUIRED 0.29.0)
find_package(nlohmann_json 3.2.0)

if(NOT LMDBXX_INCLUDE_DIR)
	find_path(LMDBXX_INCLUDE_DIR
		NAMES lmdb++.h
		PATHS /usr/include
		/usr/local/include
		$ENV{LIB_DIR}/include
		$ENV{LIB_DIR}/include/lmdbxx)

endif()
add_library(lmdbxx INTERFACE)
target_include_directories(lmdbxx INTERFACE ${LMDBXX_INCLUDE_DIR})
add_library(lmdbxx::lmdbxx ALIAS lmdbxx)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

qt5_wrap_cpp(MOC_HEADERS
	src/timeline/EventStore.h
	src/timeline/Permissions.h
	src/timeline/Reaction.h
	src/timeline/Timeline.h
	src/encryption/Olm.h
	src/encryption/DeviceVerificationFlow.h
	src/encryption/SelfVerificationStatus.h
	src/encryption/VerificationManager.h
	src/voip/AudioInfo.h
	src/voip/AudioDeviceControl.h
	src/voip/CallDevices.h
	src/voip/CallManager.h
	src/voip/AudioDevices.h
	src/voip/WebRTCSession.h
	src/Authentication.h
	src/CacheCryptoStructs.h
	src/Cache_p.h
	src/Client.h
	src/Config.h
	src/PresenceEmitter.h
	src/UIA.h
	src/UserProfile.h	
	src/UserSettings.h	
	src/Authentication.h
	src/UserInformation.h
)

set(HEADERS 
	timeline/EventStore.h
	timeline/Permissions.h
	timeline/Reaction.h
	timeline/Timeline.h
	encryption/Olm.h
	encryption/DeviceVerificationFlow.h
	encryption/SelfVerificationStatus.h
	encryption/VerificationManager.h
	voip/AudioInfo.h
	voip/AudioDeviceControl.h
	voip/CallDevices.h
	voip/CallManager.h
	voip/AudioDevices.h
	voip/WebRTCSession.h
	Authentication.h	
	CacheCryptoStructs.h
	CacheStructs.h
	Cache_p.h
	Client.h
	Config.h
	UserProfile.h	
	UserSettings.h	
	Authentication.h	
	Application.h
	Cache.h	
	EventAccessors.h
	Features.h
	Logging.h
	MatrixClient.h
	PresenceEmitter.h
	UIA.h
	Utils.h
	UserInformation.h
)

set(MATRIX_LIB_DEPS
	${SRC_FILES}
	${MOC_HEADERS})

set(DEP_LIBS 
	MatrixClient::MatrixClient
	cmark::cmark
	spdlog::spdlog
	Qt5::Gui
	Qt5::Network
	Qt5::QuickControls2
	Qt5::Svg
	nlohmann_json::nlohmann_json
	lmdbxx::lmdbxx
	liblmdb::lmdb)

if (PX_ACCOUNTS_INTEGRATION)
	message("PantherX Accounts/Secrets integration: enabled")
	find_package(CapnProto CONFIG REQUIRED)
	find_package(PxAccountsMatrixBridge REQUIRED)
	add_definitions(-DPX_ACCOUNTS_INTEGRATION)

	list(APPEND DEP_LIBS
				PxAccountsMatrixBridge::px-accounts-matrix-bridge
				CapnProto::capnp-rpc)
endif()

IF(STATIC_LIB)
	add_library (matrix-client-library ${MATRIX_LIB_DEPS})
	message(" + Build as static library.")
ELSE()
	add_library (matrix-client-library SHARED ${MATRIX_LIB_DEPS})
	message(" + Build as shared library.")
ENDIF()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
	set_target_properties(matrix-client-library PROPERTIES ENABLE_EXPORTS ON)
endif()


if (CMAKE_SYSTEM_NAME STREQUAL "Android")
	if(CIBA_AUTHENTICATION)
		list(APPEND DEP_LIBS px-auth-lib-cpp_${CMAKE_ANDROID_ARCH_ABI})
	endif()
else()
	pkg_check_modules(
		PULSE REQUIRED
		libpulse>=5.0
		libpulse-mainloop-glib>=0.9.16
	)
	if(CIBA_AUTHENTICATION)
		list(APPEND DEP_LIBS px-auth-lib-cpp)
	endif()
	find_package(Qt5 5.15 COMPONENTS Multimedia REQUIRED)
	list(APPEND DEP_LIBS Qt5::Multimedia
						${PULSE_LDFLAGS}
						${GLIB_LDFLAGS})
endif()

pkg_check_modules(coeurl REQUIRED IMPORTED_TARGET coeurl)
install(TARGETS matrix-client-library DESTINATION lib)
foreach(HEADER ${HEADERS})
	string(REGEX MATCH "(.*)[/\\]" DIR ${HEADER})
	install(FILES src/${HEADER} DESTINATION include/matrix-client-library/${DIR})
endforeach(HEADER {HEADERS})

if(TARGET PkgConfig::GSTREAMER)
	target_compile_definitions(matrix-client-library PUBLIC GSTREAMER_AVAILABLE)
	target_link_libraries(matrix-client-library PUBLIC PkgConfig::GSTREAMER)
endif()

target_link_libraries(matrix-client-library PUBLIC ${DEP_LIBS})

IF(BUILD_EXAMPLES)
	find_package(Qt5 5.15 COMPONENTS Widgets Qml QuickControls2 QuickWidgets Svg REQUIRED)
	add_executable(profileInfo examples/profileInfo.cpp)
	target_link_libraries(profileInfo matrix-client-library Qt5::Widgets Qt5::Network)

	if(VOIP)
		set(CMAKE_AUTOMOC ON)
		set(CMAKE_AUTORCC ON)
		set(CMAKE_AUTOUIC ON)
		add_executable(webrtc_example examples/webrtc.cpp
									examples/webrtc.qrc)
		target_link_libraries(webrtc_example matrix-client-library
											Qt5::Gui 
											Qt5::Widgets
											Qt5::Qml
											Qt5::QuickControls2
											Qt5::Network
											PkgConfig::GSTREAMER)
		target_compile_definitions(webrtc_example PRIVATE GSTREAMER_AVAILABLE)
	endif(VOIP)

	IF (PX_ACCOUNTS_INTEGRATION)
		add_executable(account_integration examples/account_integration.cpp)
		target_link_libraries(account_integration matrix-client-library
												  Qt5::Gui 
												  Qt5::Widgets
												  Qt5::Qml
												  Qt5::QuickControls2
												  Qt5::Network
												  PxAccountsMatrixBridge::px-accounts-matrix-bridge)
	ENDIF()
	
	message(" + \"examples\" will be built.")
ENDIF()

IF(BUILD_TESTS)
	set(CMAKE_AUTOMOC ON)
	find_package(Qt5Test)
	find_package(Qt5 5.15 COMPONENTS Widgets REQUIRED)

	enable_testing(true)
	add_executable(run_test tests/main.cpp tests/testrunner.h 
					tests/AuthenticationTest.h
					tests/ClientTest.h
					tests/UserSettingsTest.h)
	target_link_libraries(run_test PRIVATE PUBLIC Qt5::Test matrix-client-library Qt5::Gui Qt5::Network Qt5::Widgets)
	message(" + \"tests\" will be built.")
ENDIF()
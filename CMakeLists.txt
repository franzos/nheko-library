cmake_minimum_required(VERSION 3.16)
project(matrix-client-library LANGUAGES C CXX)
if (IOS)
	enable_language(OBJC)
	enable_language(OBJCXX)
endif()

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

option(VOIP "Whether to enable the voip support. disable this if you don't have the gstreamer" ON)
if(VOIP)
	message(" + VOIP: enabled.")
	pkg_check_modules(GSTREAMER REQUIRED IMPORTED_TARGET gstreamer-sdp-1.0>=1.18 gstreamer-webrtc-1.0>=1.18)
endif()

if (ANDROID OR IOS)
	option(GSTREAMER_PATH "Path to the gstreamer build directory" OFF)
	if (NOT GSTREAMER_PATH)
		if (ANDROID)
			set(GSTREAMER_PATH "/vendor/cerbero/build/dist/android_universal")
		elseif(IOS)
			set(GSTREAMER_PATH "$ENV{HOME}/Library/Developer/GStreamer/iPhone.sdk")
		endif()
	endif()
	message(" + GSTREAMER_PATH: ${GSTREAMER_PATH}")
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

find_package(Qt5 5.15 COMPONENTS Gui Network Multimedia QuickControls2 QuickWidgets Svg REQUIRED)
if (ANDROID)
	find_package(Qt5 5.15 COMPONENTS AndroidExtras REQUIRED)
elseif(IOS)
	find_package(Qt5 5.15 COMPONENTS MacExtras REQUIRED)
endif()

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
if (ANDROID)
	list(APPEND SRC_FILES 
				src/gstreamer/gstreamer_android.c
				src/gstreamer/gstreamer_android.cpp
				src/gstreamer/gstreamer_android.h
				)
elseif(IOS)
	list(APPEND SRC_FILES
			src/gstreamer/gst_ios_init.h
			src/gstreamer/gst_ios_init.m)
endif()

include(FeatureSummary)

find_package(OpenSSL 1.1.0 REQUIRED)
find_package(MatrixClient 0.8.2 REQUIRED)
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
	src/ChatPage.h
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
	ChatPage.h
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
	Qt5::Multimedia
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


if (ANDROID)
	list(APPEND DEP_LIBS Qt5::AndroidExtras)
	if(CIBA_AUTHENTICATION)
		list(APPEND DEP_LIBS px-auth-lib-cpp_${CMAKE_ANDROID_ARCH_ABI})
	endif()
elseif(IOS)
	list(APPEND DEP_LIBS Qt5::MacExtras)
	if(CIBA_AUTHENTICATION)
		list(APPEND DEP_LIBS px-auth-lib-cpp)
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
	list(APPEND DEP_LIBS ${PULSE_LDFLAGS}
			     ${GLIB_LDFLAGS})
endif()

install(TARGETS matrix-client-library DESTINATION lib)
foreach(HEADER ${HEADERS})
	string(REGEX MATCH "(.*)[/\\]" DIR ${HEADER})
	install(FILES src/${HEADER} DESTINATION include/matrix-client-library/${DIR})
endforeach(HEADER {HEADERS})

if(TARGET PkgConfig::GSTREAMER)
	target_compile_definitions(matrix-client-library PUBLIC GSTREAMER_AVAILABLE)
	target_link_libraries(matrix-client-library PUBLIC PkgConfig::GSTREAMER)
elseif (ANDROID)
	target_compile_definitions(matrix-client-library PUBLIC GSTREAMER_AVAILABLE)

	if(CMAKE_ANDROID_ARCH_ABI STREQUAL "armeabi-v7a")
		SET(GSTREAMER_ROOT ${GSTREAMER_PATH}/armv7)
	elseif (CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
		SET(GSTREAMER_ROOT ${GSTREAMER_PATH}/arm64)
	else()
		SET(GSTREAMER_ROOT ${GSTREAMER_PATH}/$${CMAKE_ANDROID_ARCH_ABI})
	endif()
	message(" + GSTREAMER_ROOT: ${GSTREAMER_ROOT}")

	target_include_directories(matrix-client-library PUBLIC
							 ${GSTREAMER_ROOT}/include/gstreamer-1.0 
							 ${GSTREAMER_ROOT}/lib/gstreamer-1.0/include
							 ${GSTREAMER_ROOT}/include/glib-2.0
							 ${GSTREAMER_ROOT}/lib/glib-2.0/include
							 ${GSTREAMER_ROOT}/include)
	
	target_link_libraries(matrix-client-library PUBLIC 
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstcoreelements.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgsttypefindfunctions.a
							# ${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstvideoconvert.a
							# ${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstvideoscale.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstvideoconvertscale.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstvideorate.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstvideofilter.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstautodetect.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstvideotestsrc.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstplayback.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstx264.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstvideoparsersbad.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstandroidmedia.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstrtp.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstrtpmanager.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstrtpmanagerbad.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstrtponvif.a							
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstopengl.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstqmlgl.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstwebrtc.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstnice.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstsdpelem.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstxingmux.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstrealmedia.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstlibav.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgsta52dec.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstamrnb.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstasf.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstaudioconvert.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstaudioresample.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstasf.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstavi.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstisomp4.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstmatroska.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstogg.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstwavparse.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstbz2.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstdtls.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstsrtp.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstopus.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstvolume.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstcompositor.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstvpx.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstrtsp.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstrtspclientsink.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstaudiotestsrc.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstopenh264.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstopensles.a
							${GSTREAMER_ROOT}/lib/gstreamer-1.0/libgstencoding.a
							
							#plugin libs
							${GSTREAMER_ROOT}/lib/liborc-0.4.a
							${GSTREAMER_ROOT}/lib/libgstgl-1.0.a
							${GSTREAMER_ROOT}/lib/libgraphene-1.0.a
							${GSTREAMER_ROOT}/lib/libpng16.a
							${GSTREAMER_ROOT}/lib/libjpeg.a
							${GSTREAMER_ROOT}/lib/libx264.a
							${GSTREAMER_ROOT}/lib/libgstvideo-1.0.a
							${GSTREAMER_ROOT}/lib/libgstrtp-1.0.a
							${GSTREAMER_ROOT}/lib/libgstsdp-1.0.a
							${GSTREAMER_ROOT}/lib/libgstwebrtc-1.0.a
							${GSTREAMER_ROOT}/lib/libgstwebrtcnice-1.0.a
							${GSTREAMER_ROOT}/lib/libwebrtc_audio_processing.a
							${GSTREAMER_ROOT}/lib/libopencore-amrnb.a
							${GSTREAMER_ROOT}/lib/libgstriff-1.0.a
							${GSTREAMER_ROOT}/lib/libFLAC.a
							${GSTREAMER_ROOT}/lib/libogg.a
							${GSTREAMER_ROOT}/lib/libavformat.a
							${GSTREAMER_ROOT}/lib/libbz2.a
							${GSTREAMER_ROOT}/lib/libfreetype.a
							${GSTREAMER_ROOT}/lib/libgstrtsp-1.0.a
							${GSTREAMER_ROOT}/lib/liba52.a
							${GSTREAMER_ROOT}/lib/libavcodec.a
							${GSTREAMER_ROOT}/lib/libavutil.a							
							${GSTREAMER_ROOT}/lib/libavfilter.a
							${GSTREAMER_ROOT}/lib/libswresample.a
							${GSTREAMER_ROOT}/lib/libopus.a
							${GSTREAMER_ROOT}/lib/libvpx.a
							${GSTREAMER_ROOT}/lib/libgstrtspserver-1.0.a
							${GSTREAMER_ROOT}/lib/libopenh264.a
							
							${GSTREAMER_ROOT}/lib/libgstaudio-1.0.a
							${GSTREAMER_ROOT}/lib/libgstpbutils-1.0.a
							${GSTREAMER_ROOT}/lib/libgsttag-1.0.a
							${GSTREAMER_ROOT}/lib/libgstphotography-1.0.a
							${GSTREAMER_ROOT}/lib/libgstcontroller-1.0.a
							${GSTREAMER_ROOT}/lib/libgstcodecparsers-1.0.a
							EGL GLESv2 OpenSLES

							${GSTREAMER_ROOT}/lib/libgio-2.0.a
							${GSTREAMER_ROOT}/lib/libglib-2.0.a
							${GSTREAMER_ROOT}/lib/libgobject-2.0.a
							${GSTREAMER_ROOT}/lib/libnice.a
							${GSTREAMER_ROOT}/lib/libgstapp-1.0.a
							${GSTREAMER_ROOT}/lib/libgstsctp-1.0.a
							${GSTREAMER_ROOT}/lib/libgstnet-1.0.a

							#gstreamer libs

							${GSTREAMER_ROOT}/lib/libgstbase-1.0.a
							${GSTREAMER_ROOT}/lib/libgstreamer-1.0.a
							${GSTREAMER_ROOT}/lib/libgmodule-2.0.a
							${GSTREAMER_ROOT}/lib/libgobject-2.0.a
							${GSTREAMER_ROOT}/lib/libglib-2.0.a
							${GSTREAMER_ROOT}/lib/libffi.a
							#LIBS += $${GSTREAMER_ROOT}/lib/libpcre.a
							${GSTREAMER_ROOT}/lib/libsrtp2.a
							${GSTREAMER_ROOT}/lib/libpcre2-8.a
							${GSTREAMER_ROOT}/lib/libiconv.a
							${GSTREAMER_ROOT}/lib/libintl.a
							${GSTREAMER_ROOT}/lib/libz.a
							)
elseif(IOS)
	target_compile_definitions(matrix-client-library PUBLIC GSTREAMER_AVAILABLE)

	target_include_directories(matrix-client-library 
							  PUBLIC ${GSTREAMER_PATH}/GStreamer.framework/Headers)
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

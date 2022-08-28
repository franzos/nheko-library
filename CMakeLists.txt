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
	pkg_check_modules(GSTREAMER REQUIRED IMPORTED_TARGET gstreamer-sdp-1.0>=1.18 gstreamer-webrtc-1.0>=1.18)
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
	src/CibaAuthentication.cpp
	src/timeline/EventStore.cpp
	src/timeline/Permissions.cpp
	src/timeline/Reaction.cpp
	src/timeline/Timeline.cpp
	src/encryption/Olm.cpp
	src/encryption/DeviceVerificationFlow.cpp
	src/encryption/SelfVerificationStatus.cpp
	src/encryption/VerificationManager.cpp
	src/px/PxCMManager.cpp
	src/voip/AudioInfo.cpp
	src/voip/AudioInputControl.cpp
	src/voip/CallDevices.cpp
	src/voip/CallManager.cpp
	src/voip/InputDevices.cpp
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
	src/RestRequest.cpp
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
	src/voip/AudioInputControl.h
	src/voip/CallDevices.h
	src/voip/CallManager.h
	src/voip/InputDevices.h
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
	src/CibaAuthentication.h
	src/RestRequest.h
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
	voip/AudioInputControl.h
	voip/CallDevices.h
	voip/CallManager.h
	voip/InputDevices.h
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
	RestRequest.h
	Cache.h	
	EventAccessors.h
	Logging.h
	MatrixClient.h
	PresenceEmitter.h
	UIA.h
	Utils.h
	CibaAuthentication.h
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
	message("PantherX Accounts and Secrets services integration set to enabled.")
	
	find_package(CapnProto CONFIG REQUIRED)
	set(INTERFACE_DIR interfaces)
	set(CAPNPC_SRC_PREFIX "${INTERFACE_DIR}" CACHE STRING "" FORCE)
	set(CAPNPC_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/interfaces" CACHE STRING "" FORCE)
	file(MAKE_DIRECTORY "${CAPNPC_OUTPUT_DIR}")
	capnp_generate_cpp(CapnpSources CapnpHeaders
		${INTERFACE_DIR}/Account.capnp
		${INTERFACE_DIR}/AccountReader.capnp
		${INTERFACE_DIR}/Secret.capnp)

	include_directories(
		${CapnProto_INCLUDE_DIRS}	
		${CAPNPC_OUTPUT_DIR}
		${CMAKE_CURRENT_BINARY_DIR})

	add_definitions(-DPX_ACCOUNTS_INTEGRATION)
	list(APPEND MATRIX_LIB_DEPS
				src/px/RpcClient.cpp
				src/px/PxAccountsClient.cpp
				src/px/PxSecretsClient.cpp
				${CapnpSources} ${CapnpHeaders})

	list(APPEND DEP_LIBS
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
	list(APPEND DEP_LIBS px-auth-lib-cpp_${CMAKE_ANDROID_ARCH_ABI})
else()
	pkg_check_modules(
		PULSE REQUIRED
		libpulse>=5.0
		libpulse-mainloop-glib>=0.9.16
	)
	find_package(Qt5 5.15 COMPONENTS Multimedia REQUIRED)
	list(APPEND DEP_LIBS Qt5::Multimedia px-auth-lib-cpp)
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
		add_executable(account_integration examples/account_integration.cpp
										   ${CapnpSources} ${CapnpHeaders})
		target_link_libraries(account_integration matrix-client-library
												  Qt5::Gui 
												  Qt5::Widgets
												  Qt5::Qml
												  Qt5::QuickControls2
												  Qt5::Network
												  CapnProto::capnp-rpc)
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
	target_link_libraries(run_test PRIVATE PUBLIC Qt5::Test matrix-client-library px-auth-lib-cpp Qt5::Gui Qt5::Network Qt5::Widgets)
	message(" + \"tests\" will be built.")
ENDIF()
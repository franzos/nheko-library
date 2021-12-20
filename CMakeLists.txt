cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard")
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "Require C++ standard to be supported")
set(CMAKE_POSITION_INDEPENDENT_CODE ON CACHE BOOL "compile as PIC by default")

include(CMakeDependentOption)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(GNUInstallDirs)
include_directories(src)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(PkgConfig REQUIRED) 
pkg_check_modules(libevent_core REQUIRED IMPORTED_TARGET libevent_core)
pkg_check_modules(libevent_pthreads REQUIRED IMPORTED_TARGET
		libevent_pthreads)

find_package(PkgConfig REQUIRED) 
pkg_check_modules(libcurl REQUIRED IMPORTED_TARGET libcurl)

find_package(spdlog 1.0.0 CONFIG REQUIRED)
find_package(LMDB REQUIRED)
find_package(Qt5 5.15 COMPONENTS Core Widgets LinguistTools Concurrent Svg Multimedia Qml QuickControls2 QuickWidgets REQUIRED)
find_package(Qt5QuickCompiler)
find_package(Qt5DBus)
find_package(Qt5Keychain REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
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

if(NOT (CMAKE_BUILD_TYPE OR CMAKE_CONFIGURATION_TYPES))
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
		"Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
		FORCE)
	message("Setting build type to '${CMAKE_BUILD_TYPE}'")
else(NOT (CMAKE_BUILD_TYPE OR CMAKE_CONFIGURATION_TYPES))
	message("Build type set to '${CMAKE_BUILD_TYPE}'")
endif(NOT (CMAKE_BUILD_TYPE OR CMAKE_CONFIGURATION_TYPES))

set(SPDLOG_DEBUG_ON false)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(SPDLOG_DEBUG_ON true)
else()
	set(SPDLOG_DEBUG_ON false)
endif()

#
# Declare source and header files.
#
set(SRC_FILES
	src/timeline/EventStore.cpp
	src/timeline/Reaction.cpp
	src/encryption/Olm.cpp
	src/AvatarProvider.cpp
	src/Cache.cpp
	src/ChatPage.cpp
	src/EventAccessors.cpp
	src/Logging.cpp
	src/MatrixClient.cpp
	src/MxcImageProvider.cpp
	src/UserSettingsPage.cpp
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
	src/timeline/Reaction.h
	src/encryption/Olm.h
	src/AvatarProvider.h
	src/CacheCryptoStructs.h
	src/Cache_p.h
	src/ChatPage.h
	src/UserSettingsPage.h
)


set(PX_MATRIX_LIB_DEPS
	${SRC_FILES}
	${MOC_HEADERS})


add_library (px-matrix-client-library ${OS_BUNDLE} ${PX_MATRIX_LIB_DEPS})

if (HAVE_BACKTRACE_SYMBOLS_FD AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
	set_target_properties(px-matrix-client-library PROPERTIES ENABLE_EXPORTS ON)
endif()

set(DEP_LIBS 
	Qt5::DBus
	MatrixClient::MatrixClient
	cmark::cmark
	spdlog::spdlog
	Qt5::Widgets
	Qt5::Concurrent
	Qt5::QuickControls2
	Qt5::QuickWidgets
	qt5keychain
	nlohmann_json::nlohmann_json
	lmdbxx::lmdbxx
	liblmdb::lmdb
	PkgConfig::coeurl)

find_package(PkgConfig REQUIRED)
pkg_check_modules(coeurl REQUIRED IMPORTED_TARGET coeurl)
target_link_libraries(px-matrix-client-library PUBLIC ${DEP_LIBS})

add_executable(nheko examples/example.cpp)
target_link_libraries(nheko px-matrix-client-library)

WARHAWK_DIR = $(shell cd ../..; pwd)
BUILD_DIR = ${WARHAWK_DIR}/../build-warhawk-reborn

WARHAWK_BUILD = $(shell cd ${WARHAWK_DIR}; git rev-parse --short HEAD)

WARHAWK_VERSION = $(shell grep '^const std::string Version = ' ../../src/main.cpp | sed -e 's/^const std::string Version = //' -e 's/\"//g' -e 's/;//')

all : macos

clean :
	rm -rf ${BUILD_DIR}

macos : 
	@echo WARHAWK_VERSION = ${WARHAWK_VERSION}
	@echo WARHAWK_BUILD = ${WARHAWK_BUILD}
	@echo WARHAWK_DIR = ${WARHAWK_DIR}
	@echo BUILD_DIR = ${BUILD_DIR}
	if [ ! -d ${BUILD_DIR} ]; then \
		mkdir -p ${BUILD_DIR}; \
		cd ${BUILD_DIR};       \
		cmake ${WARHAWK_DIR};  \
	fi
	cd ${BUILD_DIR}; cmake --build .
	cd ${BUILD_DIR}; rm -rf *.dmg
	cd ${BUILD_DIR}; mkdir app; cp src/warhawk-reborn app
	cd ${BUILD_DIR}; ${WARHAWK_DIR}/packages/macos/create-dmg \
		--volname "WARHAWK REBORN ${WARHAWK_VERSION}.${WARHAWK_BUILD} Installer" \
		"warhawk-${WARHAWK_VERSION}.${WARHAWK_BUILD}.dmg" \
		"${BUILD_DIR}/app"

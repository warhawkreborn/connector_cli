NAME := warhawkreborn
WARHAWK_DIR = $(shell cd ../..; pwd)
BUILD_DIR = ${WARHAWK_DIR}/../build-${NAME}

WARHAWK_BUILD = $(shell cd ${WARHAWK_DIR}; git rev-parse --short HEAD)

WARHAWK_VERSION = $(shell grep '^const std::string Version = ' ../../src/main.cpp | sed -e 's/^const std::string Version = //' -e 's/\"//g' -e 's/;//')

all : build_pkg

clean :
	rm -rf ${BUILD_DIR}

create_build_dir :
	@echo WARHAWK_VERSION = ${WARHAWK_VERSION}
	@echo WARHAWK_BUILD = ${WARHAWK_BUILD}
	@echo WARHAWK_DIR = ${WARHAWK_DIR}
	@echo BUILD_DIR = ${BUILD_DIR}
	if [ ! -d ${BUILD_DIR} ]; then \
		mkdir -p ${BUILD_DIR}; \
		cd ${BUILD_DIR};       \
	fi

build : create_build_dir
	cd ${BUILD_DIR}; cmake ${WARHAWK_DIR};
	cd ${BUILD_DIR}; cmake --build .

build_dmg : build
	cd ${BUILD_DIR}; rm -rf app *.dmg
	cd ${BUILD_DIR}; mkdir -p app; cp src/${NAME} app
	cd ${BUILD_DIR}; ${WARHAWK_DIR}/packages/macos/create-dmg \
		--volname "WARHAWK REBORN ${WARHAWK_VERSION}.${WARHAWK_BUILD} Installer" \
		"warhawk-${WARHAWK_VERSION}.${WARHAWK_BUILD}.dmg" \
		"${BUILD_DIR}/app"

build_pkg : build
	
	cd ${BUILD_DIR}; rm -rf pkg *.pkg
	cd ${BUILD_DIR}; mkdir -p pkg; cp src/${NAME} pkg/
	cd ${BUILD_DIR}; productbuild --root ${BUILD_DIR}/pkg /usr/local/bin ${BUILD_DIR}/${NAME}-${WARHAWK_VERSION}-${WARHAWK_BUILD}.pkg

NAME        := WarHawkReborn
WARHAWK_DIR := $(shell cd ../..; pwd)
BUILD_DIR   := ${WARHAWK_DIR}/../build-${NAME}
GIT_HASH    := $(shell cd ${WARHAWK_DIR}; git rev-parse --short HEAD)

WARHAWK_VERSION = $(shell grep 'WarHawkReborn VERSION' ../../src/CMakeLists.txt | awk '{ print $$4 }')

all : build_pkg

clean :
	rm -rf ${BUILD_DIR}

create_build_dir :
	@echo BUILD_DIR = ${BUILD_DIR}
	@echo WARHAWK_DIR = ${WARHAWK_DIR}
	@echo WARHAWK_VERSION = ${WARHAWK_VERSION}
	@echo GIT_HASH = ${GIT_HASH}
	if [ ! -d ${BUILD_DIR} ]; then \
		mkdir -p ${BUILD_DIR}; \
		cd ${BUILD_DIR};       \
	fi

build : create_build_dir
	cd ${BUILD_DIR}; cmake -DGIT_HASH=$${GIT_HASH} ${WARHAWK_DIR};
	cd ${BUILD_DIR}; cmake --build .

build_dmg : build
	cd ${BUILD_DIR}; rm -rf app *.dmg
	cd ${BUILD_DIR}; mkdir -p app; cp src/${NAME} app
	cd ${BUILD_DIR}; ${WARHAWK_DIR}/packages/macos/create-dmg \
		--volname "WARHAWK REBORN ${WARHAWK_VERSION}.${GIT_HASH} Installer" \
		"${NAME}-${WARHAWK_VERSION}.${GIT_HASH}.dmg" \
		"${BUILD_DIR}/app"

build_pkg : build
	
	cd ${BUILD_DIR}; rm -rf pkg *.pkg
	cd ${BUILD_DIR}; mkdir -p pkg; cp src/warhawkreborn pkg/
	cd ${BUILD_DIR}; productbuild --root ${BUILD_DIR}/pkg /usr/local/bin ${BUILD_DIR}/${NAME}-${WARHAWK_VERSION}-${GIT_HASH}.pkg

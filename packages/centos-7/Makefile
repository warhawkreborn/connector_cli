##########################################################################
# Build an RPM from the current spec file and FAVOR directory tree.      #
#                                                                        #
# Note: The RPM and Makefile are set up to GENERATE RPMS as a non-root   #
# user in the user's home directory.  Of course to INSTALL the resulting #
# binary RPM the user has to log in as root.                             #
##########################################################################

SPEC_FILE := ./warhawk-reborn.spec
VERSION_FILE := ./warhawk-reborn.spec

RPMDIR := rpmbuild

TOP_DIR := $(shell pwd)/../..

NAME    := $(shell grep '^Name:'   < $(SPEC_FILE) | sed -e 's/^Name: //')
VERSION := $(shell grep '^const std::string Version = ' ../../src/main.cpp | sed -e 's/^const std::string Version = //' -e 's/\"//g' -e 's/;//')
RELEASE := $(shell git rev-parse --short HEAD)

SOURCE_RPM_DIR := $(HOME)/$(RPMDIR)/SOURCES/

all : rpm

subversion rpm : check_release build_srpm_files build_rpm_files

check_release :
	@if [ "$(RELEASE)"x = "x" ]; \
	   then \
		echo "Please specifiy RELEASE"; \
		exit 1 \
	    else \
		exit 0; \
	fi
	@echo "Making $(NAME)-$(VERSION)-$(RELEASE) (S)RPM..."

build_rpm_files : build_srpm_files
	cd $(SOURCE_RPM_DIR); rpmbuild -ba `basename $(SPEC_FILE)`

build_srpm_files : $(HOME)/.rpmmacros $(HOME)/$(RPMDIR) $(SPEC_FILE)
	@echo "*** Building SRPM for $(NAME)-$(VERSION)-$(RELEASE)"
	rm -rf $(SOURCE_RPM_DIR)
	mkdir -p $(SOURCE_RPM_DIR)
	cd $(TOP_DIR); #make clean
	sed -e 's/@VERSION@/$(VERSION)/g' -e 's/@RELEASE@/$(RELEASE)/g' < $(SPEC_FILE) > $(SOURCE_RPM_DIR)/$(SPEC_FILE)
	cd $(SOURCE_RPM_DIR); cp -r $(TOP_DIR) $(NAME)-$(VERSION); \
	   tar cfz $(NAME)-$(VERSION).tar.gz $(NAME)-$(VERSION) --exclude .git --exclude build; \
	   rm -rf $(NAME)-$(VERSION)

$(HOME)/.rpmmacros :
	@if [ ! -f $(HOME)/.rpmmacros ]; \
	   then \
	   echo "Creating $(HOME)/.rpmmacros"; \
	     rpmdev-setuptree; \
	   fi

$(HOME)/$(RPMDIR) :
	@echo "*** Testing for rpmbuild directory...."

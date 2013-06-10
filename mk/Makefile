USE_BRANDING := yes
IMPORT_BRANDING := yes
include $(B_BASE)/common.mk
include $(B_BASE)/rpmbuild.mk

XCA_RELEASE := 1
XCA_VERSION := 2.4.1

PACKAGE_NAME=xen-crashdump-analyser
SPECFILE := $(PACKAGE_NAME).spec

.PHONY: build
build: $(RPM_DIRECTORIES) srpm  $(MY_SOURCES)/MANIFEST
	$(RPMBUILD) --rebuild $(MY_OUTPUT_DIR)/SRPMS/$(PACKAGE_NAME)-*.src.rpm
	mkdir -p $(MY_MAIN_PACKAGES)
	cp $(MY_OUTPUT_DIR)/RPMS/*/$(PACKAGE_NAME)-*.rpm $(MY_MAIN_PACKAGES)

$(MY_SOURCES)/MANIFEST: $(MY_OUTPUT_DIR)/SRPMS
	mkdir -p $(MY_SOURCES)
	/bin/sh ./srpms-to-manifest $(PACKAGE_NAME) $(MY_OUTPUT_DIR)/SRPMS > $@

REPO = $(call git_loc,$(PACKAGE_NAME))

$(SPECFILE): $(SPECFILE).in
	sed "/%description/ r ../README" < $< > $@.tmp
	sed -i "s/@XCA_VERSION@/$(XCA_VERSION)/g" $@.tmp
	sed -i "s/@RPM_RELEASE@/$(XCA_RELEASE)/g" $@.tmp
	mv -f $@.tmp $@

.PHONY: srpm
srpm: $(SPECFILE)
	mkdir -p $(RPM_SOURCESDIR)
	cd $(REPO) && git archive --format=tar --prefix=$(PACKAGE_NAME)/ \
		`cat .git/refs/heads/master` \
		| gzip > $(RPM_SOURCESDIR)/$(PACKAGE_NAME)-$(XCA_VERSION).tar.gz
	mkdir -p $(RPM_SRPMSDIR)
	$(RPMBUILD) -bs $(SPECFILE)

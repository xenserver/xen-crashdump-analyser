include $(B_BASE)/common.mk
include $(B_BASE)/rpmbuild.mk

XCA_RELEASE := 1
XCA_VERSION := 2.4.1

PACKAGE_NAME := xen-crashdump-analyser
SPECFILE := $(PACKAGE_NAME).spec

.PHONY: build
build: $(RPM_DIRECTORIES) srpm
	$(RPMBUILD) --rebuild $(MY_OUTPUT_DIR)/SRPMS/$(PACKAGE_NAME)-*.src.rpm

REPO = $(call git_loc,$(PACKAGE_NAME))

$(SPECFILE): $(SPECFILE).in
	{ set -e; sed \
		-e "/%description/ r ../README" \
		-e "/%changelog/ r ../ChangeLog" \
		-e "s/@XCA_VERSION@/$(XCA_VERSION)/g" \
		-e "s/@XCA_RELEASE@/$(XCA_RELEASE)/g" \
		< $< > $@.tmp; \
	mv -f $@.tmp $@; }

.PHONY: srpm
srpm: $(SPECFILE) $(RPM_DIRECTORIES)
	mkdir -p $(RPM_SOURCESDIR)
	cd $(REPO) && git archive --format=tar --prefix=$(PACKAGE_NAME)/ \
		`cat .git/refs/heads/master` \
		| gzip > $(RPM_SOURCESDIR)/$(PACKAGE_NAME)-$(XCA_VERSION).tar.gz
	mkdir -p $(RPM_SRPMSDIR)
	$(RPMBUILD) -bs $(SPECFILE)

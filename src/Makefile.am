AM_CPPFLAGS = \
	-DSCIM_CANNA_LOCALEDIR=\"$(datadir)/locale\" \
	-DSCIM_ICONDIR=\"@SCIM_ICONDIR@\"
INCLUDE	= \
	-I$(top_builddir) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src 

noinst_HEADERS = \
	canna_jrkanji.h \
	intl.h \
	scim_canna_imengine_factory.h \
	scim_canna_imengine.h \
	scim_canna_prefs.h

moduledir = @SCIM_MODULEDIR@/IMEngine
module_LTLIBRARIES = canna.la

canna_la_SOURCES = \
	canna_jrkanji.cpp \
	scim_canna_imengine_factory.cpp \
	scim_canna_imengine.cpp

canna_la_CFLAGS = @SCIM_CFLAGS@  @SCIM_DEBUG_FLAGS@ @CANNA_CFLAGS@
canna_la_CXXFLAGS = @SCIM_CFLAGS@ @SCIM_DEBUG_FLAGS@

canna_la_LDFLAGS = \
	-avoid-version \
	-rpath $(moduledir) \
	-module \
	@LIBTOOL_EXPORT_OPTIONS@ \
	@SCIM_LIBS@ \
	@CANNA_LIBS@

if SCIM_BUILD_SETUP
SETUP_MODULE = canna-imengine-setup.la
endif

setupmoduledir           = @SCIM_MODULEDIR@/SetupUI
setupmodule_LTLIBRARIES  = $(SETUP_MODULE)

canna_imengine_setup_la_SOURCES = scim_canna_imengine_setup.cpp

canna_imengine_setup_la_CFLAGS  = @SCIM_GTKUTILS_CFLAGS@ @SCIM_CFLAGS@ @SCIM_DEBUG_FLAGS@

canna_imengine_setup_la_CXXFLAGS= @SCIM_GTKUTILS_CFLAGS@ @SCIM_CFLAGS@ @SCIM_DEBUG_FLAGS@

canna_imengine_setup_la_LDFLAGS = -avoid-version \
                                   -rpath $(setupmoduledir) \
                                   -module \
                                   @LIBTOOL_EXPORT_OPTIONS@ \
                                   @SCIM_GTKUTILS_LIBS@ \
                                   @INTLLIBS@ \
                                   @SCIM_LIBS@

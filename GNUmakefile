# makefile for a gtk interface lib

firstTarget: all

-include make.config

VERSION_MAJOR       = 3.5
VERSION_MINOR       = 0
VERSION             = $(VERSION_MAJOR).$(VERSION_MINOR)
DEFINES				+= VERSION='"$(VERSION)"' BOOST_COROUTINES_NO_DEPRECATION_WARNING CRYPTOPP_ENABLE_NAMESPACE_WEAK

CXX					?= clang++
PACKAGES			+= x11 gtk+-3.0 gtk+-x11-3.0 libcanberra-gtk3 zlib libcrypto++

# main build variables
# CXX                 ?= clangc++
CXX					= clang++
CXXFLAGS            += -I. -pthread -std=c++17
WARNINGS            += all no-multichar
CXXFLAGS            += -g
LD                  ?= $(CXX)
LDFLAGS				= -g -pthread

SVN					= $(shell which svn)
MRC					= $(shell which mrc)

# Use the DEBUG flag to build debug versions of the code
DEBUG               ?= 0

ifneq ($(PACKAGES),)
CXXFLAGS			+= $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --cflags $(PACKAGES))
LDFLAGS				+= $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs $(PACKAGES))
endif

ZEEP_DIR			?= ../libzeep-svn/

ZEEP_INC_DIR		= $(ZEEP_DIR:%=%/include)
ZEEP_LIB_DIR		= $(ZEEP_DIR:%=%/lib)

ZEEP_LIBS			= webapp rest http el xml generic
ZEEP_LIBS			:= $(ZEEP_LIBS:%=zeep-%)

BOOST_INC_DIR       = $(BOOST:%=%/)
BOOST_LIB_DIR       = $(BOOST:%=%/stage/lib)

BOOST_LIBS          = system thread filesystem regex random program_options date_time locale coroutine context
BOOST_LIBS          := $(BOOST_LIBS:%=boost_%$(BOOST_LIB_SUFFIX))

CXXFLAGS			+= -ILib -Ilibassh
LIBS                := $(ZEEP_LIBS) $(BOOST_LIBS) m $(LIBS) dl stdc++ util stdc++fs # attr
LDFLAGS             += $(ZEEP_LIB_DIR:%=-L%) $(BOOST_LIB_DIR:%=-L%) $(LIBS:%=-l%) -g

ifeq "$(DEBUG)" "1"
DEFINES				+= DEBUG
else
CXXFLAGS			+= -O2
DEFINES				+= NDEBUG
endif

# targets

VPATH += Sources:libassh/src:Lib:Lib/Gtk:Lib/X11:Resources

CXXFLAGS			+= $(ZEEP_INC_DIR:%=-I%) $(BOOST_INC_DIR:%=-I%) -ILib/Gtk -ILib/X11
CXXFLAGS			+= $(DEFINES:%=-D%) $(WARNINGS:%=-W%)

OBJDIR = obj
ifeq "$(DEBUG)" "1"
	OBJDIR	:= $(OBJDIR).dbg
endif

$(OBJDIR):
	mkdir -p $(OBJDIR)

RESOURCES = \
	Menus/terminal-window-menu.xml \
	Alerts/only-openssh-server-supported.xml \
	Alerts/algo-unsupported.xml \
	Alerts/failed-to-install-public-key.xml \
	Alerts/close-session-alert.xml \
	Alerts/error-alert.xml \
	Alerts/old.xml \
	Alerts/about-alert.xml \
	Alerts/host-key-changed-alert.xml \
	Alerts/unknown-host-alert.xml \
	Alerts/expired.xml \
	Alerts/installed-public-key.xml \
	Alerts/exception-alert.xml \
	Alerts/close-all-sessions-alert.xml \
	Icons/appicon-small.ico \
	close.png \
	version.rc \
	templates/status.html \
	templates/style.css \
	templates/index.html \
	templates/error.html \
	salt.desktop \
	Icons/appicon.png \
	Dialogs/connect-dialog.xml \
	Dialogs/color-picker.xml \
	Dialogs/http-proxy-dialog.xml \
	Dialogs/prefs-dialog.xml \
	Dialogs/find-dialog.xml \
	Dialogs/auth-dialog.xml \
	Dialogs/port-forwarding-dialog.xml \
	Dialogs/socks5-proxy-dialog.xml \
	Dialogs/add-totp-hash.xml \
	strings.xml.nl \
	MakeRes.pl \
	Icons/appicon-small.png \
	Icons/appicon.ico
	

$(OBJDIR)/MSalt_rsrc.o: $(RESOURCES)
	$(MRC) -o $@ Resources/ --verbose

OBJECTS = \
	$(OBJDIR)/MAddTOTPHashDialog.o \
	$(OBJDIR)/MAuthDialog.o \
	$(OBJDIR)/MConnectDialog.o \
	$(OBJDIR)/MHTTPProxy.o \
	$(OBJDIR)/MPortForwardingDialog.o \
	$(OBJDIR)/MPreferencesDialog.o \
	$(OBJDIR)/MSaltApp.o \
	$(OBJDIR)/MSearchPanel.o \
	$(OBJDIR)/MTerminalBuffer.o \
	$(OBJDIR)/MTerminalChannel.o \
	$(OBJDIR)/MTerminalColours.o \
	$(OBJDIR)/MTerminalView.o \
	$(OBJDIR)/MTerminalWindow.o \
	$(OBJDIR)/channel.o \
	$(OBJDIR)/connection.o \
	$(OBJDIR)/connection_pool.o \
	$(OBJDIR)/debug.o \
	$(OBJDIR)/error.o \
	$(OBJDIR)/key_exchange.o \
	$(OBJDIR)/packet.o \
	$(OBJDIR)/port_forwarding.o \
	$(OBJDIR)/posix_ssh_agent_impl.o \
	$(OBJDIR)/proxy_cmd.o \
	$(OBJDIR)/sftp_channel.o \
	$(OBJDIR)/ssh_agent.o \
	$(OBJDIR)/terminal_channel.o \
	$(OBJDIR)/x11_channel.o \
	$(OBJDIR)/MAcceleratorTable.o \
	$(OBJDIR)/MAlerts.o \
	$(OBJDIR)/MAnimation.o \
	$(OBJDIR)/MApplication.o \
	$(OBJDIR)/MCanvas.o \
	$(OBJDIR)/MClipboard.o \
	$(OBJDIR)/MColor.o \
	$(OBJDIR)/MColorPicker.o \
	$(OBJDIR)/MController.o \
	$(OBJDIR)/MControls.o \
	$(OBJDIR)/MDevice.o \
	$(OBJDIR)/MDialog.o \
	$(OBJDIR)/MDocApplication.o \
	$(OBJDIR)/MDocument.o \
	$(OBJDIR)/MDocWindow.o \
	$(OBJDIR)/MError.o \
	$(OBJDIR)/MFile.o \
	$(OBJDIR)/MHandler.o \
	$(OBJDIR)/MLib.o \
	$(OBJDIR)/MMenu.o \
	$(OBJDIR)/MPreferences.o \
	$(OBJDIR)/MPrinter.o \
	$(OBJDIR)/MResources.o \
	$(OBJDIR)/MStrings.o \
	$(OBJDIR)/MTypes.o \
	$(OBJDIR)/MUnicode.o \
	$(OBJDIR)/MUtils.o \
	$(OBJDIR)/MView.o \
	$(OBJDIR)/MWindow.o \
	$(OBJDIR)/MGtkAlerts.o \
	$(OBJDIR)/MGtkAnimationImpl.o \
	$(OBJDIR)/MGtkApplicationImpl.o \
	$(OBJDIR)/MGtkCanvasImpl.o \
	$(OBJDIR)/MGtkClipboardImpl.o \
	$(OBJDIR)/MGtkControlsImpl.o \
	$(OBJDIR)/MGtkDeviceImpl.o \
	$(OBJDIR)/MGtkDialogImpl.o \
	$(OBJDIR)/MGtkDocClosedNotifier.o \
	$(OBJDIR)/MGtkError.o \
	$(OBJDIR)/MGtkFile.o \
	$(OBJDIR)/MGtkList.o \
	$(OBJDIR)/MGtkMain.o \
	$(OBJDIR)/MGtkMenu.o \
	$(OBJDIR)/MGtkResources.o \
	$(OBJDIR)/MGtkSound.o \
	$(OBJDIR)/MGtkUtils.o \
	$(OBJDIR)/MGtkWidgetMixin.o \
	$(OBJDIR)/MGtkWindowImpl.o \
	$(OBJDIR)/MPrimary.o \
	$(OBJDIR)/bsd-closefrom.o \
	$(OBJDIR)/MSalt_rsrc.o

salt: $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

install: salt
	install salt /usr/local/bin/salt

-include $(OBJECTS:%.o=%.d)

$(OBJECTS:.o=.d):

$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	@ echo ">>" $<
	@ $(CXX) -MD -c -o $@ $< $(CFLAGS) $(CXXFLAGS)

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	@ echo ">>" $<
	@ $(CXX) -MD -c -o $@ $< $(CFLAGS) $(CXXFLAGS)

#WCREV				= $(shell svn info | grep '^Revision' | sed -e s'/Revision: //')
WCREV				= $(shell svnversion)
WCNOW				= $(shell svn info | sed -n -e 's/.*(.*, \(.*\))/\1/p')
WCNOW_DAY			= $(shell echo $(WCNOW) | awk '{print $$1}')
WCNOW_MONTH			= $(shell echo $(WCNOW) | awk '{print $$2}')
WCNOW_YEAR			= $(shell echo $(WCNOW) | awk '{print $$3}')

Sources/MSaltVersion.h: Sources/MSaltVersion.h.tmpl GNUmakefile
	sed -e 's/\$$WCREV\$$/$(WCREV)/' \
	    -e 's/\$$WCNOW=%Y\$$/$(WCNOW_YEAR)/' \
	    -e 's/\$$WCNOW=%m\$$/$(WCNOW_MONTH)/' \
	    -e 's/\$$WCNOW=%d\$$/$(WCNOW_DAY)/' \
		< Sources/MSaltVersion.h.tmpl > $@


clean:
	rm -rf salt $(OBJDIR)/*

all: salt

test:

dist-clean: clean

FORCE:
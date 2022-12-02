//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MGtkLib.hpp"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>

#include <boost/algorithm/string.hpp>
#include <mcfp/mcfp.hpp>

#include "MApplication.hpp"
#include "MError.hpp"
#include "MGtkApplicationImpl.hpp"

#include "../../src/revision.hpp"

using namespace std;
namespace fs = std::filesystem;
namespace ba = boost::algorithm;

#define MGDBUS_SERVER_NAME "com.hekkelman.GDBus.SaltServer"
#define MGDBUS_SERVER_OBJECT_NAME "/com/hekkelman/GDBus/SaltObject"

// bool gQuit = false;

MGtkApplicationImpl *MGtkApplicationImpl::sInstance = nullptr;

class MGDbusServer
{
  public:
	MGDbusServer(const string &inOpenParameter)
		: mIntrospectionData(nullptr)
		, mOwnerId(0)
		, mRegistrationID(0)
		, mWatcherID(0)
		, mOpenParameter(inOpenParameter)
	{
		static const char my_server_xml[] =
			"<node>"
			"  <interface name='" MGDBUS_SERVER_NAME "'>"
			"    <annotation name='org.gtk.GDBus.Annotation' value='OnInterface'/>"
			"    <annotation name='org.gtk.GDBus.Annotation' value='AlsoOnInterface'/>"
			"    <method name='New'>"
			"      <annotation name='org.gtk.GDBus.Annotation' value='OnMethod'/>"
			"      <arg type='s' name='result' direction='out'/>"
			"    </method>"
			"    <method name='Open'>"
			"      <annotation name='org.gtk.GDBus.Annotation' value='OnMethod'/>"
			"      <arg type='s' name='url' direction='in'/>"
			"      <arg type='s' name='result' direction='out'/>"
			"    </method>"
			"    <method name='Connect'>"
			"      <annotation name='org.gtk.GDBus.Annotation' value='OnMethod'/>"
			"      <arg type='s' name='result' direction='out'/>"
			"    </method>"
			"  </interface>"
			"</node>";

		mIntrospectionData = g_dbus_node_info_new_for_xml(my_server_xml, nullptr);
		if (mIntrospectionData == nullptr)
			throw runtime_error("failed to parse introspection data");

		mOwnerId = g_bus_own_name(G_BUS_TYPE_SESSION, MGDBUS_SERVER_NAME,
			G_BUS_NAME_OWNER_FLAGS_NONE, HandleBusAcquired, HandleNameAcquired, HandleNameLost,
			this, nullptr);
	}

	~MGDbusServer()
	{
		if (mWatcherID != 0)
			g_bus_unwatch_name(mWatcherID);

		if (mOwnerId != 0)
			g_bus_unown_name(mOwnerId);

		if (mIntrospectionData != nullptr)
			g_dbus_node_info_unref(mIntrospectionData);
	}

	bool IsServer() const
	{
		return mOwnerId != 0 and mRegistrationID != 0;
	}

  private:
	void BusAcquired(GDBusConnection *connection, const char *name)
	{
		mRegistrationID = g_dbus_connection_register_object(
			connection, MGDBUS_SERVER_OBJECT_NAME,
			mIntrospectionData->interfaces[0],
			&sInterfaceVTable, this, nullptr, nullptr);
		assert(mRegistrationID > 0);
	}

	void NameAcquired(GDBusConnection *connection, const char *name)
	{
PRINT(("Name: %s", name));
		if (mOpenParameter.empty())
			gApp->DoNew();
		else
			gApp->Open(mOpenParameter);
	}

	void NameLost(GDBusConnection *connection, const char *name)
	{
PRINT(("Name Lost: %s", name));
		mWatcherID = g_bus_watch_name(G_BUS_TYPE_SESSION, MGDBUS_SERVER_NAME,
			G_BUS_NAME_WATCHER_FLAGS_NONE, HandleNameAppeared, HandleNameVanished,
			this, nullptr);
	}

	void NameAppeared(GDBusConnection *connection, const gchar *name, const char *name_owner)
	{
PRINT(("Name Appeared: %s", name));
		if (mOpenParameter.empty())
		{
			g_dbus_connection_call(
				connection,
				MGDBUS_SERVER_NAME, MGDBUS_SERVER_OBJECT_NAME, MGDBUS_SERVER_NAME,
				"New",
				nullptr,
				nullptr,
				G_DBUS_CALL_FLAGS_NONE,
				-1,
				nullptr,
				&HandleAsyncReady,
				nullptr);
		}
		else
		{
			g_dbus_connection_call(
				connection,
				MGDBUS_SERVER_NAME, MGDBUS_SERVER_OBJECT_NAME, MGDBUS_SERVER_NAME,
				"Open",
				g_variant_new("(s)", mOpenParameter.c_str()),
				nullptr,
				G_DBUS_CALL_FLAGS_NONE,
				-1,
				nullptr,
				&HandleAsyncReady,
				nullptr);
		}
	}

	void AsyncReady(GDBusConnection *connection, GVariant *result, GError *error)
	{
		//		PRINT(("Ready, result is %s", result ? "not null" : "null"));
		if (error)
		{
			cerr << error->message << endl;
			//			PRINT(("Error: '%s'", error->message));
			g_error_free(error);
		}
	}

	void NameVanished(GDBusConnection *connection, const gchar *name)
	{
PRINT(("Name Vanished: %s", name));
		// something fishy... just open
		if (mOpenParameter.empty())
			gApp->DoNew();
		else
			gApp->Open(mOpenParameter);
	}

	void MethodCall(GDBusConnection *connection, const gchar *sender,
		const gchar *object_path, const gchar *interface_name,
		const gchar *method_name, GVariant *parameters,
		GDBusMethodInvocation *invocation)
	{
		try
		{
			if (strcmp(method_name, "Open") == 0)
			{
				const gchar *gurl;
				g_variant_get(parameters, "(&s)", &gurl);
				string url(gurl ? gurl : "");

				gApp->Open(url);
			}
			else if (strcmp(method_name, "Connect") == 0)
				gApp->ProcessCommand('Conn', nullptr, 0, 0);
			else if (strcmp(method_name, "New") == 0)
				gApp->DoNew();
			else
				throw runtime_error("unimplemented DBus Method");

			g_dbus_method_invocation_return_value(invocation,
				g_variant_new("(s)", "ok"));
		}
		catch (exception &e)
		{
			g_dbus_method_invocation_return_value(invocation,
				g_variant_new("(s)", e.what()));
		}
	}

	static void HandleBusAcquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
	{
		MGDbusServer *server = reinterpret_cast<MGDbusServer *>(user_data);
		server->BusAcquired(connection, name);
	}

	static void HandleNameAcquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
	{
		MGDbusServer *server = reinterpret_cast<MGDbusServer *>(user_data);
		server->NameAcquired(connection, name);
	}

	static void HandleNameLost(GDBusConnection *connection, const gchar *name, gpointer user_data)
	{
		MGDbusServer *server = reinterpret_cast<MGDbusServer *>(user_data);
		server->NameLost(connection, name);
	}

	static void HandleMethodCall(GDBusConnection *connection, const gchar *sender,
		const gchar *object_path, const gchar *interface_name,
		const gchar *method_name, GVariant *parameters,
		GDBusMethodInvocation *invocation, gpointer user_data)
	{
		MGDbusServer *server = reinterpret_cast<MGDbusServer *>(user_data);
		server->MethodCall(connection, sender, object_path, interface_name, method_name, parameters, invocation);
	}

	static void HandleNameAppeared(GDBusConnection *connection, const gchar *name, const char *name_owner, gpointer user_data)
	{
		MGDbusServer *server = reinterpret_cast<MGDbusServer *>(user_data);
		server->NameAppeared(connection, name, name_owner);
	}

	static void HandleNameVanished(GDBusConnection *connection, const gchar *name, gpointer user_data)
	{
		MGDbusServer *server = reinterpret_cast<MGDbusServer *>(user_data);
		server->NameVanished(connection, name);
	}

	static void HandleAsyncReady(GObject *source_object, GAsyncResult *result, gpointer user_data)
	{
		GError *error = nullptr;
		GVariant *var = g_dbus_connection_call_finish((GDBusConnection *)source_object, result, &error);

		MGDbusServer *server = reinterpret_cast<MGDbusServer *>(user_data);
		server->AsyncReady((GDBusConnection *)source_object, var, error);
	}

	static const GDBusInterfaceVTable sInterfaceVTable;

	GDBusNodeInfo *mIntrospectionData;
	uint32_t mOwnerId, mRegistrationID, mWatcherID;
	string mOpenParameter;
};

const GDBusInterfaceVTable MGDbusServer::sInterfaceVTable = {
	MGDbusServer::HandleMethodCall
};

// ----------------------------------------------------------------------------
//	Main routines

void my_signal_handler(int inSignal)
{
	switch (inSignal)
	{
		case SIGPIPE:
			break;

		case SIGUSR1:
			break;

		case SIGINT:
			//			gQuit = true;
			gApp->DoQuit();
			break;

		case SIGTERM:
			//			gQuit = true;
			gApp->DoQuit();
			break;
	}
}

int main(int argc, char *argv[])
{
	setenv("UBUNTU_MENUPROXY", "0", true);

	try
	{
		auto &config = mcfp::config::instance();

		config.init("usage: salt [options] [url-to-open]",
			mcfp::make_option("help,h", "Display this message"),
			mcfp::make_option("version", "Show version number"),
			mcfp::make_option("connect,c", "Show connect dialog"),
			mcfp::make_option("install,i", "Install the application"),
			mcfp::make_option<std::string>("prefix,p", "/usr/local", "Installation prefix, default is /usr/local"));

		config.parse(argc, argv);

		if (config.has("help"))
		{
			cerr << config << endl;
			exit(0);
		}

		if (config.has("version"))
		{
			write_version_string(cout, false);
			exit(0);
		}

		// First find out who we are. Uses proc filesystem to find out.
		char exePath[PATH_MAX + 1];

		int r = readlink("/proc/self/exe", exePath, PATH_MAX);
		if (r > 0)
		{
			exePath[r] = 0;
			gExecutablePath = fs::canonical(exePath);
			gPrefixPath = gExecutablePath.parent_path();
		}

		if (not fs::exists(gExecutablePath))
			gExecutablePath = fs::canonical(argv[0]);

		if (config.has("install"))
		{
			string prefix = config.get<std::string>("prefix");
			MApplication::Install(prefix);
			exit(0);
		}

		// gdk_threads_init();
		gtk_init(&argc, &argv);

		string url;
		if (config.operands().size() > 0)
			url = config.operands().front();

		MGDbusServer dBusServer(url);

		unique_ptr<MApplication> app(MApplication::Create(
			new MGtkApplicationImpl));

		app->Initialise();

		struct sigaction act, oact;
		act.sa_handler = my_signal_handler;
		sigemptyset(&act.sa_mask);
		act.sa_flags = 0;
		::sigaction(SIGTERM, &act, &oact);
		::sigaction(SIGUSR1, &act, &oact);
		::sigaction(SIGPIPE, &act, &oact);
		::sigaction(SIGINT, &act, &oact);

		app->RunEventLoop();
	}
	catch (exception &e)
	{
		cerr << e.what() << endl;
	}
	catch (...)
	{
		cerr << "Exception caught" << endl;
	}

	return 0;
}

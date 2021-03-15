//          Copyright Maarten L. Hekkelman 2006-2014
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MGtkLib.hpp"

#include <cassert>
#include <filesystem>

#include "MApplication.hpp"
#include "MError.hpp"
#include "MFile.hpp"
#include "MStrings.hpp"

using namespace std;
namespace fs = std::filesystem;

// ------------------------------------------------------------------
//
//  Three different implementations of extended attributes...
//

// ------------------------------------------------------------------
//  FreeBSD

#if defined(__FreeBSD__) and (__FreeBSD__ > 0)

#include <sys/extattr.h>

int32_t read_attribute(const fs::path &inPath, const char *inName, void *outData, size_t inDataSize)
{
	string path = inPath.string();

	return extattr_get_file(path.c_str(), EXTATTR_NAMESPACE_USER,
	                        inName, outData, inDataSize);
}

int32_t write_attribute(const fs::path &inPath, const char *inName, const void *inData, size_t inDataSize)
{
	string path = inPath.string();

	time_t t = last_write_time(inPath);

	int r = extattr_set_file(path.c_str(), EXTATTR_NAMESPACE_USER,
	                         inName, inData, inDataSize);

	last_write_time(inPath, t);
}

#endif

// ------------------------------------------------------------------
//  Linux

#if defined(__linux__)

// #include <attr/attributes.h>

int32_t read_attribute(const fs::path &inPath, const char *inName, void *outData, size_t inDataSize)
{
	string path = inPath.string();

	int length = inDataSize;
	// int err = ::attr_get(path.c_str(), inName,
	// 	reinterpret_cast<char*>(outData), &length, 0);

	// if (err != 0)
	// 	length = 0;

	return length;
}

int32_t write_attribute(const fs::path &inPath, const char *inName, const void *inData, size_t inDataSize)
{
	string path = inPath.string();

	// (void)::attr_set(path.c_str(), inName,
	// 	reinterpret_cast<const char*>(inData), inDataSize, 0);

	return inDataSize;
}

#endif

// ------------------------------------------------------------------
//  MacOS X

#if defined(__APPLE__)

#include <sys/xattr.h>

int32_t read_attribute(const fs::path &inPath, const char *inName, void *outData, size_t inDataSize)
{
	string path = inPath.string();

	return ::getxattr(path.c_str(), inName, outData, inDataSize, 0, 0);
}

int32_t write_attribute(const fs::path &inPath, const char *inName, const void *inData, size_t inDataSize)
{
	string path = inPath.string();

	(void)::setxattr(path.c_str(), inName, inData, inDataSize, 0, 0);
}

#endif

time_t GetFileCreationTime(const fs::path &inFile)
{
	assert(false);
	return 0;
}

bool FileIsReadOnly(const fs::path &inFile)
{
	bool result = false;

	assert(false);

	//	//WIN32_FILE_ATTRIBUTE_DATA data = {};
	//	//
	//	//wstring path = c2w(inFile.native_file_string());
	//	//if (::GetFileAttributesExW(path.c_str(), GetFileExInfoStandard,
	//	//	 &data))
	//	//{
	//	//	result = (data.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0;
	//	//}
	//
	//	wchar_t username[1024];
	//	DWORD n = sizeof(username) / sizeof(wchar_t);
	//	if (::GetUserNameW(username, &n))
	//	{
	//		username[n] = 0;
	//
	//		MAuthzContext context(username);
	//		result = not context.IsWritable(inFile);
	//	}

	return result;
}

namespace MFileDialogs

{

bool ChooseDirectory(MWindow *inParent, fs::path &outDirectory)
{
	GtkWidget *dialog = nullptr;
	bool result = false;

	try
	{
		dialog =
			gtk_file_chooser_dialog_new(_("Select Folder"), nullptr,
		                                GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
		                                "_Cancel", GTK_RESPONSE_CANCEL,
		                                "_Open", GTK_RESPONSE_ACCEPT,
		                                NULL);

		THROW_IF_NIL(dialog);

		string currentFolder = "."; //gApp->GetCurrentFolder();

		if (currentFolder.length() > 0)
		{
			gtk_file_chooser_set_current_folder_uri(
				GTK_FILE_CHOOSER(dialog), currentFolder.c_str());
		}

		if (fs::exists(outDirectory) and outDirectory != fs::path())
		{
			gtk_file_chooser_select_filename(GTK_FILE_CHOOSER(dialog),
			                                 outDirectory.string().c_str());
		}

		if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		{
			char *uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog));
			if (uri != nullptr)
			{
				MFile url(uri, true);
				outDirectory = url.GetPath();

				g_free(uri);

				result = true;
			}
			//
			//			gApp->SetCurrentFolder(
			//				gtk_file_chooser_get_current_folder_uri(GTK_FILE_CHOOSER(dialog)));
		}
	}
	catch (exception &e)
	{
		if (dialog)
			gtk_widget_destroy(dialog);

		throw;
	}

	gtk_widget_destroy(dialog);

	return result;
}

//bool ChooseDirectory(
//	fs::path&			outDirectory)
//{
//	bool result = true;
//
//	MFile dir(outDirectory);
//
//	if (ChooseDirectory(dir))
//	{
//		outDirectory = dir.GetPath();
//		result = true;
//	}
//
//	return result;
//}

bool ChooseOneFile(MWindow *inParent, fs::path &ioFile)
{
	GtkWidget *dialog = nullptr;
	bool result = false;

	try
	{
		dialog =
			gtk_file_chooser_dialog_new(_("Select File"), nullptr,
		                                GTK_FILE_CHOOSER_ACTION_OPEN,
		                                "_Cancel", GTK_RESPONSE_CANCEL,
		                                "_Open", GTK_RESPONSE_ACCEPT,
		                                NULL);

		THROW_IF_NIL(dialog);

		gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), false);
		gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), false);

		if (fs::exists(ioFile))
		{
			gtk_file_chooser_select_filename(GTK_FILE_CHOOSER(dialog),
			                                 ioFile.string().c_str());
		}
		//		else if (gApp->GetCurrentFolder().length() > 0)
		//		{
		//			gtk_file_chooser_set_current_folder_uri(
		//				GTK_FILE_CHOOSER(dialog), gApp->GetCurrentFolder().c_str());
		//		}

		if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		{
			char *uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog));
			if (uri != nullptr)
			{
				ioFile = MFile(uri, true).GetPath();
				g_free(uri);

				//				gApp->SetCurrentFolder(
				//					gtk_file_chooser_get_current_folder_uri(GTK_FILE_CHOOSER(dialog)));

				result = true;
			}
		}
	}
	catch (exception &e)
	{
		if (dialog)
			gtk_widget_destroy(dialog);

		throw;
	}

	gtk_widget_destroy(dialog);

	return result;
}

bool ChooseFiles(MWindow *inParent, bool inLocalOnly, std::vector<fs::path> &outFiles)
{
	GtkWidget *dialog = nullptr;

	try
	{
		dialog =
			gtk_file_chooser_dialog_new(_("Open"), nullptr,
		                                GTK_FILE_CHOOSER_ACTION_OPEN,
		                                "_Cancel", GTK_RESPONSE_CANCEL,
		                                "_Open", GTK_RESPONSE_ACCEPT,
		                                NULL);

		THROW_IF_NIL(dialog);

		gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), true);
		gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), inLocalOnly);

		//		if (gApp->GetCurrentFolder().length() > 0)
		//		{
		//			gtk_file_chooser_set_current_folder_uri(
		//				GTK_FILE_CHOOSER(dialog), gApp->GetCurrentFolder().c_str());
		//		}

		if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		{
			GSList *uris = gtk_file_chooser_get_uris(GTK_FILE_CHOOSER(dialog));

			GSList *file = uris;

			while (file != nullptr)
			{
				MFile url(reinterpret_cast<char *>(file->data), true);

				g_free(file->data);
				file->data = nullptr;

				outFiles.push_back(url.GetPath());

				file = file->next;
			}

			g_slist_free(uris);
		}

		char *cwd = gtk_file_chooser_get_current_folder_uri(GTK_FILE_CHOOSER(dialog));
		if (cwd != nullptr)
		{
			//			gApp->SetCurrentFolder(cwd);
			g_free(cwd);
		}
	}
	catch (exception &e)
	{
		if (dialog)
			gtk_widget_destroy(dialog);

		throw;
	}

	gtk_widget_destroy(dialog);

	return outFiles.size() > 0;
}

//
//
//bool ChooseDirectory(
//	fs::path&	outDirectory)
//{
//	GtkWidget* dialog = nullptr;
//	bool result = false;
//
//	try
//	{
//		dialog =
//			gtk_file_chooser_dialog_new(_("Select Folder"), nullptr,
//				GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
//				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
//				GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
//				NULL);
//
//		THROW_IF_NIL(dialog);
//
//		string currentFolder = gApp->GetCurrentFolder();
//
//		if (currentFolder.length() > 0)
//		{
//			gtk_file_chooser_set_current_folder_uri(
//				GTK_FILE_CHOOSER(dialog), currentFolder.c_str());
//		}
//
//		if (fs::exists(outDirectory) and outDirectory != fs::path())
//		{
//			gtk_file_chooser_select_filename(GTK_FILE_CHOOSER(dialog),
//				outDirectory.string().c_str());
//		}
//
//		if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
//		{
//			char* uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog));
//			if (uri != nullptr)
//			{
//				MFile url(uri, true);
//				outDirectory = url.GetPath();
//
//				g_free(uri);
//
//				result = true;
//			}
////
////			gApp->SetCurrentFolder(
////				gtk_file_chooser_get_current_folder_uri(GTK_FILE_CHOOSER(dialog)));
//		}
//	}
//	catch (exception& e)
//	{
//		if (dialog)
//			gtk_widget_destroy(dialog);
//
//		throw;
//	}
//
//	gtk_widget_destroy(dialog);
//
//	return result;
//}
//
////bool ChooseDirectory(
////	fs::path&			outDirectory)
////{
////	bool result = true;
////
////	MFile dir(outDirectory);
////
////	if (ChooseDirectory(dir))
////	{
////		outDirectory = dir.GetPath();
////		result = true;
////	}
////
////	return result;
////}
//
//bool ChooseOneFile(
//	MFile&	ioFile)
//{
//	GtkWidget* dialog = nullptr;
//	bool result = false;
//
//	try
//	{
//		dialog =
//			gtk_file_chooser_dialog_new(_("Select File"), nullptr,
//				GTK_FILE_CHOOSER_ACTION_OPEN,
//				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
//				GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
//				NULL);
//
//		THROW_IF_NIL(dialog);
//
//		gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), false);
//		gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), false);
//
//		if (ioFile.IsValid())
//		{
//			gtk_file_chooser_select_uri(GTK_FILE_CHOOSER(dialog),
//				ioFile.GetURI().c_str());
//		}
//		else if (gApp->GetCurrentFolder().length() > 0)
//		{
//			gtk_file_chooser_set_current_folder_uri(
//				GTK_FILE_CHOOSER(dialog), gApp->GetCurrentFolder().c_str());
//		}
//
//		if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
//		{
//			char* uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog));
//			if (uri != nullptr)
//			{
//				ioFile = MFile(uri, true);
//				g_free(uri);
//
//				gApp->SetCurrentFolder(
//					gtk_file_chooser_get_current_folder_uri(GTK_FILE_CHOOSER(dialog)));
//
//				result = true;
//			}
//		}
//	}
//	catch (exception& e)
//	{
//		if (dialog)
//			gtk_widget_destroy(dialog);
//
//		throw;
//	}
//
//	gtk_widget_destroy(dialog);
//
//	return result;
//}
//
//bool ChooseFiles(
//	bool				inLocalOnly,
//	std::vector<MFile>&	outFiles)
//{
//	GtkWidget* dialog = nullptr;
//
//	try
//	{
//		dialog =
//			gtk_file_chooser_dialog_new(_("Open"), nullptr,
//				GTK_FILE_CHOOSER_ACTION_OPEN,
//				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
//				GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
//				NULL);
//
//		THROW_IF_NIL(dialog);
//
//		gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), true);
//		gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), inLocalOnly);
//
//		if (gApp->GetCurrentFolder().length() > 0)
//		{
//			gtk_file_chooser_set_current_folder_uri(
//				GTK_FILE_CHOOSER(dialog), gApp->GetCurrentFolder().c_str());
//		}
//
//		if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
//		{
//			GSList* uris = gtk_file_chooser_get_uris(GTK_FILE_CHOOSER(dialog));
//
//			GSList* file = uris;
//
//			while (file != nullptr)
//			{
//				MFile url(reinterpret_cast<char*>(file->data), true);
//
//				g_free(file->data);
//				file->data = nullptr;
//
//				outFiles.push_back(url);
//
//				file = file->next;
//			}
//
//			g_slist_free(uris);
//		}
//
//		char* cwd = gtk_file_chooser_get_current_folder_uri(GTK_FILE_CHOOSER(dialog));
//		if (cwd != nullptr)
//		{
//			gApp->SetCurrentFolder(cwd);
//			g_free(cwd);
//		}
//	}
//	catch (exception& e)
//	{
//		if (dialog)
//			gtk_widget_destroy(dialog);
//
//		throw;
//	}
//
//	gtk_widget_destroy(dialog);
//
//	return outFiles.size() > 0;
//}

bool SaveFileAs(MWindow *inParent, fs::path &ioFile)
{
	assert(false);
	return false;
}

} // namespace MFileDialogs

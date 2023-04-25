//          Copyright Maarten L. Hekkelman 2006-2014
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "Gtk/MGtkLib.hpp"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <iostream>

#include "MAlerts.hpp"
#include "MError.hpp"
#include "MSound.hpp"
#include "MStrings.hpp"
#include "MTypes.hpp"
#include "MUtils.hpp"

using namespace std;

#ifndef NDEBUG

const char *__S_FILE;
int __S_LINE;

#ifndef _MSC_VER
void __debug_printf(const char *inStr, ...)
{
	char msg[1024];

	va_list vl;
	va_start(vl, inStr);
	vsnprintf(msg, sizeof(msg), inStr, vl);
	va_end(vl);

	cerr << msg << endl;
}

void __signal_throw(
	const char *inCode,
	const char *inFunction,
	const char *inFile,
	int inLine)
{
	cerr << "Throwing in file " << inFile << " line " << inLine
		 << " \"" << inFunction << "\": " << endl
		 << inCode << endl;

	if (StOKToThrow::IsOK())
		return;

	GtkWidget *dlg = gtk_message_dialog_new(nullptr, GTK_DIALOG_MODAL,
	                                        GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
	                                        "Exception thrown in file '%s', line %d, function: '%s'\n\n"
	                                        "code: %s",
	                                        inFile, inLine, inFunction, inCode);

	PlaySound("error");
	(void)gtk_dialog_run(GTK_DIALOG(dlg));

	gtk_widget_destroy(dlg);
}
#endif

#endif

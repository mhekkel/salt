salt
====

Synopsis
--------

Simply start salt and open a terminal

**salt** [*options*]

Start a session on a remote host

**salt** --connect *host*

Start a command in a new terminal

**salt** -- *program* [args...]

Description
-----------

:program:`salt` is a terminal emulator with built-in SSH.

Options
-------

.. option:: --help

	Display the options allowed for this program.

.. option:: --version

	Display the version of this program.

.. option:: --verbose

	Use a more verbose output, especially for the *--version* option.


.. option:: -c, --connect host

	Connect to the remote *host*

.. option:: --select-host

	Show connection dialog

.. option:: -i, --install

	Install the application

.. option:: -p, --prefix path

	Specify the installation prefix

Description
-----------

Salt is a terminal emulator with built-in SSH capabilities.
The emulation should be comparable to xterm offering the usual
VTxx emulation as well as more modern features like colors.

The SSH implementation uses multiplexing for connections to hosts
which allows to quickly open multiple sessions reusing the open
connection. There's also support for transferring files as well
as three ways of acting as a proxy.

Specific features
-----------------

Unlike other terminal emulators, Salt handles a right click the
same as a middle button click allowing you to simply paste
from the primary buffer when using a laptop with only a simple
trackpad.

To switch between windows you can use Control-Tab and Control-Shift-Tab
sequences. However, if you are using Wayland and have an older
implemenation of a compositor than this will not work as expected
since the required protocol (*xdg_activation_v1*) is not yet
supported by many. The workaround it so start salt using an X11
backend. (launch with GDK_BACKEND=x11 in the environment)

File Transfer
-------------

Files can be transferred using drag and drop. Drag a file into
a terminal connected to a remote host and the file will be uploaded.
Use *--hyperlink* switch for ls in the terminal and the files will show
an underline when hovered. Press the control key and click a
link and the file will be downloaded.

Upload and download locations can be specified in the preferences
dialog. Salt can find out where to upload files by using a trick
similar to the one used to set a window title. To do so, you can source
the following script in your .bashrc or .zschrc file.

.. code:: bash

	# A script that produces a prompt containing useful information

	# Only works with zsh and bash
	[ -n $BASH_VERSION -o -n $ZSH_VERSION ] || return 0

	# Only run in interactive shell, test if 'i' is in $- (the startup options)
	[[ $- == *i* ]] || return 0

	# Enocde URL
	encodeurl() {
		LC_ALL=C
		local str="$1"
		local enc="" safe o
		
		while [ -n "$str" ]; do
			safe="${str%%[!a-zA-Z0-9/:_\.\-\!\'\(\)~]*}"
			enc+=$safe
			str="${str#"$safe"}"
			if [ -n "$str" ]; then
				printf -v o "%%%02X" "'$str"
				enc+=$o
				str="${str#?}"
			fi
		done
		
		echo "${enc}"
	}

	# Generic OSC 7 writing command 
	osc7_cmd () {
		# Add command for salt to know the current directory
		# HOSTNAME is not set in zsh, by default, but HOST is
		printf "\033]7;file://%s%s\x9c" "${HOSTNAME:-${HOST}}" "$(encodeurl "${PWD}")"
	}

	# The prompt command function for use in bash
	prompt_cmd () {
		local EXIT="$?"
		local user=""
		if [ $EXIT != 0 ]; then
			user+='\[\e[37;1;41m\]\u\[\e[0m\]'
		else
			user+='\u'
		fi

		# Standard prompt with red notification of non-zero exit status
		PS1="${user}@\[\e[4m\]\h\[\e[24m\]:\[\e[1m\]\w\[\e[0m\]\$ "

		# Set the title to user@host:dir
		PS1="\[\e]0;\u@\h: \w\a\]$PS1"

		# And add the OSC 7 command to notify the current host and dir
		cwd=$(osc7_cmd)
		PS1="\[$(osc7_cmd)\]$PS1"
	}

	# And add this command to the prompt, depending on the shell
	case "$TERM" in
	xterm*|vte*|salt*)
		[ -n "$BASH_VERSION" ] && PROMPT_COMMAND="prompt_cmd"
		[ -n "$ZSH_VERSION"  ] && precmd_functions+=(osc7_cmd)
		;;
	esac

	true

An alternative way of up- and downloading files is by using some
bash scripts. Add these two functions to your *.bashrc* file and
you will be able to *get* and *put* files directly from your
terminal session. The *put* command will open a file chooser 
dialog at your local computer to select the file to upload. The
get will show a directory selection dialog unless there is a
default location for downloads specified in the preferences dialog.

.. code:: bash

	get() {
	  file="$1";
	  printf "\033_7;%s\x9c" $(realpath -qez "$file" | base64 -w0)
	}

	put() {
	  file="$1";
	  printf "\033_8;%s\x9c" $(realpath -qz "$file" | base64 -w0)
	}

The bash scripts use an escape code sequence called **APC** which
stands for *Application Program-Control functions*. These are escape
sequences that start with an *ESC* followed by an underscore *_*.
The first numeric argument is used by Salt. The rest is a string
containing a path which is base64 encoded.

The commands recognized by Salt are:

+-------------------+---------------------------------------------+
| command number    | Action                                      |
+===================+=============================================+
| 7                 | Download the specified file                 |
+-------------------+---------------------------------------------+
| 8                 | Upload a file to the specified location     |
+-------------------+---------------------------------------------+

Bugs
----

There undoubtely will be bugs in this application. Please report
at https://github.com/mhekkel/salt/issues


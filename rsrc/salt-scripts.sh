# A script that produces a prompt containing useful information

# Only works with zsh and bash
[ -n "$BASH_VERSION" -o -n "$ZSH_VERSION" ] || return 0

# Only run in interactive shell, test if 'i' is in $- (the startup options)
[[ $- == *i* ]] || return 0

# These two commands can be used in a terminal to up- and download files

# Use get to download a file from the server to your local computer
get() {
	file="$1";
	printf "\033_7;%s\x9c" $(realpath -qez "$file" | base64 -w0)
}

# Use get to upload a file from your local computer to the server.
# Note that this may overwrite an existing file.
put() {
	file="$1";
	printf "\033_8;%s\x9c" $(realpath -qz "$file" | base64 -w0)
}

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

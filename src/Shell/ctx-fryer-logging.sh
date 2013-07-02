# Log levels
LOG_LEVEL_FATAL=0
LOG_LEVEL_ERROR=1
LOG_LEVEL_WARN=2
LOG_LEVEL_INFO=3
LOG_LEVEL_DEBUG=4

# Colors usage autosetting
if test -z "$LOG_USE_COLOURS"; then
    if test -t 2; then
        LOG_USE_COLOURS=`tput colours 2>/dev/null || echo 0`
    else
        LOG_USE_COLOURS=0
    fi
fi


# Default log level
test -n "$LOG_LEVEL" || LOG_LEVEL=$LOG_LEVEL_ERROR


# Log function (private)
_log() {
    test "$1" -le "$LOG_LEVEL" || return 0

    colour=""
    emoticon=""

    case $1 in
        $LOG_LEVEL_FATAL)
            colour=31  # red
            emoticon="X-("
            ;;
        $LOG_LEVEL_ERROR)
            colour=31  # red
            emoticon=":-("
            ;;
        $LOG_LEVEL_WARN)
            colour=33  # yellow
            emoticon=":-o"
            ;;
        $LOG_LEVEL_INFO)
            colour=32  # green
            emoticon=":-)"
            ;;
        $LOG_LEVEL_DEBUG)
            colour=30  # dark grey
            emoticon=":-D"
            ;;
        *)
            colour=37  # white
            emoticon="8-?"
            ;;
    esac

    shift

    if test -n "$LOG_USE_COLOURS"; then
        emoticon="\033[01;${colour}m${emoticon}\033[00m"
    fi

    echo "${emoticon}  $*" >&2
}


# Log functions (level-specific shortcuts)
FATAL() { _log $LOG_LEVEL_FATAL $*; exit 127; }
ERROR() { _log $LOG_LEVEL_ERROR $*; }
WARN()  { _log $LOG_LEVEL_WARN  $*; }
INFO()  { _log $LOG_LEVEL_INFO  $*; }
DEBUG() { _log $LOG_LEVEL_DEBUG $*; }

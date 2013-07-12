# Log levels
LOG_LEVEL_FATAL=0
LOG_LEVEL_ERROR=1
LOG_LEVEL_WARN=2
LOG_LEVEL_INFO=3
LOG_LEVEL_DEBUG=4

# Date logging autosetting
if test -z "$LOG_USE_DATE"; then
    if test -t 2; then
        LOG_USE_DATE=0
    else
        LOG_USE_DATE=1
    fi
fi

# Process logging autosetting
if test -z "$LOG_USE_PROCESS"; then
    if test -t 2; then
        LOG_USE_PROCESS=0
    else
        LOG_USE_PROCESS=1
    fi
fi

# Colors usage autosetting
if test -z "$LOG_USE_COLOURS"; then
    if test -t 2; then
        LOG_USE_COLOURS=`tput colors 2>/dev/null || echo 0`
    else
        LOG_USE_COLOURS=0
    fi
fi


# Default log level
test -n "$LOG_LEVEL" || LOG_LEVEL=$LOG_LEVEL_INFO


# Log function (private)
_log() {
    test "$1" -le "$LOG_LEVEL" || return 0

    colour=""
    date=""
    process=""
    level=""

    case $1 in
        $LOG_LEVEL_FATAL)
            colour=31  # red
            level="FATAL"
            ;;
        $LOG_LEVEL_ERROR)
            colour=31  # red
            level="ERROR"
            ;;
        $LOG_LEVEL_WARN)
            colour=33  # yellow
            level="WARN"
            ;;
        $LOG_LEVEL_INFO)
            colour=32  # green
            level="INFO"
            ;;
        $LOG_LEVEL_DEBUG)
            colour=30  # dark grey
            level="DEBUG"
            ;;
        *)
            colour=37  # white
            level="????"
            ;;
    esac

    shift

    if test "$LOG_USE_DATE" -gt 0; then
        date=`date "+%Y/%m/%d %H:%M:%S "`
    fi

    if test "$LOG_USE_PROCESS" -gt 0; then
        process=`basename $0`
        process="$process ($$) "
    fi

    if test "$LOG_USE_COLOURS" -gt 0; then
        level="\033[01;${colour}m${level}\033[00m"
    fi

    echo "${date}${process}[${level}] $*" >&2
}


# Log functions (level-specific shortcuts)
FATAL() { _log $LOG_LEVEL_FATAL $*; exit 127; }
ERROR() { _log $LOG_LEVEL_ERROR $*; }
WARN()  { _log $LOG_LEVEL_WARN  $*; }
INFO()  { _log $LOG_LEVEL_INFO  $*; }
DEBUG() { _log $LOG_LEVEL_DEBUG $*; }

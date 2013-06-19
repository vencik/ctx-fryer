# Log levels
LOG_LEVEL_FATAL=0
LOG_LEVEL_ERROR=1
LOG_LEVEL_WARN=2
LOG_LEVEL_INFO=3
LOG_LEVEL_DEBUG=4


# Log level stringifisation (private)
_logLevel2str() {
    case $1 in
        $LOG_LEVEL_FATAL)  echo "X-(";;
        $LOG_LEVEL_ERROR)  echo ":-(";;
        $LOG_LEVEL_WARN)   echo ":-o";;
        $LOG_LEVEL_INFO)   echo ":-)";;
        $LOG_LEVEL_DEBUG)  echo ":-D";;
        *)                 echo "8-?";;
    esac
}


# Log function (private)
_log() {
    local log_level="$log_level"
    local msg_level=$1

    shift

    test -n "$log_level" || log_level=$LOG_LEVEL_ERROR
    test "$msg_level" -le "$log_level" || return 0

    echo "`_logLevel2str $msg_level`  $*" >&2
}


# Log functions (level-specific shortcuts)
FATAL() { _log $LOG_LEVEL_FATAL $*; exit 127; }
ERROR() { _log $LOG_LEVEL_ERROR $*; }
WARN()  { _log $LOG_LEVEL_WARN  $*; }
INFO()  { _log $LOG_LEVEL_INFO  $*; }
DEBUG() { _log $LOG_LEVEL_DEBUG $*; }

#!/bin/sh

PID_FILE="$PWD/daemon.$$.pid"
SIG_PIPE="$PWD/daemon.$$.pipe"


# Log error and terminate
error () {
    echo "$*" >&2

    # Clean up
    rm "$SIG_PIPE" 2>/dev/null

    exit 1
}


# Create signal pipe
mknod "$SIG_PIPE" p || error "Failed to create signal pipe $SIG_PIPE"

# Start daemon
./daemon "$PID_FILE" "$SIG_PIPE" || error "Failed to start daemon"
sleep 0.1  # wait a little bit to make sure the daemon has started

# Check that daemon has created PID file
test -f "$PID_FILE" || error "Daemon hasn't created PID file $PID_FILE"

# Check if daemon runs as PID specified
PID=`cat "$PID_FILE"`
EXE=`readlink /proc/$PID/exe`

test "$EXE" = "$PWD/.libs/lt-daemon" || error "Process PID $PID is $EXE"

# Shut daemon down
echo "" > "$SIG_PIPE"
sleep 0.1  # wait a little bit to make sure the daemon has stopped

# Check that daemon has stopped
ps $PID 2>&1 >/dev/null && error "Process $PID is still running"

# Check that PID file was removed
test -f "$PID_FILE" && error "Pid file $PID_FILE still exists"

# Remove signal pipe
rm "$SIG_PIPE" || error "Failed to remove signal pipe $SIG_PIPE"

# OK
exit 0

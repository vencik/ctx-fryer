# Check that the script is executed with absolute path
if expr substr "$0" 1 1 != '/' >/dev/null; then
    echo "The script must be executed with absolute path" >&2

    exit 127
fi

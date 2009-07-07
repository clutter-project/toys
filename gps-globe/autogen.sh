#! /bin/sh

PROJECT=gps-globe

AUTORECONF=`which autoreconf`

if test -z "$AUTORECONF"; then
        echo "*** No autoreconf found ***"
        exit 1
else
        autoreconf -v --install || exit $?
fi

./configure "$@" && echo "Now type 'make' to compile $PROJECT."

#! /bin/sh

if [ "`which hphpize 2>/dev/null`" != "" ]; then
    # HHVM 3.2.0 or newer
    hphpize
else
    # HHVM older than 3.2.0
    if [ "$HPHP_HOME" == "" ]; then
        echo HPHP_HOME environment variable must be set!
        exit 1
    fi

    $HPHP_HOME/hphp/tools/hphpize/hphpize
fi

cmake . && make


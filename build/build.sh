#!/bin/bash

test -r ./config && source ./config || PLATFORM="ARDUINO"

case x${PLATFORM} in
  xARDUINO)
	source ./config.${PLATFORM}
	pushd ${PLATFORM_DIR}
	echo "Building sketch..."
	# TODO...
	echo "Uploading sketch..."
	make upload
	popd
	;;
  *) 	source ./config.${PLATFORM}
	;;
esac

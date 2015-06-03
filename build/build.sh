#!/bin/bash

test -r ./config && source ./config || PLATFORM="ARDUINO"

case x${PLATFORM} in
  xARDUINO)
	source ./config.${PLATFORM}
	pushd ${PLATFORM_DIR}
	echo "Compilation de core.ino..."
	make && echo "Projet compilé dans ${HOME}/BaliseBuild" \
		|| { echo "La compilation a échoué"; exit 1; }
	ls -1 /dev/ttyUSB* 2>&1 > /dev/null && { \
		echo "Téléversement de l'image..." ; \
		make upload ; \
	}
	popd
	;;
  *) 	source ./config.${PLATFORM}
	;;
esac

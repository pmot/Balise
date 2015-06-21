#!/bin/bash

test -r ./config && source ./config || PLATFORM="ARDUINO"

for flag in DEBUG WIFI_SCAN GPS ACCELEROMETER CELLULAR_SMS CELLULAR_DATA
do
  if [ -n ${flag} ]
  then
    FLAGS=" -D${flag} ${C_FLAGS}" 
  fi
done
echo "${C_FLAGS}"

case x${PLATFORM} in
  xARDUINO)
	source ./config.${PLATFORM}
	pushd ${PLATFORM_DIR}
	echo "Compilation de core.ino..."
	C_FLAG=${FLAGS} make && echo "Projet compilé dans ${HOME}/BaliseBuild" \
		|| { echo "La compilation a échoué"; exit 1; }

	ls -1 /dev/ttyUSB* 2>&1 > /dev/null && { \
		echo "Téléversement de l'image..." ; \
		C_FLAG=${FLAGS} make upload ; \
	} || echo "Aucun périphérique Arduino connecté"
	popd
	;;
  *) 	source ./config.${PLATFORM}
	;;
esac

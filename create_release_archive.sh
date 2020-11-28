rm -rf telemetryjet-arduino-sdk
mkdir telemetryjet-arduino-sdk

rsync -avz examples telemetryjet-arduino-sdk/
rsync -avz extras telemetryjet-arduino-sdk/
rsync -avz src telemetryjet-arduino-sdk/
cp keywords.txt telemetryjet-arduino-sdk/
cp library.properties telemetryjet-arduino-sdk/
cp LICENSE.md telemetryjet-arduino-sdk/
cp README.md telemetryjet-arduino-sdk/

zip -r telemetryjet-arduino-sdk.zip telemetryjet-arduino-sdk/ -x "*.DS_Store"
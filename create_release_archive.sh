rm -rf telemetryjet-arduino-sdk
mkdir telemetryjet-arduino-sdk

cp -avr examples/ telemetryjet-arduino-sdk/
cp -avr extras/ telemetryjet-arduino-sdk/
cp -avr src/ telemetryjet-arduino-sdk/
cp -avr keywords.txt telemetryjet-arduino-sdk/
cp -avr library.properties telemetryjet-arduino-sdk/
cp -avr LICENSE.md telemetryjet-arduino-sdk/
cp -avr README.md telemetryjet-arduino-sdk/

zip -r telemetryjet-arduino-sdk.zip telemetryjet-arduino-sdk/ -x "*.DS_Store"
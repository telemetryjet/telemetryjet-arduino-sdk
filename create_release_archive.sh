# Creates a release archive from the arduino library.
echo "Creating release archive..."

# Exit on error
set -e

# Zip library folder
zip -r TelemetryJetArduinoSDK.zip TelemetryJetArduinoSDK/ -x "*.DS_Store"

echo "Done."
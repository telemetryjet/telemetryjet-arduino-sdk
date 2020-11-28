#!groovy

pipeline {
    agent none
    stages {
        stage('Create Release Archive') {
            when {
                buildingTag()
            }
            steps {
                sh 'echo ==== Create Release Archive ===='
                sh './create_release_archive.sh'
            }
        }
        stage('Copy to Download Server') {
            when {
                buildingTag()
            }
            steps {
                sh "echo \"${TAG_NAME}\""
                sh "mv \"telemetryjet-arduino-sdk.zip\" \"telemetryjet-arduino-sdk-${TAG_NAME}.zip\""
                sh "yes | cp -rf \"telemetryjet-arduino-sdk-${TAG_NAME}.zip\" /var/telemetryjet-downloads/builds/arduino-sdk/"
            }
        }
    }
}
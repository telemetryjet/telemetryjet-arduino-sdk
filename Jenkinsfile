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
                sh 'zip -r TelemetryJetArduinoSDK.zip TelemetryJetArduinoSDK/ -x "*.DS_Store"'
            }
        }
        stage('Copy to Download Server') {
            when {
                buildingTag()
            }
            steps {
                sh "echo \"${TAG_NAME}\""
                sh "mv "
            }
        }
    }
}
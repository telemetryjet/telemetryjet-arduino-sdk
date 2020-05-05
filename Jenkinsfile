
pipeline {
    agent {
        label 'linux'
    }
    stages {
        stage('Create Release Archive') {
            steps {
                sh 'echo ==== Create Release Archive ===='
                sh './create_release_archive.sh'
            }
        }
        stage('Upload Release Archive (Dev)') {
            when {
                branch 'develop'
            }
            steps {
                script {
                    withAWS(region: 'us-east-2', credentials: 'aws-s3-artifact-upload') {
                        s3Upload(file: 'TelemetryJetArduinoSDK.zip', bucket: 'files.telemetryjet.com', path: '/releases/arduino_sdk/TelemetryJetArduinoSDK_latest.zip')
                    }
                }
            }
        }
        stage('Upload Release Archive (Release)') {
            when {
                branch 'release/*'
            }
            steps {
                script {
                    def tagName
                    idx = env.BRANCH_NAME.lastIndexOf('/')
                    if (idx >= 0) {
                        tagName = "release_${env.BRANCH_NAME.substring(idx + 1)}"
                    } else {
                        tagName = "release_${env.BRANCH_NAME}"
                    }
                    tagName += "_build_${env.BUILD_NUMBER}"
                    withAWS(region: 'us-east-2', credentials: 'aws-s3-artifact-upload') {
                        s3Upload(file: 'TelemetryJetArduinoSDK.zip', bucket: 'files.telemetryjet.com', path: "/releases/arduino_sdk/TelemetryJetArduinoSDK_${tagName}.zip")
                    }
                }
            }
        }
        stage('Upload Release Archive (Hotfix)') {
            when {
                branch 'hotfix/*'
            }
            steps {
                script {
                    def tagName
                    idx = env.BRANCH_NAME.lastIndexOf('/')
                    if (idx >= 0) {
                        tagName = "hotfix_${env.BRANCH_NAME.substring(idx + 1)}"
                    } else {
                        tagName = "hotfix_${env.BRANCH_NAME}"
                    }
                    tagName += "_build_${env.BUILD_NUMBER}"
                    withAWS(region: 'us-east-2', credentials: 'aws-s3-artifact-upload') {
                        s3Upload(file: 'TelemetryJetArduinoSDK.zip', bucket: 'files.telemetryjet.com', path: "/releases/arduino_sdk/TelemetryJetArduinoSDK_${tagName}.zip")
                    }
                }
            }
        }
    }
}
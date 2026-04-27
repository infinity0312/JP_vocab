import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Window {
    id: mode2Window
    width: 640
    height: 720
    title: qsTr("练拼写")
    color: "#f5f5f5"

    property string feedbackMsg: ""
    property string feedbackColor: "#333"
    property string lastCorrectWord: ""

    Rectangle {
        id: topBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 48
        color: "#2196F3"
        z: 2

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12

            Button {
                text: qsTr("返回")
                flat: true
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    if (wordsModel.wrongWordCount > 0) {
                        exportDialog.open()
                    } else {
                        wordsModel.clearSession()
                        mode2Window.hide()
                    }
                }
            }

            Item { Layout.fillWidth: true }

            Label {
                text: qsTr("%1 / %2").arg(wordsModel.completedCount).arg(wordsModel.completedCount + wordsModel.remainingCount)
                font.pixelSize: 14
                color: "white"
            }
        }
    }

    ProgressBar {
        id: progressBar
        anchors.top: topBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        from: 0
        to: wordsModel.completedCount + wordsModel.remainingCount
        value: wordsModel.completedCount
        height: 4
    }

    StackLayout {
        id: stack
        anchors.top: progressBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        currentIndex: wordsModel.hasCurrentWord ? 0 : (wordsModel.completedCount > 0 ? 2 : 1)

        ColumnLayout {
            id: reviewPage
            spacing: 16

            Item { Layout.fillHeight: true }

            Label {
                text: wordsModel.currentMeaning
                font.pixelSize: 32
                font.bold: true
                color: "#333"
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 60
                color: "white"
                radius: 8
                border.color: answerField.activeFocus ? "#2196F3" : "#ccc"
                border.width: answerField.activeFocus ? 2 : 1

                TextField {
                    id: answerField
                    anchors.fill: parent
                    anchors.margins: 10
                    font.pixelSize: 20
                    verticalAlignment: Text.AlignVCenter
                    placeholderTextColor: "#bbb"
                    onAccepted: {
                        if (wordsModel.answerShown) {
                            continueBtn.clicked()
                        } else {
                            submitBtn.clicked()
                        }
                    }
                }
            }

            Label {
                id: feedbackLabel
                text: feedbackMsg
                font.pixelSize: 16
                color: feedbackColor
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                visible: feedbackMsg !== ""
            }

            Button {
                id: submitBtn
                text: qsTr("确认")
                highlighted: true
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                palette.button: "#2196F3"
                enabled: answerField.text.trim().length > 0 && !wordsModel.answerShown
                onClicked: {
                    var answer = answerField.text.trim()
                    var correct = wordsModel.checkAnswer(answer)
                    if (correct) {
                        feedbackMsg = qsTr("✓ 正确！")
                        feedbackColor = "#4CAF50"
                        lastCorrectWord = ""
                        answerField.text = ""
                        autoAdvanceTimer.start()
                    } else {
                        lastCorrectWord = wordsModel.currentWord
                        feedbackMsg = qsTr("✗ 错误！正确答案: %1").arg(lastCorrectWord)
                        feedbackColor = "#f44336"
                        answerField.text = ""
                    }
                }
            }

            Button {
                id: continueBtn
                text: qsTr("继续")
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                visible: wordsModel.answerShown
                onClicked: {
                    feedbackMsg = ""
                    answerField.text = ""
                    answerField.forceActiveFocus()
                    wordsModel.nextWord()
                }
            }

            Item { Layout.fillHeight: true }


        }

        ColumnLayout {
            id: noWordsPage
            spacing: 16

            Item { Layout.fillHeight: true }

            Label {
                text: qsTr("暂无待复习单词")
                font.pixelSize: 20
                color: "#999"
                Layout.alignment: Qt.AlignHCenter
            }

            Button {
                text: qsTr("返回")
                Layout.alignment: Qt.AlignHCenter
                onClicked: {
                    if (wordsModel.wrongWordCount > 0) {
                        exportDialog.open()
                    } else {
                        wordsModel.clearSession()
                        mode2Window.hide()
                    }
                }
            }

            Item { Layout.fillHeight: true }
        }

        ColumnLayout {
            id: completedPage
            spacing: 16

            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
            }

            Label {
                text: qsTr("🎉 今日复习完成！")
                font.pixelSize: 28
                font.bold: true
                color: "#2196F3"
                Layout.alignment: Qt.AlignCenter
            }

            Label {
                text: qsTr("已完成 %1 个单词").arg(wordsModel.completedCount)
                font.pixelSize: 16
                color: "#666"
                Layout.alignment: Qt.AlignHCenter
            }

            Button {
                text: qsTr("返回主界面")
                highlighted: true
                palette.button: "#2196F3"
                Layout.alignment: Qt.AlignHCenter
                onClicked: {
                    if (wordsModel.wrongWordCount > 0) {
                        exportDialog.open()
                    } else {
                        wordsModel.clearSession()
                        mode2Window.hide()
                    }
                }
            }

            Item { Layout.fillHeight: true }
        }
    }

    Timer {
        id: autoAdvanceTimer
        interval: 800
        repeat: false
        onTriggered: {
            feedbackMsg = ""
            wordsModel.nextWord()
            if (wordsModel.hasCurrentWord)
                answerField.forceActiveFocus()
        }
    }

    Connections {
        target: wordsModel
        function onCurrentWordChanged() {
            feedbackMsg = ""
            answerField.text = ""
            if (wordsModel.hasCurrentWord)
                answerField.forceActiveFocus()
        }
    }

    Dialog {
        id: exportDialog
        title: qsTr("导出错词")
        modal: true
        standardButtons: Dialog.Yes | Dialog.No
        anchors.centerIn: parent

        Label {
            text: qsTr("本次学习中有 %1 个错词，是否导出？").arg(wordsModel.wrongWordCount)
            font.pixelSize: 14
        }

        onAccepted: saveFileDialog.open()
        onRejected: {
            wordsModel.clearSession()
            mode2Window.hide()
        }
    }

    FileDialog {
        id: saveFileDialog
        title: qsTr("保存错词文件")
        acceptLabel: qsTr("保存")
        rejectLabel: qsTr("取消")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "csv"
        nameFilters: ["CSV files (*.csv)"]

        onAccepted: {
            wordsModel.exportWrongWords(selectedFile)
            wordsModel.clearSession()
            mode2Window.hide()
        }
        onRejected: {
            wordsModel.clearSession()
            mode2Window.hide()
        }
    }
}

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Window {
    id: mode1Window
    width: 640
    height: 720
    title: qsTr("背生词")
    color: "#f5f5f5"

    Rectangle {
        id: topBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 48
        color: "#4CAF50"
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
                        mode1Window.hide()
                    }
                }
            }

            Button {
                text: qsTr("撤销")
                flat: true
                enabled: wordsModel.canGoBack
                contentItem: Text {
                    text: parent.text
                    color: wordsModel.canGoBack ? "white" : "rgba(255,255,255,0.4)"
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: wordsModel.goBack()
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
            Layout.fillWidth: true
            Layout.fillHeight: true

            Item { Layout.fillHeight: true }

            Label {
                text: wordsModel.currentWord
                font.pixelSize: 42
                font.bold: true
                color: "#333"
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap
            }

            Label {
                text: wordsModel.currentMeaning
                font.pixelSize: 22
                color: "#666"
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap
                visible: wordsModel.meaningRevealed
            }

            Button {
                text: qsTr("点击显示释义")
                font.pixelSize: 14
                visible: !wordsModel.meaningRevealed
                Layout.alignment: Qt.AlignHCenter
                onClicked: wordsModel.revealMeaning()
            }

            Item { Layout.fillHeight: true }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 80
                Layout.alignment: Qt.AlignHCenter
                color: "white"
                radius: 8

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 16

                    Button {
                        id: unknownBtn
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: qsTr("不认识")
                        font.pixelSize: 18
                        font.bold: true
                        palette.button: "#f44336"
                        palette.buttonText: "white"
                        onClicked: wordsModel.markUnknown()
                    }

                    Button {
                        id: knownBtn
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: qsTr("认识")
                        font.pixelSize: 18
                        font.bold: true
                        palette.button: "#4CAF50"
                        palette.buttonText: "white"
                        onClicked: wordsModel.markKnown()
                    }
                }
            }
        }

        ColumnLayout {
            id: noWordsPage
            spacing: 16
            Layout.fillWidth: true
            Layout.fillHeight: true

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
                        mode1Window.hide()
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
                color: "#4CAF50"
                Layout.alignment: Qt.AlignHCenter
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
                Layout.alignment: Qt.AlignHCenter
                onClicked: {
                    if (wordsModel.wrongWordCount > 0) {
                        exportDialog.open()
                    } else {
                        wordsModel.clearSession()
                        mode1Window.hide()
                    }
                }
            }

            Item { Layout.fillHeight: true }
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
            mode1Window.hide()
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
            mode1Window.hide()
        }
        onRejected: {
            wordsModel.clearSession()
            mode1Window.hide()
        }
    }
}

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Window {
    id: mainWindow
    width: 640
    height: 720
    visible: true
    title: qsTr("背单词JP")
    color: "#f5f5f5"

    FileDialog {
        id: fileDialog
        title: qsTr("选择单词文件")
        nameFilters: ["Text files (*.txt)", "CSV files (*.csv)", "All files (*)"]
        onAccepted: {
            importPopup.fileUrl = selectedFile
            importPopup.open()
        }
    }

    Popup {
        id: importPopup
        anchors.centerIn: parent
        width: 360
        height: 240
        modal: true
        closePolicy: Popup.CloseOnEscape
        padding: 20

        property url fileUrl: ""

        background: Rectangle {
            color: "white"
            radius: 12
            border.color: "#e0e0e0"
            border.width: 1
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 16

            Label {
                text: qsTr("导入单词文件")
                font.pixelSize: 18
                font.bold: true
                Layout.fillWidth: true
            }

            Label {
                text: qsTr("CSV格式: 单词,释义")
                font.pixelSize: 13
                color: "#888"
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Label {
                    text: qsTr("分隔符:")
                    font.pixelSize: 14
                }

                TextField {
                    id: delimiterField
                    Layout.fillWidth: true
                    text: ","
                    maximumLength: 5
                    placeholderText: qsTr("输入分隔符")
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight
                spacing: 8

                Button {
                    text: qsTr("取消")
                    onClicked: importPopup.close()
                }

                Button {
                    text: qsTr("导入")
                    highlighted: true
                    onClicked: {
                        wordsModel.importCSV(importPopup.fileUrl, delimiterField.text)
                        importPopup.close()
                    }
                }
            }
        }
    }

    Popup {
        id: importResultPopup
        anchors.centerIn: parent
        width: 280
        height: 120
        modal: true
        padding: 20

        background: Rectangle {
            color: "white"
            radius: 12
            border.color: "#e0e0e0"
            border.width: 1
        }

        property string message: ""

        ColumnLayout {
            anchors.fill: parent
            spacing: 12

            Label {
                text: importResultPopup.message
                font.pixelSize: 14
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            Button {
                text: qsTr("确定")
                Layout.alignment: Qt.AlignHCenter
                onClicked: importResultPopup.close()
            }
        }
    }

    Connections {
        target: wordsModel
        function onImportCompleted(count) {
            importResultPopup.message = qsTr("成功导入 %1 个单词").arg(count)
            importResultPopup.open()
        }
        function onImportFailed(error) {
            importResultPopup.message = qsTr("导入失败: %1").arg(error)
            importResultPopup.open()
        }
    }

    Rectangle {
        id: header
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 56
        color: "#4CAF50"

        Button {
            id: libraryBtn
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("词库")
            flat: true
            contentItem: Text {
                text: parent.text
                color: "white"
                font.pixelSize: 14
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            onClicked: editorWindow.show()
        }

        Label {
            anchors.centerIn: parent
            text: qsTr("WordsList")
            font.pixelSize: 20
            font.bold: true
            color: "white"
        }

        Button {
            anchors.right: parent.right
            anchors.rightMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("导入")
            flat: true
            contentItem: Text {
                text: parent.text
                color: "white"
                font.pixelSize: 14
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            onClicked: fileDialog.open()
        }
    }

    ColumnLayout {
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 24
        spacing: 20

        Label {
            text: qsTr("背生词待复习: %1 | 练拼写待复习: %2").arg(wordsModel.mode1DueCount).arg(wordsModel.mode2DueCount)
            font.pixelSize: 16
            color: "#555"
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
        }

        Label {
            text: qsTr("词库总量: %1").arg(wordsModel.totalWords)
            font.pixelSize: 13
            color: "#999"
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
        }

        Item { Layout.fillHeight: true; Layout.fillWidth: true }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 160
            color: "white"
            radius: 12
            border.color: "#e0e0e0"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 8

                Label {
                    text: qsTr("背生词")
                    font.pixelSize: 20
                    font.bold: true
                    color: "#4CAF50"
                    Layout.fillWidth: true
                }

                Label {
                    text: qsTr("显示单词，选择认识/不认识")
                    font.pixelSize: 13
                    color: "#777"
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }

                Button {
                    text: qsTr("开始")
                    highlighted: true
                    Layout.alignment: Qt.AlignRight
                    enabled: wordsModel.mode1DueCount > 0
                    onClicked: {
                        mode1Window.show()
                        wordsModel.startSession(1)
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 160
            color: "white"
            radius: 12
            border.color: "#e0e0e0"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 8

                Label {
                    text: qsTr("练拼写")
                    font.pixelSize: 20
                    font.bold: true
                    color: "#2196F3"
                    Layout.fillWidth: true
                }

                Label {
                    text: qsTr("显示释义，输入对应单词拼写")
                    font.pixelSize: 13
                    color: "#777"
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }

                Button {
                    text: qsTr("开始")
                    highlighted: true
                    Layout.alignment: Qt.AlignRight
                    palette.button: "#2196F3"
                    enabled: wordsModel.mode2DueCount > 0
                    onClicked: {
                        mode2Window.show()
                        wordsModel.startSession(2)
                    }
                }
            }
        }

        Item { Layout.fillHeight: true; Layout.fillWidth: true }
    }

    Mode1Window {
        id: mode1Window
    }

    Mode2Window {
        id: mode2Window
    }

    EditorWindow {
        id: editorWindow
    }
}

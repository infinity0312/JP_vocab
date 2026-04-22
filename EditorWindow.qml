import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Window {
    id: editorWindow
    width: 640
    height: 720
    title: qsTr("词库管理")
    color: "#f5f5f5"

    property int filterMode: 0
    property int selectedWordIndex: -1
    property var selectedWordIndices: []
    property bool isSelectAll: false
    property string searchText: ""

    Rectangle {
        id: topBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 48
        color: "#FF9800"
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
                onClicked: editorWindow.hide()
            }

            Item { Layout.fillWidth: true }

            Label {
                text: qsTr("词库管理")
                font.pixelSize: 16
                font.bold: true
                color: "white"
            }

            Item { Layout.fillWidth: true }

            Label {
                text: qsTr("共 %1 词").arg(filteredModel.count)
                font.pixelSize: 13
                color: "#ffffff"
            }
        }
    }

    RowLayout {
        id: filterBar
        anchors.top: topBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 12
        height: 48
        spacing: 8

        TextField {
            id: searchField
            placeholderText: qsTr("搜索单词...")
            Layout.fillWidth: true
            Layout.preferredWidth: 150
            onTextChanged: searchText = text
        }

        Button {
            text: qsTr("全部")
            highlighted: filterMode === 0
            Layout.fillWidth: true
            onClicked: filterMode = 0
        }
        Button {
            text: qsTr("已背诵")
            highlighted: filterMode === 1
            Layout.fillWidth: true
            onClicked: filterMode = 1
        }
        Button {
            text: qsTr("未背诵")
            highlighted: filterMode === 2
            Layout.fillWidth: true
            onClicked: filterMode = 2
        }

        CheckBox {
            text: qsTr("全选")
            checked: isSelectAll
            onCheckedChanged: {
                if (checked) {
                    selectedWordIndices = []
                    for (var i = 0; i < filteredModel.count; i++) {
                        selectedWordIndices.push(filteredModel.get(i).wordIndex)
                    }
                    isSelectAll = true
                } else {
                    selectedWordIndices = []
                    isSelectAll = false
                }
            }
        }
    }

    ListView {
        id: wordList
        anchors.top: filterBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: bottomBar.top
        anchors.margins: 12
        clip: true
        spacing: 8

        model: filteredModel
        delegate: Rectangle {
            width: wordList.width
            height: 100
            color: selectedWordIndices.includes(wordIndex) ? "#e3f2fd" : "white"
            radius: 8
            border.color: selectedWordIndices.includes(wordIndex) ? "#2196f3" : "#e0e0e0"
            border.width: selectedWordIndices.includes(wordIndex) ? 2 : 1

            property int wordIndex: model.wordIndex

            RowLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.maximumWidth: 440
                    spacing: 2

                    RowLayout {
                        spacing: 6

                        Label {
                            text: model.word
                            font.pixelSize: 16
                            font.bold: true
                            color: "#333"
                        }
                    }

                    Label {
                        text: model.meaning
                        font.pixelSize: 13
                        color: "#666"
                        Layout.fillWidth: true
                        Layout.maximumWidth: 440
                        elide: Text.ElideRight
                    }

                    Label {
                        text: qsTr("下次复习: %1 | 间隔: %2天 | 重复: %3").arg(model.nextReview).arg(model.interval).arg(model.repetition)
                        font.pixelSize: 11
                        color: "#999"
                    }
                }

                ColumnLayout {
                    spacing: 1
                    Layout.fillWidth: false
                    Layout.preferredWidth: 80

                    Button {
                        text: qsTr("重置")
                        font.pixelSize: 12
                        Layout.fillWidth: true
                        Layout.preferredHeight: 40
                        onClicked: {
                            selectedWordIndex = wordIndex
                            confirmDialog.actionType = "reset"
                            confirmDialog.message = selectedWordIndices.length > 0
                                ? qsTr("确定要重置选中的 %1 个单词吗？").arg(selectedWordIndices.length)
                                : qsTr("确定要重置该单词的进度吗？")
                            confirmDialog.open()
                        }
                    }

                    Button {
                        text: qsTr("删除")
                        font.pixelSize: 12
                        Layout.fillWidth: true
                        Layout.preferredHeight: 40
                        palette.button: "#f44336"
                        palette.buttonText: "white"
                        onClicked: {
                            selectedWordIndex = wordIndex
                            confirmDialog.actionType = "delete"
                            confirmDialog.message = selectedWordIndices.length > 0
                                ? qsTr("确定要删除选中的 %1 个单词吗？").arg(selectedWordIndices.length)
                                : qsTr("确定要删除该单词吗？")
                            confirmDialog.open()
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: confirmDialog
        title: qsTr("确认操作")
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel

        property string actionType: ""
        property string message: ""

        Label {
            text: confirmDialog.message
            font.pixelSize: 14
        }

        onAccepted: {
            if (actionType === "reset") {
                if (selectedWordIndices.length > 0) {
                    wordsModel.resetWordsProgress(selectedWordIndices)
                    selectedWordIndices = []
                    isSelectAll = false
                } else {
                    wordsModel.resetWordProgress(selectedWordIndex)
                }
            } else if (actionType === "delete") {
                if (selectedWordIndices.length > 0) {
                    wordsModel.removeWords(selectedWordIndices)
                    selectedWordIndices = []
                    isSelectAll = false
                } else {
                    wordsModel.removeWord(selectedWordIndex)
                }
            }
        }
    }

    Rectangle {
        id: bottomBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 56
        color: "#ffffff"
        border.color: "#e0e0e0"
        border.width: 1

        Button {
            anchors.centerIn: parent
            width: 120
            height: 36
            text: qsTr("添加单词")
            highlighted: true
            onClicked: {
                selectedWordIndex = -1
                wordEditDialog.open()
            }
        }
    }

    Dialog {
        id: wordEditDialog
        title: qsTr("添加单词")
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel

        ColumnLayout {
            spacing: 12

            TextField {
                id: newWordField
                placeholderText: qsTr("请输入单词")
                Layout.fillWidth: true
            }

            TextField {
                id: newMeaningField
                placeholderText: qsTr("请输入释义")
                Layout.fillWidth: true
            }
        }

        onAccepted: {
            if (String(newWordField.text).trim() !== "" && String(newMeaningField.text).trim() !== "") {
                wordsModel.addWord(String(newWordField.text).trim(), String(newMeaningField.text).trim())
            }
        }

        onOpened: {
            newWordField.text = ""
            newMeaningField.text = ""
        }
    }

    ListModel {
        id: filteredModel

        function refresh() {
            filteredModel.clear()
            for (var i = 0; i < wordsModel.rowCount(); i++) {
                var idx = wordsModel.index(i, 0)
                var nextReviewStr = wordsModel.data(idx, 262)
                var parts = nextReviewStr.split("-")
                var nrDate = new Date(parseInt(parts[0]), parseInt(parts[1]) - 1, parseInt(parts[2]))
                var today = new Date()
                today.setHours(0, 0, 0, 0)
                var isDue = nrDate <= today

                var word = wordsModel.data(idx, 257)
                var meaning = wordsModel.data(idx, 258)
                var matchSearch = searchText === "" || word.toLowerCase().includes(searchText.toLowerCase()) || meaning.toLowerCase().includes(searchText.toLowerCase())

                if (!matchSearch) return

                if (filterMode === 0) {
                    filteredModel.append({
                        "wordIndex": i,
                        "word": word,
                        "meaning": meaning,
                        "nextReview": nextReviewStr,
                        "interval": wordsModel.data(idx, 261),
                        "repetition": wordsModel.data(idx, 259),
                        "isMemorized": !isDue
                    })
                } else if (filterMode === 1 && !isDue) {
                    filteredModel.append({
                        "wordIndex": i,
                        "word": word,
                        "meaning": meaning,
                        "nextReview": nextReviewStr,
                        "interval": wordsModel.data(idx, 261),
                        "repetition": wordsModel.data(idx, 259),
                        "isMemorized": !isDue
                    })
                } else if (filterMode === 2 && isDue) {
                    filteredModel.append({
                        "wordIndex": i,
                        "word": word,
                        "meaning": meaning,
                        "nextReview": nextReviewStr,
                        "interval": wordsModel.data(idx, 261),
                        "repetition": wordsModel.data(idx, 259),
                        "isMemorized": !isDue
                    })
                }
            }
        }
    }

    Connections {
        target: wordsModel
        function onTotalWordsChanged() { filteredModel.refresh() }
        function onDueCountChanged() { filteredModel.refresh() }
    }

    onFilterModeChanged: {
            selectedWordIndices = []
            isSelectAll = false
            filteredModel.refresh()
        }
    onSearchTextChanged: {
            selectedWordIndices = []
            isSelectAll = false
            filteredModel.refresh()
        }
    onVisibleChanged: if (visible) filteredModel.refresh()
}

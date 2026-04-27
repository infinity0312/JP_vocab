#include "wordsmodel.h"
#include <QFile>
#include <QTextStream>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include <algorithm>
#include <random>
#include <QSet>
#include <QPair>

WordsModel::WordsModel(QObject *parent) : QAbstractListModel(parent) { loadData(); }

int WordsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_allWords.size();
}

QVariant WordsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_allWords.size())
        return {};
    const Word &w = m_allWords[index.row()];
    switch (role) {
    case WordRole:
        return w.word();
    case MeaningRole:
        return w.meaning();
    case RepetitionRole:
        return w.repetition();
    case EasinessFactorRole:
        return w.easinessFactor();
    case IntervalRole:
        return w.interval();
    case NextReviewRole:
        return w.nextReview().toString(Qt::ISODate);
    case SessionCompletedRole:
        return w.sessionCompleted();
    case Mode1CompletedRole:
        return w.mode1Completed();
    case Mode2CompletedRole:
        return w.mode2Completed();
    default:
        return {};
    }
}

QHash<int, QByteArray> WordsModel::roleNames() const
{
    return {{WordRole, "word"},
            {MeaningRole, "meaning"},
            {RepetitionRole, "repetition"},
            {EasinessFactorRole, "easinessFactor"},
            {IntervalRole, "interval"},
            {NextReviewRole, "nextReview"},
            {SessionCompletedRole, "sessionCompleted"},
            {Mode1CompletedRole, "mode1Completed"},
            {Mode2CompletedRole, "mode2Completed"}};
}

QString WordsModel::currentWord() const
{
    if (m_currentQueueIndex >= 0 && m_currentQueueIndex < m_reviewQueue.size())
        return m_allWords[m_reviewQueue[m_currentQueueIndex]].word();
    return {};
}

QString WordsModel::currentMeaning() const
{
    if (m_currentQueueIndex >= 0 && m_currentQueueIndex < m_reviewQueue.size())
        return m_allWords[m_reviewQueue[m_currentQueueIndex]].meaning();
    return {};
}

bool WordsModel::hasCurrentWord() const { return m_currentQueueIndex >= 0 && m_currentQueueIndex < m_reviewQueue.size(); }

int WordsModel::dueCount() const
{
    int c = 0;
    for (const auto &w : m_allWords)
        if (w.isDueToday())
            c++;
    return c;
}

int WordsModel::mode1DueCount() const
{
    int c = 0;
    for (const auto &w : m_allWords)
        if (!w.mode1Completed())
            c++;
    return c;
}

int WordsModel::mode2DueCount() const
{
    int c = 0;
    for (const auto &w : m_allWords)
        if (!w.mode2Completed())
            c++;
    return c;
}

int WordsModel::completedCount() const { return m_completedCount; }

int WordsModel::remainingCount() const
{
    return m_reviewQueue.size() - m_completedCount;
}

int WordsModel::totalWords() const { return m_allWords.size(); }

int WordsModel::memorizedCount() const
{
    int c = 0;
    for (const auto &w : m_allWords)
        if (!w.isDueToday())
            c++;
    return c;
}

bool WordsModel::meaningRevealed() const { return m_meaningRevealed; }
bool WordsModel::answerShown() const { return m_answerShown; }

bool WordsModel::canGoBack() const { return !m_actionHistory.isEmpty(); }

int WordsModel::wrongWordCount() const
{
    QSet<int> seen;
    int count = 0;
    for (int idx : m_reviewQueue) {
        if (m_allWords[idx].sessionAttempts() > 0
            && !m_allWords[idx].sessionCompleted()
            && !seen.contains(idx)) {
            seen.insert(idx);
            count++;
        }
    }
    return count;
}

void WordsModel::importCSV(const QUrl &fileUrl, const QString &delimiter)
{
    QString filePath = fileUrl.toLocalFile();
    if (filePath.isEmpty())
        filePath = fileUrl.path();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit importFailed(QString("Cannot open file: %1").arg(filePath));
        return;
    }

    QList<Word> newWords;
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    int lineNum = 0;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        lineNum++;
        if (line.isEmpty())
            continue;
        QStringList parts = line.split(delimiter);
        if (parts.size() < 2) {
            emit importFailed(QString("Line %1: insufficient fields").arg(lineNum));
            continue;
        }
        QString word = parts[0].trimmed();
        QString meaning = parts[1].trimmed();
        if (word.isEmpty() || meaning.isEmpty())
            continue;
        newWords.append(Word(word, meaning));
    }
    file.close();

    if (newWords.isEmpty()) {
        emit importFailed("No valid words found in file");
        return;
    }

    beginResetModel();
    m_allWords.append(newWords);
    endResetModel();

    saveData();
    emit totalWordsChanged();
    emit dueCountChanged();
    emit mode1DueCountChanged();
    emit mode2DueCountChanged();
    emit importCompleted(newWords.size());
}

void WordsModel::addWord(const QString &word, const QString &meaning)
{
    if (word.trimmed().isEmpty() || meaning.trimmed().isEmpty())
        return;

    beginResetModel();
    m_allWords.append(Word(word.trimmed(), meaning.trimmed()));
    endResetModel();

    saveData();
    emit totalWordsChanged();
    emit dueCountChanged();
    emit mode1DueCountChanged();
    emit mode2DueCountChanged();
}

void WordsModel::startSession(int mode)
{
    m_currentMode = mode;
    m_completedCount = 0;
    m_currentQueueIndex = -1;
    m_meaningRevealed = false;
    m_answerShown = false;
    m_actionHistory.clear();
    emit canGoBackChanged();

    for (auto &w : m_allWords) {
        w.resetSessionAttempts();
    }

    buildReviewQueue();

    if (m_reviewQueue.isEmpty()) {
        emit wrongWordCountChanged();
        return;
    }

    m_currentQueueIndex = 0;
    emit currentWordChanged();
    emit currentMeaningChanged();
    emit hasCurrentWordChanged();
    emitProgressChanged();
    emit meaningRevealedChanged();
    emit answerShownChanged();
    emit wrongWordCountChanged();
}

void WordsModel::markKnown()
{
    if (!hasCurrentWord())
        return;

    int idx = m_reviewQueue[m_currentQueueIndex];

    UndoRecord rec;
    rec.wordIndex = idx;
    rec.queueIndex = m_currentQueueIndex;
    rec.wasKnown = true;
    rec.prevSessionAttempts = m_allWords[idx].sessionAttempts();
    rec.prevRepetition = m_allWords[idx].repetition();
    rec.prevEasinessFactor = m_allWords[idx].easinessFactor();
    rec.prevInterval = m_allWords[idx].interval();
    m_actionHistory.push(rec);

    int attempts = m_allWords[idx].sessionAttempts();
    int quality = calculateQuality(attempts);

    m_allWords[idx].updateSM2(quality);
    m_allWords[idx].setSessionCompleted(true);
    if (m_currentMode == 1) {
        m_allWords[idx].setMode1Completed(true);
        emit mode1DueCountChanged();
    } else if (m_currentMode == 2) {
        m_allWords[idx].setMode2Completed(true);
        emit mode2DueCountChanged();
    }
    m_completedCount++;

    saveData();
    emitProgressChanged();
    emit canGoBackChanged();
    emit wrongWordCountChanged();
    advanceToNext();
}

void WordsModel::markUnknown()
{
    if (!hasCurrentWord())
        return;

    int idx = m_reviewQueue[m_currentQueueIndex];

    UndoRecord rec;
    rec.wordIndex = idx;
    rec.queueIndex = m_currentQueueIndex;
    rec.wasKnown = false;
    rec.prevSessionAttempts = m_allWords[idx].sessionAttempts();
    rec.prevRepetition = m_allWords[idx].repetition();
    rec.prevEasinessFactor = m_allWords[idx].easinessFactor();
    rec.prevInterval = m_allWords[idx].interval();
    m_actionHistory.push(rec);

    m_allWords[idx].incrementSessionAttempts();

    m_reviewQueue.append(idx);
    m_meaningRevealed = true;
    emit meaningRevealedChanged();

    emit canGoBackChanged();
    emit wrongWordCountChanged();
    advanceToNext();
}

bool WordsModel::checkAnswer(const QString &answer)
{
    if (!hasCurrentWord())
        return false;

    int idx = m_reviewQueue[m_currentQueueIndex];

    UndoRecord rec;
    rec.wordIndex = idx;
    rec.queueIndex = m_currentQueueIndex;
    rec.prevSessionAttempts = m_allWords[idx].sessionAttempts();
    rec.prevRepetition = m_allWords[idx].repetition();
    rec.prevEasinessFactor = m_allWords[idx].easinessFactor();
    rec.prevInterval = m_allWords[idx].interval();

    QString correct = m_allWords[idx].word().trimmed();
    QString input = answer.trimmed();

    correct.replace(QRegularExpression("\\([^)]*\\)"), "");
    correct.replace(QRegularExpression("\\[[^)]*\\]"), "");
    correct.replace("～", "");
    correct.replace("・", "");
    correct.replace("……", "");

    if (input.compare(correct, Qt::CaseInsensitive) == 0) {
        rec.wasKnown = true;
        m_actionHistory.push(rec);
        emit canGoBackChanged();

        int attempts = m_allWords[idx].sessionAttempts();
        int quality = calculateQuality(attempts);
        m_allWords[idx].updateSM2(quality);
        m_allWords[idx].setSessionCompleted(true);
        if (m_currentMode == 1)
            m_allWords[idx].setMode1Completed(true);
        else if (m_currentMode == 2)
            m_allWords[idx].setMode2Completed(true);
        m_completedCount++;

        saveData();
        emitProgressChanged();
        emit wrongWordCountChanged();
        return true;
    }

    rec.wasKnown = false;
    m_actionHistory.push(rec);
    emit canGoBackChanged();

    m_allWords[idx].incrementSessionAttempts();
    m_reviewQueue.append(idx);
    m_answerShown = true;
    emit answerShownChanged();
    emit wrongWordCountChanged();
    return false;
}

void WordsModel::nextWord()
{
    advanceToNext();
}

void WordsModel::revealMeaning()
{
    m_meaningRevealed = true;
    emit meaningRevealedChanged();
}

void WordsModel::goBack()
{
    if (m_actionHistory.isEmpty())
        return;

    UndoRecord rec = m_actionHistory.pop();
    int idx = rec.wordIndex;
    Word &w = m_allWords[idx];

    if (rec.wasKnown) {
        w.setRepetition(rec.prevRepetition);
        w.setEasinessFactor(rec.prevEasinessFactor);
        w.setInterval(rec.prevInterval);
        w.setSessionAttempts(rec.prevSessionAttempts);
        w.setSessionCompleted(false);
        if (m_currentMode == 1) {
            w.setMode1Completed(false);
            emit mode1DueCountChanged();
        } else if (m_currentMode == 2) {
            w.setMode2Completed(false);
            emit mode2DueCountChanged();
        }
        m_completedCount--;
    } else {
        w.setSessionAttempts(rec.prevSessionAttempts);
        if (!m_reviewQueue.isEmpty() && m_reviewQueue.last() == idx)
            m_reviewQueue.removeLast();
    }

    m_currentQueueIndex = rec.queueIndex;
    m_meaningRevealed = false;
    m_answerShown = false;

    saveData();
    emit currentWordChanged();
    emit currentMeaningChanged();
    emit hasCurrentWordChanged();
    emit meaningRevealedChanged();
    emit answerShownChanged();
    emit canGoBackChanged();
    emit wrongWordCountChanged();
    emitProgressChanged();
}

void WordsModel::exportWrongWords(const QUrl &fileUrl)
{
    QSet<int> seen;
    QList<QPair<QString, QString>> wrongWords;
    for (int idx : m_reviewQueue) {
        if (m_allWords[idx].sessionAttempts() > 0
            && !m_allWords[idx].sessionCompleted()
            && !seen.contains(idx)) {
            seen.insert(idx);
            wrongWords.append({m_allWords[idx].word(), m_allWords[idx].meaning()});
        }
    }

    if (wrongWords.isEmpty())
        return;

    QString filePath = fileUrl.toLocalFile();
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out.setGenerateByteOrderMark(true);
    for (const auto &pair : wrongWords) {
        out << pair.first << "," << pair.second << "\n";
    }
    file.close();
}

void WordsModel::clearSession()
{
    m_reviewQueue.clear();
    m_currentQueueIndex = -1;
    m_completedCount = 0;
    m_currentMode = 0;
    m_meaningRevealed = false;
    m_answerShown = false;
    m_actionHistory.clear();

    for (auto &w : m_allWords) {
        w.resetSessionAttempts();
        w.setSessionCompleted(false);
    }

    emit currentWordChanged();
    emit currentMeaningChanged();
    emit hasCurrentWordChanged();
    emit meaningRevealedChanged();
    emit answerShownChanged();
    emit canGoBackChanged();
    emit wrongWordCountChanged();
    emitProgressChanged();
}

void WordsModel::saveData()
{
    QString path = dataFilePath();
    QDir().mkpath(QFileInfo(path).absolutePath());

    QJsonArray arr;
    for (const auto &w : m_allWords)
        arr.append(w.toJson());

    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QJsonDocument doc(arr);
        file.write(doc.toJson());
        file.close();
    }
}

void WordsModel::loadData()
{
    QString path = dataFilePath();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isArray())
        return;

    beginResetModel();
    m_allWords.clear();
    const QJsonArray arr = doc.array();
    for (const auto &v : arr) {
        if (v.isObject())
            m_allWords.append(Word::fromJson(v.toObject()));
    }
    endResetModel();

    emit totalWordsChanged();
    emit dueCountChanged();
    emit mode1DueCountChanged();
    emit mode2DueCountChanged();
}

void WordsModel::advanceToNext()
{
    int next = m_currentQueueIndex + 1;
    while (next < m_reviewQueue.size()) {
        int idx = m_reviewQueue[next];
        if (!m_allWords[idx].sessionCompleted())
            break;
        next++;
    }

    if (next >= m_reviewQueue.size()) {
        bool allDone = true;
        for (int i : m_reviewQueue) {
            if (!m_allWords[i].sessionCompleted()) {
                allDone = false;
                break;
            }
        }
        if (allDone) {
            m_currentQueueIndex = -1;
            emit currentWordChanged();
            emit currentMeaningChanged();
            emit hasCurrentWordChanged();
            emit sessionCompleted();
            return;
        }
        m_currentQueueIndex = next;
    } else {
        m_currentQueueIndex = next;
    }

    m_meaningRevealed = false;
    m_answerShown = false;
    emit currentWordChanged();
    emit currentMeaningChanged();
    emit hasCurrentWordChanged();
    emit meaningRevealedChanged();
    emit answerShownChanged();
}

void WordsModel::buildReviewQueue()
{
    m_reviewQueue.clear();
    for (int i = 0; i < m_allWords.size(); ++i) {
        bool isDue = m_allWords[i].isDueToday();
        bool notCompletedInMode = false;
        if (m_currentMode == 1)
            notCompletedInMode = !m_allWords[i].mode1Completed();
        else if (m_currentMode == 2)
            notCompletedInMode = !m_allWords[i].mode2Completed();

        if (isDue || notCompletedInMode)
            m_reviewQueue.append(i);
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(m_reviewQueue.begin(), m_reviewQueue.end(), g);
}

int WordsModel::calculateQuality(int attempts) const
{
    if (attempts == 0)
        return 5;
    if (attempts == 1)
        return 3;
    return 1;
}

void WordsModel::emitProgressChanged()
{
    emit completedCountChanged();
    emit remainingCountChanged();
    emit dueCountChanged();
    emit mode1DueCountChanged();
    emit mode2DueCountChanged();
    emit memorizedCountChanged();
}

QString WordsModel::dataFilePath() const
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return dir + "/words_data.json";
}

void WordsModel::setWordNextReview(int index, const QDate &date)
{
    if (index < 0 || index >= m_allWords.size())
        return;
    m_allWords[index].setNextReview(date);
    saveData();
    emit dataChanged(this->index(index), this->index(index), {NextReviewRole});
    emit dueCountChanged();
    emit memorizedCountChanged();
}

void WordsModel::removeWord(int index)
{
    if (index < 0 || index >= m_allWords.size())
        return;
    beginRemoveRows(QModelIndex(), index, index);
    m_allWords.removeAt(index);
    endRemoveRows();
    saveData();
    emit totalWordsChanged();
    emit dueCountChanged();
    emit mode1DueCountChanged();
    emit mode2DueCountChanged();
    emit memorizedCountChanged();
}

void WordsModel::resetWordProgress(int index)
{
    if (index < 0 || index >= m_allWords.size())
        return;
    m_allWords[index].setRepetition(0);
    m_allWords[index].setEasinessFactor(2.5);
    m_allWords[index].setInterval(0);
    m_allWords[index].setNextReview(QDate::currentDate());
    m_allWords[index].setMode1Completed(false);
    m_allWords[index].setMode2Completed(false);
    saveData();
    emit dataChanged(this->index(index), this->index(index),
                     {RepetitionRole, EasinessFactorRole, IntervalRole, NextReviewRole, Mode1CompletedRole, Mode2CompletedRole});
    emit dueCountChanged();
    emit mode1DueCountChanged();
    emit mode2DueCountChanged();
    emit memorizedCountChanged();
}

void WordsModel::removeWords(const QList<int> &indices)
{
    if (indices.isEmpty())
        return;
    QList<int> sortedIndices = indices;
    std::sort(sortedIndices.begin(), sortedIndices.end(), std::greater<int>());
    beginResetModel();
    for (int idx : sortedIndices) {
        if (idx >= 0 && idx < m_allWords.size()) {
            m_allWords.removeAt(idx);
        }
    }
    endResetModel();
    saveData();
    emit totalWordsChanged();
    emit dueCountChanged();
    emit mode1DueCountChanged();
    emit mode2DueCountChanged();
    emit memorizedCountChanged();
}

void WordsModel::resetWordsProgress(const QList<int> &indices)
{
    if (indices.isEmpty())
        return;
    beginResetModel();
    for (int idx : indices) {
        if (idx >= 0 && idx < m_allWords.size()) {
            m_allWords[idx].setRepetition(0);
            m_allWords[idx].setEasinessFactor(2.5);
            m_allWords[idx].setInterval(0);
            m_allWords[idx].setNextReview(QDate::currentDate());
            m_allWords[idx].setMode1Completed(false);
            m_allWords[idx].setMode2Completed(false);
        }
    }
    endResetModel();
    saveData();
    emit dueCountChanged();
    emit mode1DueCountChanged();
    emit mode2DueCountChanged();
    emit memorizedCountChanged();
}

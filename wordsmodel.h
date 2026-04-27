#ifndef WORDSMODEL_H
#define WORDSMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QStack>
#include "words.h"

class WordsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString currentWord READ currentWord NOTIFY currentWordChanged)
    Q_PROPERTY(QString currentMeaning READ currentMeaning NOTIFY currentMeaningChanged)
    Q_PROPERTY(bool hasCurrentWord READ hasCurrentWord NOTIFY hasCurrentWordChanged)
    Q_PROPERTY(int dueCount READ dueCount NOTIFY dueCountChanged)
    Q_PROPERTY(int mode1DueCount READ mode1DueCount NOTIFY mode1DueCountChanged)
    Q_PROPERTY(int mode2DueCount READ mode2DueCount NOTIFY mode2DueCountChanged)
    Q_PROPERTY(int completedCount READ completedCount NOTIFY completedCountChanged)
    Q_PROPERTY(int remainingCount READ remainingCount NOTIFY remainingCountChanged)
    Q_PROPERTY(int totalWords READ totalWords NOTIFY totalWordsChanged)
    Q_PROPERTY(bool meaningRevealed READ meaningRevealed NOTIFY meaningRevealedChanged)
    Q_PROPERTY(bool answerShown READ answerShown NOTIFY answerShownChanged)
    Q_PROPERTY(int memorizedCount READ memorizedCount NOTIFY memorizedCountChanged)
    Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY canGoBackChanged)
    Q_PROPERTY(int wrongWordCount READ wrongWordCount NOTIFY wrongWordCountChanged)

public:
    enum Roles
    {
        WordRole = Qt::UserRole + 1,
        MeaningRole,
        RepetitionRole,
        EasinessFactorRole,
        IntervalRole,
        NextReviewRole,
        SessionCompletedRole,
        Mode1CompletedRole,
        Mode2CompletedRole
    };

    explicit WordsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString currentWord() const;
    QString currentMeaning() const;
    bool hasCurrentWord() const;
    int dueCount() const;
    int mode1DueCount() const;
    int mode2DueCount() const;
    int completedCount() const;
    int remainingCount() const;
    int totalWords() const;
    bool meaningRevealed() const;
    bool answerShown() const;
    bool canGoBack() const;
    int wrongWordCount() const;
    int memorizedCount() const;

    Q_INVOKABLE void importCSV(const QUrl &fileUrl, const QString &delimiter);
    Q_INVOKABLE void startSession(int mode);
    Q_INVOKABLE void markKnown();
    Q_INVOKABLE void markUnknown();
    Q_INVOKABLE bool checkAnswer(const QString &answer);
    Q_INVOKABLE void nextWord();
    Q_INVOKABLE void revealMeaning();
    Q_INVOKABLE void clearSession();
    Q_INVOKABLE void saveData();
    Q_INVOKABLE void loadData();
    Q_INVOKABLE void setWordNextReview(int index, const QDate &date);
    Q_INVOKABLE void removeWord(int index);
    Q_INVOKABLE void removeWords(const QList<int> &indices);
    Q_INVOKABLE void resetWordProgress(int index);
    Q_INVOKABLE void resetWordsProgress(const QList<int> &indices);
    Q_INVOKABLE void addWord(const QString &word, const QString &meaning);
    Q_INVOKABLE void goBack();
    Q_INVOKABLE void exportWrongWords(const QUrl &fileUrl);

signals:
    void currentWordChanged();
    void currentMeaningChanged();
    void hasCurrentWordChanged();
    void dueCountChanged();
    void mode1DueCountChanged();
    void mode2DueCountChanged();
    void completedCountChanged();
    void remainingCountChanged();
    void totalWordsChanged();
    void meaningRevealedChanged();
    void answerShownChanged();
    void sessionCompleted();
    void memorizedCountChanged();
    void canGoBackChanged();
    void wrongWordCountChanged();
    void importCompleted(int count);
    void importFailed(const QString &error);

private:
    QList<Word> m_allWords;
    QList<int> m_reviewQueue;
    struct UndoRecord {
        int wordIndex = -1;
        int queueIndex = -1;
        bool wasKnown = false;
        int prevSessionAttempts = 0;
        int prevRepetition = 0;
        double prevEasinessFactor = 2.5;
        int prevInterval = 0;
    };
    QStack<UndoRecord> m_actionHistory;
    int m_currentQueueIndex = -1;
    int m_completedCount = 0;
    int m_currentMode = 0;
    bool m_meaningRevealed = false;
    bool m_answerShown = false;

    void advanceToNext();
    void buildReviewQueue();
    int calculateQuality(int attempts) const;
    void emitProgressChanged();
    QString dataFilePath() const;
};

#endif // WORDSMODEL_H

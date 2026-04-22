#ifndef WORDS_H
#define WORDS_H

#include <QString>
#include <QDate>
#include <QJsonObject>

class Word
{
public:
    Word();
    Word(const QString &word, const QString &meaning);

    QString word() const;
    void setWord(const QString &w);

    QString meaning() const;
    void setMeaning(const QString &m);

    int repetition() const;
    void setRepetition(int r);

    double easinessFactor() const;
    void setEasinessFactor(double ef);

    int interval() const;
    void setInterval(int i);

    QDate nextReview() const;
    void setNextReview(const QDate &date);

    int sessionAttempts() const;
    void setSessionAttempts(int a);
    void incrementSessionAttempts();
    void resetSessionAttempts();

    bool sessionCompleted() const;
    void setSessionCompleted(bool c);

    bool mode1Completed() const;
    void setMode1Completed(bool c);

    bool mode2Completed() const;
    void setMode2Completed(bool c);

    void updateSM2(int quality);
    bool isDueToday() const;

    QJsonObject toJson() const;
    static Word fromJson(const QJsonObject &obj);

private:
    QString m_word;
    QString m_meaning;
    int m_repetition = 0;
    double m_easinessFactor = 2.5;
    int m_interval = 0;
    QDate m_nextReview;
    int m_sessionAttempts = 0;
    bool m_sessionCompleted = false;
    bool m_mode1Completed = false;
    bool m_mode2Completed = false;
};

#endif // WORDS_H

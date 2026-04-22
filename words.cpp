#include "words.h"
#include <cmath>

Word::Word() : m_nextReview(QDate::currentDate()) {}

Word::Word(const QString &word, const QString &meaning)
    : m_word(word), m_meaning(meaning), m_nextReview(QDate::currentDate())
{
}

QString Word::word() const { return m_word; }
void Word::setWord(const QString &w) { m_word = w; }

QString Word::meaning() const { return m_meaning; }
void Word::setMeaning(const QString &m) { m_meaning = m; }

int Word::repetition() const { return m_repetition; }
void Word::setRepetition(int r) { m_repetition = r; }

double Word::easinessFactor() const { return m_easinessFactor; }
void Word::setEasinessFactor(double ef) { m_easinessFactor = ef; }

int Word::interval() const { return m_interval; }
void Word::setInterval(int i) { m_interval = i; }

QDate Word::nextReview() const { return m_nextReview; }
void Word::setNextReview(const QDate &date) { m_nextReview = date; }

int Word::sessionAttempts() const { return m_sessionAttempts; }
void Word::setSessionAttempts(int a) { m_sessionAttempts = a; }
void Word::incrementSessionAttempts() { m_sessionAttempts++; }
void Word::resetSessionAttempts() { m_sessionAttempts = 0; }

bool Word::sessionCompleted() const { return m_sessionCompleted; }
void Word::setSessionCompleted(bool c) { m_sessionCompleted = c; }

bool Word::mode1Completed() const { return m_mode1Completed; }
void Word::setMode1Completed(bool c) { m_mode1Completed = c; }

bool Word::mode2Completed() const { return m_mode2Completed; }
void Word::setMode2Completed(bool c) { m_mode2Completed = c; }

void Word::updateSM2(int quality)
{
    if (quality >= 3) {
        if (m_repetition == 0) {
            m_interval = 1;
        } else if (m_repetition == 1) {
            m_interval = 6;
        } else {
            m_interval = static_cast<int>(std::round(m_interval * m_easinessFactor));
        }
        m_repetition++;
    } else {
        m_repetition = 0;
        m_interval = 1;
    }

    double newEF = m_easinessFactor + (0.1 - (5 - quality) * (0.08 + (5 - quality) * 0.02));
    m_easinessFactor = (newEF < 1.3) ? 1.3 : newEF;

    m_nextReview = QDate::currentDate().addDays(m_interval);
}

bool Word::isDueToday() const
{
    return m_nextReview <= QDate::currentDate();
}

QJsonObject Word::toJson() const
{
    return QJsonObject{
        {"word", m_word},
        {"meaning", m_meaning},
        {"repetition", m_repetition},
        {"easinessFactor", m_easinessFactor},
        {"interval", m_interval},
        {"nextReview", m_nextReview.toString(Qt::ISODate)},
        {"mode1Completed", m_mode1Completed},
        {"mode2Completed", m_mode2Completed}};
}

Word Word::fromJson(const QJsonObject &obj)
{
    Word w;
    w.m_word = obj["word"].toString();
    w.m_meaning = obj["meaning"].toString();
    w.m_repetition = obj["repetition"].toInt();
    w.m_easinessFactor = obj["easinessFactor"].toDouble();
    w.m_interval = obj["interval"].toInt();
    w.m_nextReview = QDate::fromString(obj["nextReview"].toString(), Qt::ISODate);
    if (!w.m_nextReview.isValid())
        w.m_nextReview = QDate::currentDate();
    w.m_mode1Completed = obj["mode1Completed"].toBool(false);
    w.m_mode2Completed = obj["mode2Completed"].toBool(false);
    return w;
}

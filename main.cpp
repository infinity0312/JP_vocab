#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include "wordsmodel.h"

void logMessage(const QString &msg)
{
    QFile logFile("appWordsList_debug.log");
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        logFile.write(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz").toUtf8());
        logFile.write(" - ");
        logFile.write(msg.toUtf8());
        logFile.write("\n");
        logFile.close();
    }
    qDebug() << msg;
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    logMessage("Application starting...");

    QQuickStyle::setStyle("Material");

    app.setOrganizationName("WordsList");
    app.setApplicationName("WordsList");

    logMessage("Styles and app name set");

    WordsModel wordsModel;
    logMessage("WordsModel created");

    QQmlApplicationEngine engine;
    logMessage("QQmlApplicationEngine created");

    engine.rootContext()->setContextProperty("wordsModel", &wordsModel);
    logMessage("Context property set");

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []()
        {
            logMessage("QML object creation failed!");
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::warnings,
        &app,
        [](const QList<QQmlError> &warnings)
        {
            for (const auto &w : warnings)
            {
                logMessage(QString("QML Warning: %1 (%2:%3)").arg(w.description()).arg(w.url().toString()).arg(w.line()));
            }
        });

    logMessage("About to load QML module...");

    QUrl moduleUrl = QUrl(QStringLiteral("qrc:/WordsList/Main.qml"));
    logMessage(QString("Module URL: %1").arg(moduleUrl.toString()));

    engine.loadFromModule("WordsList", "Main");
    logMessage("loadFromModule called");

    if (engine.rootObjects().isEmpty())
    {
        logMessage("ERROR: No root objects loaded!");
        return -1;
    }

    logMessage(QString("Success: %1 root objects loaded").arg(engine.rootObjects().size()));

    return app.exec();
}

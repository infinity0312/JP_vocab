#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include "wordsmodel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQuickStyle::setStyle("Material");

    app.setOrganizationName("WordsList");
    app.setApplicationName("WordsList");

    WordsModel wordsModel;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("wordsModel", &wordsModel);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("WordsList", "Main");

    return app.exec();
}

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QDebug>
#include <QtQuick/QQuickItem>

#include <memory>

#include "backend.hpp"

int main(int argc, char *argv[])
{
    qDebug() << "Qt Version: " << qVersion();
    QGuiApplication app(argc, argv);

    qmlRegisterType<BackEnd>("hangul.backend", 1, 0, "BackEnd");

    QQmlApplicationEngine engine;
    QQmlComponent window(&engine,
                         QUrl(QStringLiteral("qrc:/qml/main.qml")));

    std::unique_ptr<QObject> object(window.create());

    QObject *grid = object->findChild<QObject*>("grid");

    QQmlComponent jamoview(&engine,
                           QUrl(QStringLiteral("qrc:/qml/jamoview.qml")));

    qDebug() << grid << "\n";

    return app.exec();
}

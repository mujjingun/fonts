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

    std::unique_ptr<QObject> window_obj(window.create());

    QQmlComponent jamoview(&engine,
                           QUrl(QStringLiteral("qrc:/qml/jamoview.qml")));
    std::unique_ptr<QQuickItem> jamoview_obj(
        qobject_cast<QQuickItem*>(jamoview.beginCreate(engine.rootContext())));
    jamoview_obj->setParentItem(window_obj->findChild<QQuickItem*>("consonants_view"));
    jamoview.completeCreate();

    return app.exec();
}

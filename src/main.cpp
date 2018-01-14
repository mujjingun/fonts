/****************************************************************************
**
** Copyright (C) 2018 Gun Park.
** Author: Gun Park
** Contact: mujjingun@gmail.com
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/


#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QDebug>
#include <QQuickItem>

#include <memory>

#include "jamoview.hpp"

int main(int argc, char *argv[])
{
    qDebug() << "Qt Version: " << qVersion();
    QGuiApplication app(argc, argv);

    // Register JamoView as a QML type
    qmlRegisterType<JamoView>("hangul.jamoview", 1, 0, "JamoView");

    // Make Window
    QQmlApplicationEngine engine;
    QQmlComponent window_comp(&engine, QUrl(QStringLiteral("qrc:/qml/main.qml")));
    std::unique_ptr<QObject> window(window_comp.create());
    if (window_comp.isError())
        qDebug() << window_comp.errorString();

    // Invoke QML function from C++
    QVariant returnedValue;
    for (int i = 0; i < 24; ++i) {
        QMetaObject::invokeMethod(window.get(), "myQmlFunction",
            Q_ARG(QVariant, "T"));
    }

    return app.exec();
}

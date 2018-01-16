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
#include <QQmlContext>

#include <memory>

#include "jamomodel.hpp"

int main(int argc, char *argv[])
{
    qDebug() << "Qt Version: " << qVersion();
    QGuiApplication app(argc, argv);

    JamoModel model;
    QList<QVariant> path;
    path.append(QVariantMap{{"name", "PathLine"}, {"x", 50}, {"y", 50}});
    path.append(QVariantMap{{"name", "PathLine"}, {"x", 60}, {"y", 50}});
    path.append(QVariantMap{{"name", "PathLine"}, {"x", 60}, {"y", 60}});
    path.append(QVariantMap{{"name", "PathLine"}, {"x", 50}, {"y", 50}});
    model.addJamo(Jamo{"A", QPoint(0, 0), path});
    model.addJamo(Jamo{"B"});
    model.addJamo(Jamo{"C", QPoint(30, 0), path});
    model.addJamo(Jamo{"D"});

    // Make Window
    QQmlApplicationEngine engine;
    QQmlContext *context = engine.rootContext();
    context->setContextProperty("jamoModel", &model);

    QQmlComponent window_comp(&engine, QUrl(QStringLiteral("qrc:/qml/main.qml")));
    std::unique_ptr<QObject> window(window_comp.create());
    if (window_comp.isError())
        qDebug() << window_comp.errorString();

    return app.exec();
}

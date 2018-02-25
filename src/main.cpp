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

#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickItem>

#include <memory>

#include "formmodel.hpp"
#include "jamomodel.hpp"
#include "jamoview.hpp"
#include "controller.hpp"

/* Goals
 * 1. Make Hangul Fonts (11172 forms) from scratch
 * 2. Save to project file
 * 3. Open project file
 * 4. Import font from .otf - done!
 */

int main(int argc, char* argv[])
{
    qDebug() << "Qt Version: " << qVersion();
    QGuiApplication app(argc, argv);

    JamoModel consonant_model(
        { JamoName::KIYEOK,
          JamoName::SSANGKIYEOK,
          JamoName::NIEUN,
          JamoName::TIKEUT,
          JamoName::SSANGTIKEUT,
          JamoName::RIEUL,
          JamoName::MIEUM,
          JamoName::PIEUP,
          JamoName::SSANGPIEUP,
          JamoName::SIOS,
          JamoName::SSANGSIOS,
          JamoName::IEUNG,
          JamoName::CIEUC,
          JamoName::SSANGCIEUC,
          JamoName::CHIEUCH,
          JamoName::KHIEUHK,
          JamoName::THIEUTH,
          JamoName::PHIEUPH,
          JamoName::HEIUH },
        &app);

    // Register Types
    qmlRegisterType<JamoView>("fontmaker", 1, 0, "JamoView");
    qmlRegisterType<FormModel>("fontmaker", 1, 0, "FormModel");

    // Make Window
    QQmlApplicationEngine engine;
    QQmlContext*          context = engine.rootContext();
    context->setContextProperty("consonantJamoModel", &consonant_model);

    QQmlComponent window_comp(
        &engine, QUrl(QStringLiteral("qrc:/qml/main.qml")));
    std::unique_ptr<QObject> window(window_comp.create());
    if (window_comp.isError())
        qDebug() << window_comp.errorString();

    Controller menuhandler(window.get(), &consonant_model);

    return app.exec();
}

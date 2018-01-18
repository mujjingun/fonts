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
#include "menuhandler.hpp"
#include "jamoview.hpp"

int main(int argc, char *argv[])
{
    qDebug() << "Qt Version: " << qVersion();
    QGuiApplication app(argc, argv);

    JamoModel model;
    model.addJamo(Jamo{"Test", new Glyph(fontutils::Glyph::from_charstring("Test",
R"(-120 50 900 50 hstem
100 50 700 50 vstem

100 -120 rmoveto
800 1000 -800 hlineto

400 -459 rmoveto
-318 409 rlineto
636 hlineto

-286 -450 rmoveto
318 409 rlineto
-818 vlineto

-668 -41 rmoveto
318 409 318 -409 rlineto

-668 859 rmoveto
318 -409 -318 -409 rlineto
endchar")"), &model)});

    qmlRegisterType<JamoView>("fontmaker", 1, 0, "JamoView");
    qmlRegisterUncreatableType<Glyph>("fontmaker", 1, 0, "Glyph", "Glyph is uncreatable.");

    // Make Window
    QQmlApplicationEngine engine;
    QQmlContext *context = engine.rootContext();
    context->setContextProperty("jamoModel", &model);

    QQmlComponent window_comp(&engine, QUrl(QStringLiteral("qrc:/qml/main.qml")));
    std::unique_ptr<QObject> window(window_comp.create());
    if (window_comp.isError())
        qDebug() << window_comp.errorString();

    MenuHandler menuhandler(window.get(), &model);
    QObject::connect(window.get(), SIGNAL(fileOpenSignal(QVariant)),
                     &menuhandler, SLOT(fileOpenClicked(QVariant)));

    return app.exec();
}

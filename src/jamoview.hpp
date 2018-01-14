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

#ifndef JAMOVIEW_HPP
#define JAMOVIEW_HPP

#include <QQuickItem>
#include <QString>
#include <QQuickFramebufferObject>

#include "jamoviewrenderer.hpp"

class JamoView : public QQuickFramebufferObject
{
    Q_OBJECT
    Q_PROPERTY(QString name
                READ name
                WRITE setName
                NOTIFY nameChanged)

public:
    JamoView();

    QString name() const;
    void setName(const QString &name);

    QQuickFramebufferObject::Renderer *createRenderer() const override;

signals:
    void nameChanged() const;

private:
    QString m_name = "";
    JamoViewRenderer *m_renderer = nullptr;
};

#endif

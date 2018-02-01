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
#include "fontutils/glyph.hpp"
#include "jamomodel.hpp"

class JamoView : public QQuickFramebufferObject
{
    Q_OBJECT
    Q_PROPERTY(QString name
                READ name
                WRITE setName
                NOTIFY nameChanged)
    Q_PROPERTY(Glyph *glyph
               READ glyph
               WRITE setGlyph
               NOTIFY glyphChanged)
    Q_PROPERTY(bool editable
               READ editable
               WRITE setEditable)

public:
    JamoView();

    QString name() const;
    void setName(const QString &name);

    Glyph *glyph() const;
    void setGlyph(Glyph *glyph);

    bool editable() const;
    void setEditable(bool editable);

    QQuickFramebufferObject::Renderer *createRenderer() const override;

signals:
    void nameChanged() const;
    void glyphChanged() const;
    void pressed(int x, int y) const;
    void moved(int x, int y) const;
    void unpressed(int x, int y) const;

private:
    QString m_name = "";
    Glyph *m_glyph = nullptr;
    bool m_editable = false;
    JamoViewRenderer *m_renderer = nullptr;
};

#endif

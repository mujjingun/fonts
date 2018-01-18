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

#include "jamoview.hpp"

#include <QDebug>

JamoView::JamoView()
{
}

Glyph *JamoView::glyph() const
{
    return m_glyph;
}

void JamoView::setGlyph(Glyph* glyph)
{
    m_glyph = glyph;

    emit glyphChanged();
}

QQuickFramebufferObject::Renderer *
JamoView::createRenderer() const
{
    return new JamoViewRenderer;
}

QString JamoView::name() const
{
    return m_name;
}

void JamoView::setName(const QString &name)
{
    m_name = name;
    emit nameChanged();
}

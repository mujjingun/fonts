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

#ifndef JAMOVIEWRENDERER_HPP
#define JAMOVIEWRENDERER_HPP

#include <QObject>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QQuickWindow>
#include <QOpenGLFramebufferObject>
#include <QQuickFramebufferObject>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <memory>

#include "jamomodel.hpp"

class JamoViewRenderer : public QQuickFramebufferObject::Renderer
{
public:
    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;

    void render() override;
    void synchronize(QQuickFramebufferObject *item) override;

private:
    QOpenGLShaderProgram *m_program = nullptr;
    QString m_name = "";
    Glyph *m_glyph = nullptr;
    QQuickFramebufferObject *m_view = nullptr;

    QOpenGLVertexArrayObject m_outlineVAO;
    QOpenGLVertexArrayObject m_rectVAO;
    QOpenGLBuffer m_outlineVBO;
    QOpenGLBuffer m_rectVBO;

    QVector<QPair<int, int>> intervals;
};

#endif

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

class JamoViewRenderer : public QObject, public QQuickFramebufferObject::Renderer
{
    Q_OBJECT
public:
    JamoViewRenderer();

    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;

    void render() override;
    void synchronize(QQuickFramebufferObject *item) override;
    void pressed(int x, int y);
    void moved(int x, int y);
    void unpressed(int x, int y);

private:

    void rebuild(QOpenGLFunctions *f);
    void rebuild_outlines(QOpenGLFunctions *f);
    void rebuild_grid(QOpenGLFunctions *f);

    int find_point(int mouse_x, int mouse_y) const;

    QString m_name = "";
    Glyph *m_glyph = nullptr;
    bool m_editable = false;
    QQuickFramebufferObject *m_view = nullptr;

    QOpenGLVertexArrayObject m_outlineVAO{};
    QOpenGLBuffer m_outlineVBO{};

    QOpenGLVertexArrayObject m_gridVAO{};
    QOpenGLBuffer m_gridVBO{};

    bool m_dirty = true;

    QTransform m_DU_to_GL{};
    QTransform m_screen_to_DU{};

    QVector<QPair<QVector2D, int>> m_verts{};

    QVector<int> m_indices{};
    QVector<int> m_grid_indices{};

    int m_hover_point_idx = -1;
    int m_selected_point_idx = -1;
    QVector<int> m_point_indices{};
};

#endif


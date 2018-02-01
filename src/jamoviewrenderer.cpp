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

#include "jamoviewrenderer.hpp"
#include "jamoview.hpp"

#include <cmath>
#include <array>

JamoViewRenderer::JamoViewRenderer()
{
    m_DU_to_GL.scale(1. / 1000, 1. / 1000);
}

QOpenGLFramebufferObject *
JamoViewRenderer::createFramebufferObject(const QSize &size)
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(4);

    QTransform m_screen_to_GL;
    m_screen_to_GL.translate(-1, 1);
    m_screen_to_GL.scale(2. / size.width(), -2. / size.height());
    m_screen_to_DU = m_screen_to_GL * m_DU_to_GL.inverted();

    return new QOpenGLFramebufferObject(size, format);
}

// TODO: make em per unit a uniform
static const char vertex_shader_source[] =
R"GLSL(

attribute highp vec4 vertices;

uniform mat3 u_transform;
uniform vec3 u_color;
uniform float u_pointsize;
varying vec3 v_color;

void main() {
    v_color = u_color;
    vec3 p = vec3(vertices.x, vertices.y, 1) * u_transform;
    gl_Position = vec4(p, 1);
    gl_PointSize = u_pointsize;
}

)GLSL";

static const char geometry_shader_source[] =
R"GLSL(

#version 120
#extension GL_ARB_geometry_shader4 : enable

void main() {
    int i;
    for(i = 0; i < gl_VerticesIn; ++i) {
        gl_Position = gl_PositionIn[i];
        EmitVertex();
    }
}

)GLSL";

static const char fragment_shader_source[] =
R"GLSL(

varying vec3 v_color;

void main() {
    gl_FragColor = vec4(v_color.rgb, 1);
}

)GLSL";

void JamoViewRenderer::rebuild_outlines(QOpenGLFunctions* f)
{
    m_indices.clear();
    m_point_indices.clear();
    m_verts.clear();

    QVector<QVector2D> verts;

    verts << QVector2D(0, 0);
    if (m_glyph)
    {
        for (auto const &path : m_glyph->glyph().paths)
        {
            verts << QVector2D(path.start().x, path.start().y);
            m_verts.append({QVector2D(path.start().x, path.start().y), verts.size() - 1});
            m_point_indices << verts.size() - 1;
            int start_idx = verts.size() - 1;

            for (auto const& seg : path.segments())
            {
                QVector2D b0 = verts[verts.size() - 1];
                QVector2D b1(seg.ct1.x, seg.ct1.y);
                QVector2D b2(seg.ct2.x, seg.ct2.y);
                QVector2D b3(seg.p.x, seg.p.y);

                float dist = (b3 - b0).length();
                float dt = 10 / (dist + 1);
                for (float t = dt; t < 1.0f; t += dt) {
                    QVector2D p = std::pow(1 - t, 3) * b0;
                    p += 3 * std::pow(1 - t, 2) * t * b1;
                    p += 3 * (1 - t) * std::pow(t, 2) * b2;
                    p += std::pow(t, 3) * b3;
                    verts << p;
                    m_indices << 0 << verts.size() - 2 << verts.size() - 1;
                }

                verts << b3;
                m_verts.append({b3, verts.size() - 1});
                m_indices << 0 << verts.size() - 2 << verts.size() - 1;
                m_point_indices << verts.size() - 1;
            }
            m_indices << 0 << verts.size() - 1 << start_idx;
        }
    }

    QOpenGLVertexArrayObject::Binder vao_binder(&m_outlineVAO);
    m_outlineVBO.create();
    m_outlineVBO.bind();
    m_outlineVBO.allocate(verts.constData(), verts.count() * sizeof(GLfloat) * 2);

    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    m_outlineVBO.release();
}

void JamoViewRenderer::rebuild_grid(QOpenGLFunctions* f)
{
    m_grid_indices.clear();

    QVector<QVector4D> grid_verts;
    grid_verts << QVector3D(-1000, 0, 1) << QVector3D(1000, 0, 1);
    grid_verts << QVector3D(0, -1000, 1) << QVector3D(0, 1000, 1);
    m_grid_indices << 0 << 1 << 2 << 3;

    /*
    int width = m_glyph->glyph().width;
    verts << QVector2D(width, -1000) << QVector2D(width, 1000);
    grid_indices << 4 << 5;
    */

    QOpenGLVertexArrayObject::Binder vao_binder(&m_gridVAO);
    m_gridVBO.create();
    m_gridVBO.bind();
    m_gridVBO.allocate(grid_verts.constData(), grid_verts.count() * sizeof(GLfloat) * 4);

    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    m_gridVBO.release();
}

void JamoViewRenderer::rebuild(QOpenGLFunctions *f)
{
    // outlines
    rebuild_outlines(f);

    // gridlines
    rebuild_grid(f);

    m_dirty = false;
}

static QOpenGLShaderProgram *get_program(int *u_color, int *u_pointsize, int *u_transform)
{
    static QOpenGLShaderProgram *program = nullptr;
    static int loc_color, loc_pointsize, loc_transform;
    if (!program) {
        program = new QOpenGLShaderProgram();
        program->addCacheableShaderFromSourceCode(QOpenGLShader::Vertex, vertex_shader_source);
        //program->addCacheableShaderFromSourceCode(QOpenGLShader::Geometry, geometry_shader_source);
        program->addCacheableShaderFromSourceCode(QOpenGLShader::Fragment, fragment_shader_source);

        program->bindAttributeLocation("vertices", 0);
        program->link();

        loc_color = program->uniformLocation("u_color");
        loc_pointsize = program->uniformLocation("u_pointsize");
        loc_transform = program->uniformLocation("u_transform");
    }

    *u_color = loc_color;
    *u_pointsize = loc_pointsize;
    *u_transform = loc_transform;

    return program;
}

int JamoViewRenderer::find_point(int mouse_x, int mouse_y) const
{
    auto mouse = QVector2D(mouse_x, mouse_y);

    int point_idx = -1;
    for (auto p : m_verts)
    {
        auto q = QPointF(p.first.x(), p.first.y());
        QVector2D point(q * m_screen_to_DU.inverted());
        if (point.distanceToPoint(mouse) < 10) {
            point_idx = p.second;
        }
    }

    return point_idx;
}

void JamoViewRenderer::pressed(int x, int y)
{
    m_hover_point_idx = find_point(x, y);
}

void JamoViewRenderer::moved(int x, int y)
{
    m_hover_point_idx = find_point(x, y);
}

void JamoViewRenderer::unpressed(int x, int y)
{
    m_selected_point_idx = find_point(x, y);
}

void JamoViewRenderer::render()
{
    //if (m_editable) qDebug() << "render";
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    int loc_color, loc_pointsize, loc_transform;
    QOpenGLShaderProgram *program = get_program(&loc_color, &loc_pointsize, &loc_transform);

    if (m_dirty) rebuild(f);

    program->bind();
    program->setUniformValue(loc_transform, m_DU_to_GL);

    f->glClearColor(1, 1, 1, 1);
    f->glDisable(GL_DEPTH_TEST);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_outlineVAO.bind();
    program->setUniformValue(loc_color, QVector3D(1, 1, 1));
    f->glEnable(GL_BLEND);
    f->glBlendEquationSeparate(GL_FUNC_SUBTRACT, GL_FUNC_ADD);
    f->glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ZERO);
    f->glDrawElements(GL_TRIANGLES, m_indices.count(), GL_UNSIGNED_INT, m_indices.constData());
    m_outlineVAO.release();

    f->glBlendEquation(GL_FUNC_ADD);
    f->glBlendFunc(GL_ONE, GL_ZERO);

    m_gridVAO.bind();
    program->setUniformValue(loc_color, QVector3D(0, 0, 0));
    f->glDrawElements(GL_LINES, m_grid_indices.count(), GL_UNSIGNED_INT, m_grid_indices.constData());
    m_gridVAO.release();

    if (m_editable) {
        f->glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
        m_outlineVAO.bind();
        program->setUniformValue(loc_color, QVector3D(0.2, 1, 0.2));
        program->setUniformValue(loc_pointsize, 5.f);
        f->glDrawElements(GL_POINTS, m_point_indices.count(), GL_UNSIGNED_INT, m_point_indices.constData());

        if (m_hover_point_idx >= 0) {
            program->setUniformValue(loc_color, QVector3D(1, 0.2, 0.2));
            program->setUniformValue(loc_pointsize, 10.f);
            f->glDrawElements(GL_POINTS, 1, GL_UNSIGNED_INT, &m_hover_point_idx);
        }

        if (m_selected_point_idx >= 0) {
            program->setUniformValue(loc_color, QVector3D(1, 0.2, 1));
            program->setUniformValue(loc_pointsize, 7.f);
            f->glDrawElements(GL_POINTS, 1, GL_UNSIGNED_INT, &m_selected_point_idx);
        }
        m_outlineVAO.release();
    }

    program->release();

    m_view->window()->resetOpenGLState();
}

void JamoViewRenderer::synchronize(QQuickFramebufferObject* item)
{
    JamoView *fbitem = static_cast<JamoView *>(item);
    m_view = fbitem;
    m_name = fbitem->name();
    m_glyph = fbitem->glyph();
    m_editable = fbitem->editable();
    m_dirty = true;

    update();
}

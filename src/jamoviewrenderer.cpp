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

QOpenGLFramebufferObject *
JamoViewRenderer::createFramebufferObject(const QSize &size)
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(4);
    return new QOpenGLFramebufferObject(size, format);
}

// TODO: make em per unit a uniform
static const char vertex_shader_source[] =
R"GLSL(

attribute highp vec4 vertices;

varying vec2 v_texcoords;

void main() {
    v_texcoords = vertices.zw;
    vec2 p = vec2(vertices.x, -vertices.y) / 1000.0;
    gl_Position = vec4(p, 0, 1);
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

varying vec2 v_texcoords;

void main() {
    gl_FragColor = vec4(1, 1, 1, 1);
}

)GLSL";

void JamoViewRenderer::rebuild(QOpenGLFunctions *f)
{
    QVector<QVector2D> verts;
    indices.clear();
    indices2.clear();
    verts << QVector2D(0, 0);
    for (auto const &path : m_glyph->glyph().paths)
    {
        verts << QVector2D(path.start().x, path.start().y);
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
                indices << 0 << verts.size() - 2 << verts.size() - 1;
            }

            verts << b3;
            indices << 0 << verts.size() - 2 << verts.size() - 1;
        }
        indices << 0 << verts.size() - 1 << start_idx;
    }

    QOpenGLVertexArrayObject::Binder vao_binder(&m_outlineVAO);

    m_outlineVBO.create();
    m_outlineVBO.bind();
    m_outlineVBO.allocate(verts.constData(), verts.count() * sizeof(GLfloat) * 2);

    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    m_outlineVBO.release();

    m_dirty = false;
}

static QOpenGLShaderProgram *get_program()
{
    static QOpenGLShaderProgram *program = nullptr;
    if (!program) {
        program = new QOpenGLShaderProgram();
        program->addCacheableShaderFromSourceCode(QOpenGLShader::Vertex, vertex_shader_source);
        //program->addCacheableShaderFromSourceCode(QOpenGLShader::Geometry, geometry_shader_source);
        program->addCacheableShaderFromSourceCode(QOpenGLShader::Fragment, fragment_shader_source);

        program->bindAttributeLocation("vertices", 0);
        program->link();
    }
    return program;
}

void JamoViewRenderer::render()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    QOpenGLShaderProgram *program = get_program();

    if (m_dirty) rebuild(f);

    program->bind();

    f->glClearColor(1, 1, 1, 1);
    f->glDisable(GL_DEPTH_TEST);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    f->glEnable(GL_BLEND);
    f->glBlendEquationSeparate(GL_FUNC_SUBTRACT, GL_FUNC_ADD);
    f->glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ZERO);

    m_outlineVAO.bind();

    f->glDrawElements(GL_TRIANGLES, indices.count(), GL_UNSIGNED_INT, indices.constData());
    //f->glDrawElements(GL_TRIANGLES, indices2.count(), GL_UNSIGNED_INT, indices2.constData());

    m_outlineVAO.release();
    program->release();

    f->glBlendEquation(GL_FUNC_ADD);
    f->glBlendFunc(GL_ONE, GL_ZERO);
    m_view->window()->resetOpenGLState();
}

void JamoViewRenderer::synchronize(QQuickFramebufferObject* item)
{
    JamoView *fbitem = static_cast<JamoView *>(item);
    m_view = fbitem;
    m_name = fbitem->name();
    m_glyph = fbitem->glyph();
    m_dirty = true;
    update();
}

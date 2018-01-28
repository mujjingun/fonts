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
#include <QMatrix4x3>
#include <QMatrix4x4>

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

static const char fragment_shader_source[] =
R"GLSL(

varying vec2 v_texcoords;

void main() {
    float L, M, N;
    /*L =
    float k, l, m, n;
    float c = k * k * k - l * m * n;
    if (c < 0)*/
        gl_FragColor = vec4(1, 1, 1, 1);
    /*else
        discard;*/
}

)GLSL";

static int winding(QVector2D a, QVector2D b, QVector2D c){
    double t = a.x() * b.y() - a.y() * b.x();
    t += b.x() * c.y() - b.y() * c.x();
    t += c.x() * a.y() - c.y() * a.x();
    return t > 0? 1 : -1;
}

void JamoViewRenderer::rebuild(QOpenGLFunctions *f)
{
    QGenericMatrix<4, 4, float> const M(
        std::array<float, 16>{{
         1,  0,  0,  0,
        -3,  3,  0,  0,
         3, -6,  3,  0,
        -1,  3, -3,  1
    }}.data());

    QVector<QVector4D> verts;
    indices.clear();
    indices2.clear();
    verts << QVector4D(0, 0, 0, 0);
    for (auto const &path : m_glyph->glyph().paths)
    {
        verts << QVector4D(path.start().x, path.start().y, 0, 0);
        int start_idx = verts.size() - 1;

        for (auto const& seg : path.segments())
        {
            QVector2D b0 = verts[verts.size() - 1].toVector2D();
            QVector2D b1(seg.ct1.x, seg.ct1.y);
            QVector2D b2(seg.ct2.x, seg.ct2.y);
            QVector2D b3(seg.p.x, seg.p.y);

            QMatrix4x3 B(std::array<float, 12>{{
                b0.x(), b0.y(), 1,
                b1.x(), b1.y(), 1,
                b2.x(), b2.y(), 1,
                b3.x(), b3.y(), 1
            }}.data());

            QMatrix4x3 C = B * M;

            qDebug() << C;

            verts << QVector4D(b1.x(), b1.y(), 1, 1);
            verts << QVector4D(b2.x(), b2.y(), 0, 1);
            verts << QVector4D(b3.x(), b3.y(), 1, 0);

            int s = verts.size() - 4;
            indices << 0 << s << s + 3;
            int w012 = winding(b0, b1, b2);
            int w123 = winding(b1, b2, b3);
            int w013 = winding(b0, b1, b3);
            int w023 = winding(b0, b2, b3);
            if (w012 == w123) {
                indices2 << s << s + 1 << s + 2;
                indices2 << s + 2 << s << s + 3;
            }
            else if (w013 > 0 && w013 == w012) {
                indices2 << s << s + 1 << s + 3;
            }
            else if (w023 > 0 && w013 != w012) {
                indices2 << s << s + 2 << s + 3;
            }
        }
        indices << 0 << verts.size() - 1 << start_idx;
    }

    QOpenGLVertexArrayObject::Binder vao_binder(&m_outlineVAO);

    m_outlineVBO.create();
    m_outlineVBO.bind();
    m_outlineVBO.allocate(verts.constData(), verts.count() * sizeof(GLfloat) * 4);

    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    m_outlineVBO.release();

    m_dirty = false;
}

void JamoViewRenderer::render()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    if (!m_program) {
        m_program = new QOpenGLShaderProgram();
        m_program->addCacheableShaderFromSourceCode(QOpenGLShader::Vertex, vertex_shader_source);
        m_program->addCacheableShaderFromSourceCode(QOpenGLShader::Fragment, fragment_shader_source);

        m_program->bindAttributeLocation("vertices", 0);
        m_program->link();
    }

    if (m_dirty) rebuild(f);

    m_program->bind();

    f->glClearColor(1, 1, 1, 1);
    f->glDisable(GL_DEPTH_TEST);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    f->glEnable(GL_BLEND);
    f->glBlendEquationSeparate(GL_FUNC_SUBTRACT, GL_FUNC_ADD);
    f->glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ZERO);

    m_outlineVAO.bind();

    f->glDrawElements(GL_TRIANGLES, indices.count(), GL_UNSIGNED_INT, indices.constData());
    f->glDrawElements(GL_TRIANGLES, indices2.count(), GL_UNSIGNED_INT, indices2.constData());

    m_outlineVAO.release();
    m_program->release();

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

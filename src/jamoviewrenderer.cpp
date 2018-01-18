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

QOpenGLFramebufferObject *
JamoViewRenderer::createFramebufferObject(const QSize &size)
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(4);
    return new QOpenGLFramebufferObject(size, format);
}

static const char vertex_shader_source[] =
R"GLSL(

attribute highp vec4 vertices;

void main() {
    gl_Position = vertices;
}

)GLSL";

static const char fragment_shader_source[] =
R"GLSL(

void main() {
    gl_FragColor = vec4(0.5, 1, 1, 1);
}

)GLSL";

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

    m_program->bind();

    f->glClearColor(0, 0, 0, 1);
    f->glEnable(GL_STENCIL_TEST);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    m_outlineVAO.bind();

    f->glStencilFunc(GL_NEVER, 1, 0xFF);
    f->glStencilOp(GL_INVERT, GL_KEEP, GL_KEEP);
    f->glStencilMask(0xFF);
    for (auto const& interval : intervals) {
        f->glDrawArrays(GL_TRIANGLE_FAN, interval.first, interval.second);
    }

    m_outlineVAO.release();
    m_rectVAO.bind();
    f->glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
    f->glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    f->glStencilMask(0x00);

    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    m_rectVAO.release();
    m_program->release();

    //update();
}

void JamoViewRenderer::synchronize(QQuickFramebufferObject* item)
{
    JamoView *fbitem = static_cast<JamoView *>(item);
    m_name = fbitem->name();
    m_glyph = fbitem->glyph();

    QVector<QVector2D> values;
    intervals.clear();
    for (auto const &path : m_glyph->glyph().paths) {
        intervals << QPair<int, int>(values.count(), path.segments().size() + 1);
        values << QVector2D(path.start().x, path.start().y) / 1000.0;
        for (auto const& seg : m_glyph->glyph().paths[0].segments()) {
            auto p = seg.get<fontutils::Line>().p;
            values << QVector2D(p.x, p.y) / 1000.0;
        }
    }

    QVector<QVector2D> quad_values = {
        {-1, -1},
        {-1, 1},
        {1, -1},
        {1, 1}
    };

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    {
        QOpenGLVertexArrayObject::Binder vao_binder(&m_outlineVAO);

        m_outlineVBO.create();
        m_outlineVBO.bind();
        m_outlineVBO.allocate(values.constData(), values.count() * sizeof(GLfloat) * 2);

        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
        m_outlineVBO.release();
    }
    {
        QOpenGLVertexArrayObject::Binder vao_binder(&m_rectVAO);
        m_rectVBO.create();
        m_rectVBO.bind();
        m_rectVBO.allocate(quad_values.constData(), quad_values.count() * sizeof(GLfloat) * 2);

        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
        m_rectVBO.release();
    }
}

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

// TODO: make em per unit a uniform
static const char vertex_shader_source[] =
R"GLSL(

attribute highp vec4 vertices;

void main() {
    vec2 p = vec2(vertices.x, -vertices.y) / 1000.0;
    gl_Position = vec4(p, 0, 1);
}

)GLSL";

static const char fragment_shader_source[] =
R"GLSL(

void main() {
    gl_FragColor = vec4(1, 1, 1, 1);
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

    f->glClearColor(1, 1, 1, 1);
    f->glDisable(GL_DEPTH_TEST);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    f->glBlendEquationSeparate(GL_FUNC_SUBTRACT, GL_FUNC_ADD);
    f->glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ZERO);

    m_outlineVAO.bind();

    for (auto const& interval : intervals) {
        f->glDrawArrays(GL_TRIANGLE_FAN, interval.first, interval.second);
    }

    m_outlineVAO.release();
    m_program->release();

    f->glBlendEquation(GL_FUNC_ADD);
    f->glBlendFunc(GL_ONE, GL_ZERO);
    f->glEnable(GL_DEPTH_TEST);
    //m_view->window()->resetOpenGLState();
}

void JamoViewRenderer::synchronize(QQuickFramebufferObject* item)
{
    JamoView *fbitem = static_cast<JamoView *>(item);
    m_view = fbitem;
    m_name = fbitem->name();
    m_glyph = fbitem->glyph();

    QVector<QVector2D> values;
    intervals.clear();
    for (auto const &path : m_glyph->glyph().paths) {
        intervals << QPair<int, int>(values.count(), path.segments().size() + 1);
        values << QVector2D(path.start().x, path.start().y);
        for (auto const& seg : path.segments()) {
            fontutils::Line const *line;
            fontutils::CubicBezier const *bezier;
            if ((line = seg.get<fontutils::Line>())) {
                values << QVector2D(line->p.x, line->p.y);
            }
            else if ((bezier = seg.get<fontutils::CubicBezier>())) {
                values << QVector2D(bezier->p.x, bezier->p.y);
            }
        }
    }

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

    // Initialize Quad
    {
        QVector<QVector2D> quad_values = {
            {-1, -1},
            {-1, 1},
            {1, -1},
            {1, -1},
            {-1, 1},
            {1, 1},
            {1, -1},
            {-1, 1},
            {1, 1},
        };

        QOpenGLVertexArrayObject::Binder vao_binder(&m_rectVAO);
        m_rectVBO.create();
        m_rectVBO.bind();
        m_rectVBO.allocate(quad_values.constData(), quad_values.count() * sizeof(GLfloat) * 2);

        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
        m_rectVBO.release();
    }
}

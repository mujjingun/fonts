#include "formmodel.hpp"

GlyphObject::GlyphObject(QObject* parent)
    : QObject(parent)
{}

FormModel::FormModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_defaultglyph(new GlyphObject(parent))
{}

void FormModel::setDefaultGlyphObject(GlyphObject *glyph)
{
    m_defaultglyph = glyph;
    emit defaultGlyphObjectChanged();
}

GlyphObject *FormModel::defaultGlyphObject()
{
    return m_defaultglyph;
}

void FormModel::setDefaultGlyph(const geul::Glyph& glyph)
{
    m_defaultglyph->glyph = glyph;
    emit defaultGlyphObjectChanged();
}

QVariant FormModel::data(const QModelIndex& index, int role) const
{
    if (role == NameRole)
        return "test";
    else if (role == GlyphRole)
        return QVariant::fromValue(m_defaultglyph);

    return QVariant();
}

Qt::ItemFlags FormModel::flags(const QModelIndex& index) const
{
    return QAbstractItemModel::flags(index);
}

// Returns the number of rows under the given parent. When the parent is valid
// it means that rowCount is returning the number of children of parent.
int FormModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return 3;
}

QHash<int, QByteArray> FormModel::roleNames() const
{
    return { { NameRole, "name" }, { GlyphRole, "glyph" } };
}

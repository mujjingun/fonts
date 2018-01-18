#include "jamomodel.hpp"

Glyph::Glyph(fontutils::Glyph g, QObject* parent)
    : QObject(parent), m_glyph(g)
{
}

fontutils::Glyph Glyph::glyph()
{
    return m_glyph;
}

JamoModel::JamoModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void JamoModel::addJamo(const Jamo& jamo)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_jamos << jamo;
    endInsertRows();
}

QVariant JamoModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= m_jamos.count())
        return QVariant();

    const Jamo &jamo = m_jamos[index.row()];
    if (role == NameRole)
        return jamo.name;
    if (role == GlyphRole)
        return QVariant::fromValue(jamo.glyph);

    return QVariant();
}

int JamoModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_jamos.count();
}

QHash<int, QByteArray> JamoModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[GlyphRole] = "glyph";
    return roles;
}

#include "jamomodel.hpp"

Glyph::Glyph(fontutils::Glyph g, QObject* parent)
    : QObject(parent), m_glyph(g)
{
}

fontutils::Glyph Glyph::glyph()
{
    return m_glyph;
}

JamoModel::JamoModel(QList<JamoName> names, QObject *parent)
    : QAbstractListModel(parent)
{
    for (auto const& name : names) {
        QString nm = QChar(static_cast<char32_t>(name));
        m_jamos[name] = Jamo{nm, new Glyph({}, this)};
    }
}

void JamoModel::setJamo(JamoName name, Jamo jamo)
{
    m_jamos[name] = jamo;
    auto idx = index(std::distance(m_jamos.begin(), m_jamos.find(name)), 0);
    emit dataChanged(idx, idx);
}

QVariant JamoModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= m_jamos.count())
        return QVariant();

    const Jamo &jamo = *(m_jamos.begin() + index.row());
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

Qt::ItemFlags JamoModel::flags(const QModelIndex& index) const
{
    return QAbstractItemModel::flags(index);// | Qt::ItemIsEditable;
}

#include "jamomodel.hpp"

JamoModel::JamoModel(QList<JamoName> names, QObject* parent)
    : QAbstractListModel(parent)
{
    for (auto const& name : names)
    {
        m_jamos[name] = std::make_unique<FormModel>(parent);
    }
}

void JamoModel::setGlyph(JamoName name, geul::Glyph const& glyph)
{
    m_jamos[name]->setDefaultGlyph(glyph);
    auto idx = index(std::distance(m_jamos.begin(), m_jamos.find(name)), 0);
    emit dataChanged(idx, idx);
}

QVariant JamoModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= int(m_jamos.size()))
        return QVariant();

    const auto it = std::next(m_jamos.begin(), index.row());
    if (role == NameRole)
        return QChar(char32_t(it->first));
    if (role == FormModelRole)
        return QVariant::fromValue(it->second.get());

    return QVariant();
}

int JamoModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_jamos.size();
}

QHash<int, QByteArray> JamoModel::roleNames() const
{
    return { { NameRole, "name" }, { FormModelRole, "formModel" } };
}

Qt::ItemFlags JamoModel::flags(const QModelIndex& index) const
{
    return QAbstractItemModel::flags(index); // | Qt::ItemIsEditable;
}

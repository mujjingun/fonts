#include "jamomodel.hpp"

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
    if (role == StartRole)
        return jamo.start;
    if (role == PathRole)
        return jamo.path;

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
    roles[StartRole] = "start";
    roles[PathRole] = "path";
    return roles;
}

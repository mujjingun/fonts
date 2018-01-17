#ifndef JAMOMODEL_HPP
#define JAMOMODEL_HPP

#include <QObject>
#include <QString>
#include <QList>
#include <QAbstractListModel>
#include <QPoint>

struct Jamo
{
    void addPath(QPoint start, QList<QVariantMap> points);

    QString name;
    QVariantList paths = {};
};

class JamoModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum JamoModelRoles {
        NameRole = Qt::UserRole + 1,
        PathsRole
    };

    JamoModel(QObject *parent = nullptr);

    void addJamo(const Jamo &jamo);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    QList<Jamo> m_jamos;
};

#endif

#ifndef CUSTOMTABLEDELEGATE_H
#define CUSTOMTABLEDELEGATE_H

#include <QStyledItemDelegate>

class CustomTableDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit CustomTableDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
};

#endif // CUSTOMTABLEDELEGATE_H

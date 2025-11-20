#ifndef UTILS_H
#define UTILS_H

#include <QPainter>
#include <QPainterPath>
#include <QStyledItemDelegate>

class CustomTableDelegate : public QStyledItemDelegate {
  public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
};

#endif // UTILS_H

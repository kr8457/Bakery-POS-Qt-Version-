#include "CustomTableDelegate.h"
#include <QPainter>

CustomTableDelegate::CustomTableDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {}

void CustomTableDelegate::paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const {
    QStyledItemDelegate::paint(painter, option, index);
}

QSize CustomTableDelegate::sizeHint(const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const {
    return QStyledItemDelegate::sizeHint(option, index);
}
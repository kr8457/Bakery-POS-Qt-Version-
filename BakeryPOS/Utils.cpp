#include "Utils.h"

void CustomTableDelegate::paint(QPainter                   *painter,
                                const QStyleOptionViewItem &option,
                                const QModelIndex          &index) const {
    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    QStyledItemDelegate::paint(painter, opt, index);

    int row      = index.row();
    int col      = index.column();
    int rowCount = index.model()->rowCount();
    int colCount = index.model()->columnCount();

    painter->save();

    // Draw bold line below the header (above first row)
    if (row == 0) {
        QPen headerPen(QColor("#1F1F1F"), 2); // bold/dark line
        painter->setPen(headerPen);
        painter->drawLine(opt.rect.topLeft(), opt.rect.topRight());
    }

    // Draw row separator under each cell (except last row if you want)
    if (row < rowCount - 1) {
        QPen rowPen(QColor("#ccc"), 1); // normal row separator
        painter->setPen(rowPen);
        painter->drawLine(opt.rect.bottomLeft(), opt.rect.bottomRight());
    }

    // Don't draw vertical lines on left/right edges
    if (col > 0 && col < colCount - 1) {
        // Optional: you could draw vertical lines here if you wanted.
        // We're skipping them to remove column separators.
    }

    painter->restore();
}

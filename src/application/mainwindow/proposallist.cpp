// albert - a simple application launcher for linux
// Copyright (C) 2014-2015 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <QKeyEvent>
#include <QPainter>
#include "proposallist.h"
#include "roles.hpp"

/** ***************************************************************************/
class ProposalList::ItemDelegate final : public QStyledItemDelegate
{
public:
    ItemDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent), drawIcon(true) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const override;

    bool drawIcon;
};



/** ***************************************************************************/
ProposalList::ProposalList(QWidget *parent) : ResizingList(parent) {
    setItemDelegate(delegate_ = new ItemDelegate(this));
}



/** ***************************************************************************/
bool ProposalList::displayIcons() const {
    return delegate_->drawIcon;
}



/** ***************************************************************************/
void ProposalList::setDisplayIcons(bool value) {
    delegate_->drawIcon = value;
    update();
}



/** ***************************************************************************/
bool ProposalList::eventFilter(QObject*, QEvent *event) {

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        switch (keyEvent->key()) {

        // Display different subtexts according to the KeyboardModifiers
        case Qt::Key_Control:
        case Qt::Key_Shift:
        case Qt::Key_Alt:
        case Qt::Key_Meta:
            if ( currentIndex().isValid() )
                update(currentIndex());
            return false;

        // Navigation
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
        // Activation
        case Qt::Key_Enter:
        case Qt::Key_Return:
            keyPressEvent(keyEvent);
            return true;
        }
    }

    if (event->type() == QEvent::KeyRelease) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        switch (keyEvent->key()) {

        // Display different subtexts according to the KeyboardModifiers
        case Qt::Key_Control:
        case Qt::Key_Shift:
        case Qt::Key_Alt:
        case Qt::Key_Meta:
            if ( currentIndex().isValid() )
                update(currentIndex());
            return false;
        }
    }
    return false;
}



/** ***************************************************************************/
void ProposalList::ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const {

    painter->save();

    QStyleOptionViewItemV4 option = options;
    initStyleOption(&option, index);

    /*
     * fm(x) := fontmetrics of x
     * DR := DisplayRole
     * TR := ToolTipRole
     *  +---------------------+----------------------------------------+
     *  |                     |                                        |
     *  |   +-------------+   |                                        |
     *  |   |             |   |                                        |
     *  |   |             |   |a*fm(DR)/(fm(DR)+fm(TR))    DisplayRole |
     * a|   |     icon    |   |                                        |
     *  |   |             |   |                                        |
     *  |   |             |   +----------------------------------------+
     *  |   |             |   |                                        |
     *  |   +-------------+   |a*fm(TR)/(fm(DR)+fm(TR))  ToolTipRole+x |
     *  |                     |                                        |
     * +---------------------------------------------------------------+
     */


    // Draw selection
    option.widget->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, option.widget);

    // Draw icon
    if ( drawIcon ){
        QRect iconRect = QRect(
                    QPoint((option.rect.height() - option.decorationSize.width())/2 + option.rect.x(),
                           (option.rect.height() - option.decorationSize.height())/2 + option.rect.y()),
                    option.decorationSize);
        painter->drawPixmap(iconRect, QPixmap(index.data(Roles::IconPath).value<QString>()).scaled(option.decorationSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    // Calculate text rects
    QFont font1 = option.font;
    QFont font2 = option.font;
    font2.setPixelSize(18);
    QFontMetrics fontMetrics1 = QFontMetrics(font1);
    QFontMetrics fontMetrics2 = QFontMetrics(font2);
    QRect contentRect = option.rect;
    contentRect.setLeft(drawIcon ? option.rect.height() : 0);
    contentRect.setTop(option.rect.y()+option.rect.height()/2-(fontMetrics1.height()+fontMetrics2.height())/2);
    contentRect.setBottom(option.rect.y()+option.rect.height()/2+(fontMetrics1.height()+fontMetrics2.height())/2);
    QRect textRect = contentRect.adjusted(0,-2,0,-fontMetrics2.height()-2);
    QRect subTextRect = contentRect.adjusted(0,fontMetrics1.height()-2,0,-2);

    //    // Test
    //    painter->fillRect(iconRect, Qt::magenta);
    //    painter->fillRect(contentRect, Qt::red);
    //    painter->fillRect(textRect, Qt::blue);
    //    painter->fillRect(subTextRect, Qt::yellow);


    // Draw display role
    painter->setFont(font1);
    QString text = fontMetrics1.elidedText(index.data(Roles::Text).toString(), option.textElideMode, textRect.width());
    option.widget->style()->drawItemText(painter, textRect, option.displayAlignment, option.palette, option.state & QStyle::State_Enabled, text, QPalette::WindowText);
    //    painter->drawText(textRect, Qt::AlignTop|Qt::AlignLeft, text);

    // Draw tooltip role
    painter->setFont(font2);
    text = fontMetrics2.elidedText(index.data(Roles::SubText).toString(), option.textElideMode, subTextRect.width());
    painter->drawText(subTextRect   , Qt::AlignBottom|Qt::AlignLeft, text);

    painter->restore();
}

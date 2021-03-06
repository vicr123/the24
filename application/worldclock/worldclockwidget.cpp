/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2020 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#include "worldclockwidget.h"
#include "ui_worldclockwidget.h"

#include <QTimer>
#include <QMenu>
#include <QContextMenuEvent>
//#include <QGraphicsDropShadowEffect>

struct WorldClockWidgetPrivate {
    QTimeZone tz;
    QTimer* ticker;

    bool removable = true;
};

WorldClockWidget::WorldClockWidget(QTimeZone tz, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::WorldClockWidget) {
    ui->setupUi(this);
    d = new WorldClockWidgetPrivate();
    d->tz = tz;

//    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
//    shadow->setBlurRadius(20);
//    shadow->setOffset(0, 0);
//    ui->clockWidget->setGraphicsEffect(shadow);

    d->ticker = new QTimer(this);
    d->ticker->setInterval(50);
    connect(d->ticker, &QTimer::timeout, this, &WorldClockWidget::updateClock);
    d->ticker->start();
    updateClock();
}

WorldClockWidget::~WorldClockWidget() {
    delete d;
    delete ui;
}

void WorldClockWidget::setRemovable(bool removable) {
    d->removable = removable;
}

void WorldClockWidget::contextMenuEvent(QContextMenuEvent* event) {
    if (!d->removable) return;

    QMenu* menu = new QMenu();
    menu->addSection(tr("For this clock"));
    menu->addAction(QIcon::fromTheme("list-remove"), tr("Remove"), this, &WorldClockWidget::remove);

    connect(menu, &QMenu::aboutToHide, menu, &QMenu::deleteLater);
    menu->popup(event->globalPos());
}

void WorldClockWidget::updateClock() {
    QDateTime date = QDateTime::currentDateTimeUtc();
    int utcOffset = d->tz.offsetFromUtc(date);
    date = date.addSecs(d->tz.offsetFromUtc(date));
    ui->timeLabel->setText(QLocale().toString(date.time(), QLocale::ShortFormat));
    ui->clockWidget->setTime(date.time());

    int dayDifference = QDateTime::currentDateTime().daysTo(date);
    if (dayDifference == 0) {
        ui->dayDifferenceLabel->setVisible(false);
    } else if (dayDifference < 0) {
        ui->dayDifferenceLabel->setText(QStringLiteral("-%1").arg(dayDifference * -1));
        ui->dayDifferenceLabel->setVisible(true);
    } else {
        ui->dayDifferenceLabel->setText(QStringLiteral("+%1").arg(dayDifference));
        ui->dayDifferenceLabel->setVisible(true);
    }

    QStringList labelParts;

    int differenceFromLocal = utcOffset - QTimeZone::systemTimeZone().offsetFromUtc(QDateTime::currentDateTimeUtc());
    if (differenceFromLocal == 0) {
        labelParts.append(tr("Current Timezone"));
    } else {
        QTime offset = QTime(0, 0).addSecs(qAbs(differenceFromLocal));
        if (offset.minute() == 0) {
            if (differenceFromLocal < 0) {
                labelParts.append(tr("%n hours behind", nullptr, qAbs(differenceFromLocal / 3600)));
            } else {
                labelParts.append(tr("%n hours ahead", nullptr, qAbs(differenceFromLocal / 3600)));
            }
        } else {
            if (differenceFromLocal < 0) {
                labelParts.append(tr("%1 behind").arg(offset.toString("hh:mm")));
            } else {
                labelParts.append(tr("%1 ahead").arg(offset.toString("hh:mm")));
            }
        }
    }

    labelParts.append(d->tz.displayName(QDateTime::currentDateTime(), QTimeZone::LongName));
    ui->timezoneLabel->setText(labelParts.join(" · "));
}

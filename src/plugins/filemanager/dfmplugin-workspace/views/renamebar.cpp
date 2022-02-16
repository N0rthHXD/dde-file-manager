/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "renamebar.h"
#include "private/renamebar_p.h"

#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QStackedWidget>

DPWORKSPACE_USE_NAMESPACE

RenameBar::RenameBar(QWidget *parent)
    : QFrame(parent), d(new RenameBarPrivate(this))
{
    initConnect();
}

void RenameBar::reset() noexcept
{
    ///replace
    QLineEdit *lineEdit { std::get<1>(d->replaceOperatorItems) };
    lineEdit->clear();
    lineEdit = std::get<3>(d->replaceOperatorItems);
    lineEdit->clear();

    ///add
    lineEdit = std::get<1>(d->addOperatorItems);
    lineEdit->clear();

    ///custom
    lineEdit = std::get<1>(d->customOPeratorItems);
    lineEdit->clear();
    lineEdit = std::get<3>(d->customOPeratorItems);
    lineEdit->setText(QString { "1" });

    d->flag = RenameBarPrivate::AddTextFlags::kBefore;
    d->currentPattern = RenameBarPrivate::RenamePattern::kReplace;
    d->renameButtonStates = std::array<bool, 3> { false, false, false };

    d->comboBox->setCurrentIndex(0);
    d->stackWidget->setCurrentIndex(0);
    std::get<3>(d->addOperatorItems)->setCurrentIndex(0);
}

void RenameBar::storeUrlList(const QList<QUrl> &list) noexcept
{
    Q_D(RenameBar);

    d->urlList = list;
}

void RenameBar::onVisibleChanged(bool value) noexcept
{
    Q_D(RenameBar);

    using RenamePattern = RenameBarPrivate::RenamePattern;
    if (value) {

        switch (d->currentPattern) {
        case RenamePattern::kReplace: {
            QLineEdit *lineEdit { std::get<1>(d->replaceOperatorItems) };
            lineEdit->setFocus();
            break;
        }
        case RenamePattern::kAdd: {
            QLineEdit *lineEdit { std::get<1>(d->addOperatorItems) };
            lineEdit->setFocus();
            break;
        }
        case RenamePattern::kCustom: {
            QLineEdit *lineEdit { std::get<1>(d->customOPeratorItems) };
            lineEdit->setFocus();
            break;
        }
        }
    } else {
        //还原焦点
        if (parentWidget()) {
            parentWidget()->setFocus();
        }
    }
}

void RenameBar::onRenamePatternChanged(const int &index) noexcept
{
    Q_D(RenameBar);

    d->currentPattern = static_cast<RenameBarPrivate::RenamePattern>(index);

    bool state { d->renameButtonStates[static_cast<std::size_t>(index)] };   //###: we get the value of state of button in current mode.
    d->stackWidget->setCurrentIndex(index);
    std::get<1>(d->buttonsArea)->setEnabled(state);

    ///###: here, call a slot, this function will set focus of QLineEdit in current mode.
    this->onVisibleChanged(true);
}

void RenameBar::onReplaceOperatorFileNameChanged(const QString &text) noexcept
{
    Q_D(RenameBar);

    QLineEdit *lineEdit { std::get<1>(d->replaceOperatorItems) };
    d->updateLineEditText(lineEdit);

    if (text.isEmpty()) {
        d->renameButtonStates[0] = false;   //###: record the states of rename button

        d->setRenameBtnStatus(false);
        return;
    }

    d->renameButtonStates[0] = true;   //###: record the states of rename button

    d->setRenameBtnStatus(true);
}

void RenameBar::onReplaceOperatorDestNameChanged(const QString &textChanged) noexcept
{
    Q_UNUSED(textChanged)
    Q_D(RenameBar);

    QLineEdit *lineEdit { std::get<3>(d->replaceOperatorItems) };
    d->updateLineEditText(lineEdit);
}

void RenameBar::onAddOperatorAddedContentChanged(const QString &text) noexcept
{
    Q_D(RenameBar);

    QLineEdit *lineEdit { std::get<1>(d->addOperatorItems) };
    d->updateLineEditText(lineEdit);

    if (text.isEmpty()) {
        d->renameButtonStates[1] = false;

        d->setRenameBtnStatus(false);
        return;
    }

    d->renameButtonStates[1] = true;

    d->setRenameBtnStatus(true);
}

void RenameBar::onAddTextPatternChanged(const int &index) noexcept
{
    Q_D(RenameBar);

    using AddTextFlags = RenameBarPrivate::AddTextFlags;

    d->flag = (index == 0 ? AddTextFlags::kBefore : AddTextFlags::kAfter);

    ///###: here, call a slot, this function will set focus of QLineEdit in current mode.
    this->onVisibleChanged(true);
}

void RenameBar::onCustomOperatorFileNameChanged() noexcept
{

    Q_D(RenameBar);

    QLineEdit *lineEditForFileName { std::get<1>(d->customOPeratorItems) };

    d->updateLineEditText(lineEditForFileName);

    if (lineEditForFileName->text().isEmpty()) {   //###: must be input filename.
        d->renameButtonStates[2] = false;
        d->setRenameBtnStatus(false);

    } else {

        QLineEdit *lineEditForSNNumber { std::get<3>(d->customOPeratorItems) };

        if (lineEditForSNNumber->text().isEmpty()) {
            d->renameButtonStates[2] = false;
            d->setRenameBtnStatus(false);

        } else {
            d->renameButtonStates[2] = true;
            d->setRenameBtnStatus(true);
        }
    }
}

void RenameBar::onCustomOperatorSNNumberChanged()
{
    Q_D(RenameBar);

    QLineEdit *lineEditForSNNumber { std::get<3>(d->customOPeratorItems) };
    if (lineEditForSNNumber->text().isEmpty()) {   //###: must be input filename.
        d->renameButtonStates[2] = false;
        d->setRenameBtnStatus(false);

    } else {

        QLineEdit *lineEditForFileName { std::get<3>(d->customOPeratorItems) };

        if (lineEditForFileName->text().isEmpty()) {
            d->renameButtonStates[2] = false;
            d->setRenameBtnStatus(false);

        } else {
            d->renameButtonStates[2] = true;
            d->setRenameBtnStatus(true);
        }

        ///###: renew from exception.
        std::string content { lineEditForSNNumber->text().toStdString() };
        try {
            Q_UNUSED(std::stoull(content));
        } catch (const std::out_of_range &err) {
            (void)err;
            lineEditForSNNumber->setText(QString { "1" });

        } catch (...) {
            lineEditForSNNumber->setText(QString { "1" });
        }
    }
}

void RenameBar::eventDispatcher()
{
    Q_D(RenameBar);

    bool value { false };

    using RenamePattern = RenameBarPrivate::RenamePattern;

    if (d->currentPattern == RenamePattern::kReplace) {
        QString forFindingStr { std::get<1>(d->replaceOperatorItems)->text() };
        QString forReplaceStr { std::get<3>(d->replaceOperatorItems)->text() };

        QPair<QString, QString> pair { forFindingStr, forReplaceStr };

        // Todo(yanghao): multi files replace name

    } else if (d->currentPattern == RenamePattern::kAdd) {
        QString forAddingStr { std::get<1>(d->addOperatorItems)->text() };

        // Todo(yanghao): multi files add name
        //        QPair<QString, DFileService::AddTextFlags> pair{ forAddingStr, d->m_flag };

        //        value = DFileService::instance()->multiFilesAddStrToName(d->m_urlList, pair);

    } else if (d->currentPattern == RenamePattern::kCustom) {
        QString forCustomStr { std::get<1>(d->customOPeratorItems)->text() };
        QString numberStr { std::get<3>(d->customOPeratorItems)->text() };

        QPair<QString, QString> pair { forCustomStr, numberStr };

        // Todo(yanghao): multi files custom name
        //        value = DFileService::instance()->multiFilesCustomName(d->m_urlList, pair);
    }

    if (value) {
        if (QWidget *const parent = dynamic_cast<QWidget *>(this->parent())) {
            // Todo(yanghao)???
            //            quint64 windowId { WindowManager::getWindowId(parent) };
            //            AppController::multiSelectionFilesCache.second = windowId;
        }
    }

    setVisible(false);
    reset();
}

void RenameBar::hideRenameBar()
{
    setVisible(false);
    reset();
}

void RenameBar::initConnect()
{
    Q_D(RenameBar);

    using funcType = void (QComboBox::*)(int index);

    QObject::connect(d->comboBox, static_cast<funcType>(&QComboBox::activated), this, &RenameBar::onRenamePatternChanged);

    QObject::connect(std::get<0>(d->buttonsArea), &QPushButton::clicked, this, &RenameBar::clickCancelButton);
    QObject::connect(std::get<1>(d->replaceOperatorItems), &QLineEdit::textChanged, this, &RenameBar::onReplaceOperatorFileNameChanged);
    QObject::connect(std::get<3>(d->replaceOperatorItems), &QLineEdit::textChanged, this, &RenameBar::onReplaceOperatorDestNameChanged);
    QObject::connect(std::get<1>(d->addOperatorItems), &QLineEdit::textChanged, this, &RenameBar::onAddOperatorAddedContentChanged);

    QObject::connect(std::get<1>(d->buttonsArea), &QPushButton::clicked, this, &RenameBar::eventDispatcher);
    QObject::connect(std::get<3>(d->addOperatorItems), static_cast<funcType>(&QComboBox::currentIndexChanged), this, &RenameBar::onAddTextPatternChanged);

    QObject::connect(std::get<1>(d->customOPeratorItems), &QLineEdit::textChanged, this, &RenameBar::onCustomOperatorFileNameChanged);
    QObject::connect(std::get<3>(d->customOPeratorItems), &QLineEdit::textChanged, this, &RenameBar::onCustomOperatorSNNumberChanged);

    QObject::connect(this, &RenameBar::visibleChanged, this, &RenameBar::onVisibleChanged);
    QObject::connect(this, &RenameBar::clickRenameButton, this, &RenameBar::eventDispatcher);
    QObject::connect(this, &RenameBar::clickCancelButton, this, &RenameBar::hideRenameBar);
}

/*
* Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
*
* Author:     liuyanga <liuyanga@uniontech.com>
*
* Maintainer: liuyanga <liuyanga@uniontech.com>
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
#include "richtextedit.h"
#include "webrichtexteditor.h"

#include <QVBoxLayout>

RichTextEdit::RichTextEdit(QWidget *parent)
    : QWidget(parent)
{
    ;
}

void RichTextEdit::initData(VNoteItem *data, const QString &reg, bool fouse)
{
    if (m_webRichTextEditer) {
        return m_webRichTextEditer->initData(data, reg, fouse);
    }
}

void RichTextEdit::insertVoiceItem(const QString &voicePath, qint64 voiceSize)
{
    if (m_webRichTextEditer) {
        return m_webRichTextEditer->insertVoiceItem(voicePath, voiceSize);
    }
}

void RichTextEdit::updateNote()
{
    if (m_webRichTextEditer) {
        return m_webRichTextEditer->updateNote();
    }
}

void RichTextEdit::initWebView()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_webRichTextEditer = new WebRichTextEditor(this);
    connect(m_webRichTextEditer, SIGNAL(asrStart(const QVariant &)),
            this, SIGNAL(asrStart(const QVariant &)));
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_webRichTextEditer);
}

bool RichTextEdit::findText(const QString &searchKey)
{
    if (m_webRichTextEditer) {
        return m_webRichTextEditer->findText(searchKey);
    }
    return false;
}

void RichTextEdit::unboundCurrentNoteData()
{
    if (m_webRichTextEditer) {
        return m_webRichTextEditer->unboundCurrentNoteData();
    }
}
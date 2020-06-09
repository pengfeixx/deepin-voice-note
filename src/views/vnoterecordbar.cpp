/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
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

#include "vnoterecordbar.h"
#include "globaldef.h"
#include "widgets/vnoterecordwidget.h"
#include "widgets/vnoteiconbutton.h"
#include "widgets/vnoteplaywidget.h"
#include "common/vnoteitem.h"
#include "common/vnoteaudiodevicewatcher.h"
#include "dialog/vnotemessagedialog.h"

#include <QVBoxLayout>

#include <DApplication>
#include <DLog>

VNoteRecordBar::VNoteRecordBar(QWidget *parent)
    : QWidget(parent)
{
    initUI();
    initConnections();
}

void VNoteRecordBar::initUI()
{
    m_mainLayout = new QStackedLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_recordPanel = new VNoteRecordWidget(this);
    m_recordPanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_mainLayout->addWidget(m_recordPanel);

    m_recordBtnHover = new DWidget(this);
    m_recordBtn = new VNoteIconButton(m_recordBtnHover
                                      , "audio_normal.svg"
                                      , "audio_hover.svg"
                                      , "audio_press.svg");
    m_recordBtn->SetDisableIcon("audio_disabled.svg");
    m_recordBtn->setFlat(true);
    m_recordBtn->setIconSize(QSize(REC_BTN_W, REC_BTN_H));
    m_recordBtn->setFixedSize(QSize(REC_BTN_W, REC_BTN_H));
    QGridLayout *recordBtnHoverLayout = new QGridLayout;
    recordBtnHoverLayout->addWidget(m_recordBtn,0,1);
    recordBtnHoverLayout->setColumnStretch(0,1);
    recordBtnHoverLayout->setColumnStretch(1,0);
    recordBtnHoverLayout->setColumnStretch(2,1);
    m_recordBtnHover->setLayout(recordBtnHoverLayout);
    m_mainLayout->addWidget(m_recordBtnHover);

    m_playPanel = new VNotePlayWidget(this);
    m_mainLayout->addWidget(m_playPanel);

    m_mainLayout->setCurrentWidget(m_recordBtnHover);

    //Init device exception message
    m_pDeviceExceptionMsg = new DFloatingMessage(
                DFloatingMessage::ResidentType,
                reinterpret_cast<QWidget*>(parent()));

    m_pDeviceExceptionMsg->setVisible(false);

    QString iconPath = STAND_ICON_PAHT;
    iconPath.append("warning.svg");
    m_pDeviceExceptionMsg->setIcon(QIcon(iconPath));
    m_pDeviceExceptionMsg->setMessage(
                DApplication::translate(
                    "VNoteRecordBar",
                    "Your audio recording device does not work.")
                );

    //Default unavailable
    OnMicrophoneAvailableChanged(false);
}

void VNoteRecordBar::initConnections()
{
    //Install filter to filte MousePress
    //event.
    installEventFilter(this);
    m_recordBtn->installEventFilter(this);

    connect(m_recordBtn, &VNoteIconButton::clicked, this, &VNoteRecordBar::onStartRecord);
    connect(m_recordPanel, SIGNAL(sigFinshRecord(const QString &,qint64)),
            this, SLOT(onFinshRecord(const QString &,qint64)));
    connect(m_playPanel, &VNotePlayWidget::sigWidgetClose,
            this, &VNoteRecordBar::onClosePlayWidget);
    connect(m_playPanel, SIGNAL(sigPlayVoice(VNVoiceBlock *)),
            this, SIGNAL(sigPlayVoice(VNVoiceBlock *)));
    connect(m_playPanel, SIGNAL(sigPauseVoice(VNVoiceBlock *)),
            this, SIGNAL(sigPauseVoice(VNVoiceBlock *)));
}

bool VNoteRecordBar::eventFilter(QObject *o, QEvent *e)
{
    Q_UNUSED(o);
    //Let NoteItem lost focus when click
    //outside of Note

    if (e->type() == QEvent::MouseButtonPress) {
        setFocus(Qt::MouseFocusReason);
    }

    return false;
}

void VNoteRecordBar::startRecord()
{
    m_mainLayout->setCurrentWidget(m_recordPanel);

    m_recordPanel->startRecord();
    emit sigStartRecord(m_recordPanel->getRecordPath());
}

void VNoteRecordBar::onStartRecord()
{
    if(this->isVisible() && m_mainLayout->currentWidget() == m_recordBtnHover && m_recordBtn->isEnabled() ){
        if (VNoteAudioDeviceWatcher::VolumeTooLow == m_microphoneState) {
            VNoteMessageDialog volumeLowDialog(VNoteMessageDialog::VolumeTooLow);

            connect(&volumeLowDialog, &VNoteMessageDialog::accepted, this, [this]() {
                //User confirmed record when volume too low
                //start recording anyway.
                startRecord();
            });

            volumeLowDialog.exec();
        } else {
            //Volume normal
            startRecord();
        }
    }
}

void VNoteRecordBar::onFinshRecord(const QString &voicePath,qint64 voiceSize)
{
    m_mainLayout->setCurrentWidget(m_recordBtnHover);
    emit sigFinshRecord(voicePath,voiceSize);
}

void VNoteRecordBar::OnMicrophoneAvailableChanged(int availableState)
{
    qInfo() << __FUNCTION__ << " MicrophoneState:" << availableState;

    m_microphoneState = availableState;

    if (VNoteAudioDeviceWatcher::NotAvailable == m_microphoneState) {
        m_recordBtn->setBtnDisabled(true);
        m_recordBtn->setToolTip(
                    DApplication::translate(
                        "VNoteRecordBar",
                        "No recording device detected")
                    );
        //If device don't available during recording,
        //stop recording,and give device exception message.
        if (m_mainLayout->currentWidget() == m_recordPanel) {
            cancelRecord();

            if (!m_pDeviceExceptionMsg->isVisible()) {
                m_pDeviceExceptionMsg->show();
                m_pDeviceExceptionMsg->adjustSize();
            }
        }
    } else {
        //When device available,should hide the exception
        //message
        if (m_pDeviceExceptionMsg->isVisible()) {
            m_pDeviceExceptionMsg->close();
        }

        m_recordBtn->setBtnDisabled(false);
        m_recordBtn->setToolTip("");

        //If volume is changed when recording,give message
        if (VNoteAudioDeviceWatcher::VolumeTooLow == m_microphoneState
                && m_recordPanel == m_mainLayout->currentWidget()) {
            VNoteMessageDialog volumeLowDialog(VNoteMessageDialog::VolumeTooLow);

            connect(&volumeLowDialog, &VNoteMessageDialog::rejected, this, [this]() {
                //Canel record when volume too low
                cancelRecord();
            });

            volumeLowDialog.exec();
        }
    }
}

void VNoteRecordBar::cancelRecord()
{
    m_recordPanel->cancelRecord();
}

void VNoteRecordBar::onClosePlayWidget(VNVoiceBlock *voiceData)
{
    m_mainLayout->setCurrentWidget(m_recordBtnHover);
    emit sigWidgetClose(voiceData);
}

void VNoteRecordBar::playVoice(VNVoiceBlock *voiceData)
{
    m_mainLayout->setCurrentWidget(m_playPanel);
    m_playPanel->setVoiceBlock(voiceData);
}

void VNoteRecordBar::pauseVoice(VNVoiceBlock *voiceData)
{
    if(m_mainLayout->currentWidget() == m_playPanel
            && m_playPanel->getVoiceData() == voiceData){
        m_playPanel->onPauseBtnClicked();
    }
}
bool VNoteRecordBar::stopVoice(VNVoiceBlock *voiceData)
{
    if(m_mainLayout->currentWidget() == m_playPanel
            && m_playPanel->getVoiceData() == voiceData){
        m_playPanel->stopVideo();
        m_mainLayout->setCurrentWidget(m_recordBtnHover);
        return true;
    }
    return false;
}

 VNVoiceBlock* VNoteRecordBar::getVoiceData()
 {
     if(m_mainLayout->currentWidget() == m_playPanel){
         return  m_playPanel->getVoiceData();
     }
     return nullptr;
 }

 void VNoteRecordBar::playOrPauseVoice()
 {
     if(m_mainLayout->currentWidget() == m_playPanel){
         QMediaPlayer::State status = m_playPanel->getPlayerStatus();
         if(status == QMediaPlayer::PlayingState){
             m_playPanel->onPauseBtnClicked();
         }else if (status == QMediaPlayer::PausedState) {
            m_playPanel->onPlayBtnClicked();
        }
     }
 }
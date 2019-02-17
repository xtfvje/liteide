/**************************************************************************
** This file is part of LiteIDE
**
** Copyright (c) 2011-2019 visualfc. All rights reserved.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** In addition, as a special exception,  that plugins developed for LiteIDE,
** are allowed to remain closed sourced and can be distributed under any license .
** These rights are included in the file LGPL_EXCEPTION.txt in this package.
**
**************************************************************************/
// Module: fakevimedit.cpp
// Creator: jsuppe <jon.suppe@gmail.com>

#include "fakevim/fakevim/fakevimhandler.h"
#include "fakevim/fakevim/fakevimactions.h"
#include "fakevimedit.h"
#include "fakevimedit_global.h"
#include "qtc_editutil/uncommentselection.h"
#include "litebuildapi/litebuildapi.h"
#include "fileutil/fileutil.h"
#include <QMenu>
#include <QToolBar>
#include <QAction>
#include <QTextStream>
#include <QApplication>
#include <QToolTip>
#include <QLabel>
#include <QStatusBar>
#include "liteeditorapi/liteeditorapi.h"
#include "../liteeditor/liteeditor_global.h"

using namespace FakeVim::Internal;

//lite_memory_check_begin
#if defined(WIN32) && defined(_MSC_VER) &&  defined(_DEBUG)
     #define _CRTDBG_MAP_ALLOC
     #include <stdlib.h>
     #include <crtdbg.h>
     #define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
     #define new DEBUG_NEW
#endif
//lite_memory_check_end

FakeVimEdit::FakeVimEdit(LiteApi::IApplication *app, QObject *parent) :
    QObject(parent),
    m_liteApp(app),
    m_enableUseFakeVim(false),
    m_commandLabel(0)
{
    connect(m_liteApp->editorManager(),SIGNAL(editorCreated(LiteApi::IEditor*)),this,SLOT(editorCreated(LiteApi::IEditor*)));
    connect(m_liteApp->editorManager(),SIGNAL(currentEditorChanged(LiteApi::IEditor*)),this,SLOT(currentEditorChanged(LiteApi::IEditor*)));
    connect(m_liteApp->optionManager(),SIGNAL(applyOption(QString)),this,SLOT(applyOption(QString)));

    this->applyOption(OPTION_FAKEVIMEDIT);

    m_enableUseFakeVim = m_liteApp->settings()->value(FAKEVIMEDIT_USEFAKEVIM,false).toBool();

    m_enableUseFakeVimAct = new QAction(tr("Use FakeVim Editing"),this);
    m_enableUseFakeVimAct->setCheckable(true);
    m_enableUseFakeVimAct->setChecked(m_enableUseFakeVim);

    connect(m_enableUseFakeVimAct,SIGNAL(toggled(bool)),this,SLOT(toggledEnableUseFakeVim(bool)));

    if (m_enableUseFakeVim) {
        _enableFakeVim();
    }
}

void FakeVimEdit::toggledEnableUseFakeVim(bool b)
{
    m_enableUseFakeVim = b;
    m_liteApp->settings()->setValue(FAKEVIMEDIT_USEFAKEVIM,b);
    if(m_enableUseFakeVim){
        _enableFakeVim();
    }else{
        _disableFakeVim();
    }
}

void FakeVimEdit::applyOption(const QString &option)
{
    if (option != OPTION_FAKEVIMEDIT) {
        return;
    }
    m_initCommandList = m_liteApp->settings()->value(FAKEVIMEDIT_INITCOMMANDS,initCommandList()).toStringList();
}


void FakeVimEdit::_enableFakeVim(){
    LiteApi::IEditor *editor = m_liteApp->editorManager()->currentEditor();
    _addCommandLabel();
    _addFakeVimToEditor(editor);
}

void FakeVimEdit::_disableFakeVim(){
    LiteApi::IEditor *editor = m_liteApp->editorManager()->currentEditor();
    _removeFakeVimFromEditor(editor);
    _removeCommandLabel();
}

QFont FakeVimEdit::commandLabelFont(){
    QFont font;
    font.setStyleHint(QFont::Monospace);
    font.setBold(true);
    return font;
}

void FakeVimEdit::_addCommandLabel(){
    QFont font = commandLabelFont();

    _removeCommandLabel();
    m_commandLabel = new QLabel(m_liteApp->mainWindow());
    m_commandLabel->setFont(font);
    m_liteApp->mainWindow()->statusBar()->addPermanentWidget(m_commandLabel);
}

void FakeVimEdit::_removeCommandLabel(){
    if(!m_commandLabel){
        return;
    }
    m_liteApp->mainWindow()->statusBar()->removeWidget(m_commandLabel);
    delete m_commandLabel;
    m_commandLabel = NULL;
}

void FakeVimEdit::_removeFakeVimFromEditor(LiteApi::IEditor *editor){
    LiteApi::ILiteEditor  *ed = LiteApi::getLiteEditor(editor);

    if (!ed) {
        return;
    }
    QString mime = editor->mimeType();

    bool tabToSpace = false;
    int tabWidth = 4;
    LiteApi::IMimeType *im = m_liteApp->mimeTypeManager()->findMimeType(mime);
    if (im) {
        tabToSpace = im->tabToSpace();
        tabWidth = im->tabWidth();
    }
    tabWidth = m_liteApp->settings()->value(MIMETYPE_TABWIDTH+mime,tabWidth).toInt();
    tabToSpace = m_liteApp->settings()->value(MIMETYPE_TABTOSPACE+mime,tabToSpace).toBool();
    ed->setTabOption(tabWidth,tabToSpace);

    QPlainTextEdit *ped = LiteApi::getPlainTextEdit(ed);

    if(!ped){
        return;
    }

    if(FakeVimHandler *fakeVimHandler = m_editorMap.value(ped)){
        delete fakeVimHandler;
        m_editorMap.remove(ped);
    }
}

void FakeVimEdit::_addFakeVimToEditor(LiteApi::IEditor *editor){
    LiteApi::ILiteEditor  *ed = LiteApi::getLiteEditor(editor);

    if (!ed) {
        return;
    }

    QPlainTextEdit *ped = LiteApi::getPlainTextEdit(ed);

    if(!ped){
        return;
    }

    if(m_editorMap.contains(ped)){
        return;
    }

    FakeVimHandler *fakeVimHandler;

    fakeVimHandler = new FakeVimHandler(ped,0);

    connect(fakeVimHandler, SIGNAL(handleExCommandRequested(bool*,ExCommand)),
            this, SLOT(handleExCommandRequested(bool*,ExCommand)));
    connect(fakeVimHandler, SIGNAL(commandBufferChanged(QString,int,int,int,QObject*)),
            this, SLOT(showMessage(QString,int)));
    connect(fakeVimHandler, SIGNAL(moveToMatchingParenthesis(bool *, bool *, QTextCursor *)),
            this, SLOT(moveToMatchingParenthesis(bool *, bool *,QTextCursor *)));

    //init command list
    {
        fakeVimHandler->handleCommand("set shiftwidth=4");
        fakeVimHandler->handleCommand("set tabstop=4");
        foreach(QString cmd, m_initCommandList) {
            if (cmd.startsWith("#")) {
                continue;
            }
            fakeVimHandler->handleCommand(cmd);
        }
        fakeVimHandler->handleInput("<esc>");
    }

    fakeVimHandler->setCurrentFileName(ed->filePath());
    fakeVimHandler->installEventFilter();
    fakeVimHandler->setupWidget();

    connect(ped, SIGNAL(destroyed(QObject*)), this, SLOT(plainTextEditDestroyed(QObject*)));

    m_editorMap[ped] = fakeVimHandler;
}

void FakeVimEdit::plainTextEditDestroyed(QObject *obj)
{
    m_editorMap.remove(obj);
}

void FakeVimEdit::handleExCommandRequested(bool *b, ExCommand c)
{
    // Save
    if(c.cmd == "w" ){
        m_liteApp->editorManager()->saveEditor(m_editor);
        *b = true;
    }

    // Save & Close
    if(c.cmd == "x"){
        m_liteApp->editorManager()->saveEditor(m_editor);
        m_liteApp->editorManager()->closeEditor(m_editor);
        *b = true;
    }

    // Close
    if(c.cmd == "q"){
        if(c.hasBang){
            m_editor->reload();
        }
        m_liteApp->editorManager()->closeEditor(m_editor);
        *b = true;
    }
}

void FakeVimEdit::moveToMatchingParenthesis(bool *moved, bool *forward, QTextCursor *cursor)
{
    LiteApi::IEditor *editor = m_editor;
    LiteApi::IActionContext *actionContext = m_liteApp->actionManager()->getActionContext(editor,"Editor");
    LiteApi::ActionInfo *info = actionContext->actionInfo("GotoMatchBrace");

    info->action->trigger();

    int oldPos = cursor->position();
    int newPos = this->m_editor->textCursor().position();
    cursor->setPosition(newPos);

    if(oldPos <= newPos){
        *forward = true;
    }else{
        *forward = false;
    }
    if(oldPos == newPos){
        *moved = false;
    }else{
        *moved = true;
    }
}


void FakeVimEdit::editorCreated(LiteApi::IEditor *editor)
{
    if (!editor) {
        return;
    }

    QMenu *menu = LiteApi::getEditMenu(editor);
    if (menu) {
        menu->addSeparator();
        menu->addAction(m_enableUseFakeVimAct);
    }

    if (!m_enableUseFakeVim){
        return;
    }

    m_editor = LiteApi::getLiteEditor(editor);
    if (m_editor) {
        m_plainTextEdit = LiteApi::getPlainTextEdit(editor);
    }else{
        return;
    }

    if(!m_enableUseFakeVim)
        return;

    _addFakeVimToEditor(editor);
}

void FakeVimEdit::currentEditorChanged(LiteApi::IEditor *editor)
{
    if (!editor) {
        return;
    }
    m_editor = LiteApi::getLiteEditor(editor);
    QPlainTextEdit *ped = LiteApi::getPlainTextEdit(editor);

    if (m_enableUseFakeVim){
        if(!m_editorMap.contains(ped)) {
            _addFakeVimToEditor(editor);
        }
    }else{
        if (m_editorMap.contains(ped)) {
            _removeFakeVimFromEditor(editor);
        }
    }
}

void FakeVimEdit::showMessage(QString contents, int cursorPos)
{
    if(!m_commandLabel){
        return;
    }
    QString m_statusMessage = cursorPos == -1 ? contents
        : contents.left(cursorPos) + QChar(10073) + contents.mid(cursorPos);

    int slack = 14 - m_statusMessage.size();
    QString msg = m_statusMessage + QString(slack, QLatin1Char(' '));

    m_commandLabel->setText(msg);
}

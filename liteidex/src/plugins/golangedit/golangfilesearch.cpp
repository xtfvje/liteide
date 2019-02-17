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
// Module: golangfilesearch.cpp
// Creator: visualfc <visualfc@gmail.com>

#include "golangfilesearch.h"
#include "litebuildapi/litebuildapi.h"
#include <QTextBlock>
//lite_memory_check_begin
#if defined(WIN32) && defined(_MSC_VER) &&  defined(_DEBUG)
     #define _CRTDBG_MAP_ALLOC
     #include <stdlib.h>
     #include <crtdbg.h>
     #define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
     #define new DEBUG_NEW
#endif
//lite_memory_check_end

GolangFileSearch::GolangFileSearch(LiteApi::IApplication *app, QObject *parent) :
    LiteApi::IFileSearch(parent), m_liteApp(app)
{
    m_process = new ProcessEx(this);
    m_replaceMode = false;
    m_bParserHead = true;
//    connect(m_process,SIGNAL(started()),this,SLOT(findUsagesStarted()));
    connect(m_process,SIGNAL(extOutput(QByteArray,bool)),this,SLOT(findUsagesOutput(QByteArray,bool)));
    connect(m_process,SIGNAL(extFinish(bool,int,QString)),this,SLOT(findUsagesFinish(bool,int,QString)));
}

GolangFileSearch::~GolangFileSearch()
{
}

QString GolangFileSearch::mimeType() const
{
    return "find/gofilesearch";
}

QString GolangFileSearch::displayName() const
{
    return tr("Golang Find Usages");
}

QWidget *GolangFileSearch::widget() const
{
    return 0;
}

void GolangFileSearch::start()
{

}

void GolangFileSearch::cancel()
{
}

void GolangFileSearch::activate()
{

}

QString GolangFileSearch::searchText() const
{
    return m_searchText;
}

bool GolangFileSearch::replaceMode() const
{
    return m_replaceMode;
}

void GolangFileSearch::setSearchInfo(const QString &/*text*/, const QString &/*filter*/, const QString &/*path*/)
{

}

void GolangFileSearch::findUsages(LiteApi::ITextEditor *editor, QTextCursor cursor, bool global, bool skip_goroot, bool replace)
{
    if (!m_process->isStop()) {
        m_process->stopAndWait(100,2000);
    }

    bool moveLeft = false;
    int selectStart = 0;
    m_searchText = LiteApi::wordUnderCursor(cursor,&moveLeft,&selectStart);
    if (m_searchText.isEmpty() || m_searchText.contains(" ")) {
        return;
    }

    m_liteApp->editorManager()->saveAllEditors(false);

    //int offset = moveLeft ? editor->utf8Position(true)-1: editor->utf8Position(true);
    int offset = editor->utf8Position(true,selectStart);

    LiteApi::IFileSearchManager *manager = LiteApi::getFileSearchManager(m_liteApp);
    if (!manager) {
        return;
    }
    this->m_replaceMode = replace;
    manager->setCurrentSearch(this);
    m_lastLine = 0;
    m_bParserHead = true;
    m_file.close();
    QString cmd = LiteApi::getGotools(m_liteApp);
    QFileInfo info(editor->filePath());
    m_process->setEnvironment(LiteApi::getCustomGoEnvironment(m_liteApp,editor).toStringList());
    m_process->setWorkingDirectory(info.path());

    QStringList args;
    args << "types";
    QString tags = LiteApi::getGoBuildFlagsArgument(m_liteApp,editor,"-tags");
    if (!tags.isEmpty()) {
        args << "-tags";
        args << tags;
    }
    args << "-pos";
    args << QString("\"%1:%2\"").arg(info.fileName()).arg(offset);
    args << "-info";
    args << "-use";
    QString text = selectionUnderCursor(cursor,moveLeft);
    if (!text.isEmpty()) {
        args << "-text" << text;
    }
    if (global) {
        args << "-all";
    }
    if (skip_goroot) {
        args << "-skip_goroot";
    }
    args << ".";

    emit findStarted();
    m_process->startEx(cmd,args);
}

void GolangFileSearch::findUsagesStarted()
{
}

void GolangFileSearch::findUsagesOutput(QByteArray data, bool bStdErr)
{
    if (bStdErr) {
        QString info = QString::fromUtf8(data).trimmed();
        emit findError(info);
        m_liteApp->appendLog("find usage error",info,true);
        return;
    }
    QRegExp reg(":(\\d+):(\\d+)");
    foreach (QByteArray line, data.split('\n')) {
        QString info = QString::fromUtf8(line).trimmed();
        if (m_bParserHead) {
            m_bParserHead = false;
            //package fmt
            //package ast ("go/ast")
            if (info.startsWith("package ") || info.startsWith("import ")) {
                int n = info.indexOf("(");
                if (n != -1) {
                    info = info.left(n);
                }
                //change searchText
                //m_searchText = info.mid(7).trimmed();
                QString pkgName = info.mid(7).trimmed();
                if (pkgName != m_searchText) {
                    m_searchText = pkgName;
                    emit searchTextChanged(pkgName);
                }
            }
            continue;
        }
        int pos = reg.lastIndexIn(info);
        if (pos >= 0) {
            QString fileName = info.left(pos);
            int fileLine = reg.cap(1).toInt();
            int fileCol = reg.cap(2).toInt();
            if (m_file.fileName() != fileName) {
                m_file.close();
                m_file.setFileName(fileName);
            }
            if (!m_file.isOpen()) {
                m_file.open(QFile::ReadOnly);
                m_lastLine = 0;
            }
            if (m_file.isOpen()) {
                if (fileLine != m_lastLine) {
                    while(!m_file.atEnd()) {
                        QByteArray line = m_file.readLine();
                        m_lastLine++;
                        if (fileLine == m_lastLine) {
                            m_lastLineText = QString::fromUtf8(trimmedRight(line));
                            if (fileCol > 0) {
                               fileCol = QString::fromUtf8(line.left(fileCol)).length();
                            }
                            break;
                        }
                    }
                } else {
                    QByteArray line = m_lastLineText.toUtf8();
                    if (fileCol > 0) {
                       fileCol = QString::fromUtf8(line.left(fileCol)).length();
                    }
                }
                emit findResult(LiteApi::FileSearchResult(fileName,m_lastLineText,fileLine,fileCol-1,m_searchText.length()));
            }
        }
    }
}

void GolangFileSearch::findUsagesFinish(bool b, int, QString)
{
    m_file.close();
    m_lastLine = 0;
    emit findFinished(b);
}

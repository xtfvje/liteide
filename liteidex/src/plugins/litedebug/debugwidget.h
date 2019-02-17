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
// Module: debugwidget.h
// Creator: visualfc <visualfc@gmail.com>

#ifndef DEBUGWIDGET_H
#define DEBUGWIDGET_H

#include "liteapi/liteapi.h"
#include "litedebugapi/litedebugapi.h"
#include "textoutput/textoutput.h"
#include "symboltreeview/symboltreeview.h"

class QTabWidget;
class QTreeView;
class QLineEdit;
class QPlainTextEdit;

class DebugWidget : public QObject
{
    Q_OBJECT
public:
    explicit DebugWidget(LiteApi::IApplication *app,QObject *parent = 0);
    virtual ~DebugWidget();
    virtual QWidget *widget();
    void updateView(QTreeView *view, LiteApi::IDebugger *debug, LiteApi::DEBUG_MODEL_TYPE type, const QString &title);
signals:
    void debugCmdInput();
public slots:
    void setInputFocus();
    void loadDebugInfo(const QString &id);
    void saveDebugInfo(const QString &id);
    void addWatch();
    void removeWatch();
    void removeAllWatchAct();
    void watchViewContextMenu(QPoint);
    void setDebugger(LiteApi::IDebugger *debug);
    void enterText(const QString &cmd);
    void appendLog(const QString &log);
    void clearLog();
    void expandedVarsView(QModelIndex);
    void setExpand(LiteApi::DEBUG_MODEL_TYPE type, const QModelIndex &index, bool expanded);
    void watchCreated(QString,QString);
    void watchRemoved(QString);
    void doubleClickedStack(QModelIndex);
    void beginUpdateModel(LiteApi::DEBUG_MODEL_TYPE type);
    void endUpdateModel(LiteApi::DEBUG_MODEL_TYPE type);
protected:
    LiteApi::IApplication *m_liteApp;
    QWidget *m_widget;
    LiteApi::IDebugger *m_debugger;
    QTabWidget *m_tabWidget;
    QTreeView *m_asyncView;
    SymbolTreeView *m_varsView;
    SymbolTreeView *m_watchView;
    QTreeView *m_statckView;
    QTreeView *m_libraryView;
    QTreeView *m_threadsView;
    QTreeView *m_regsView;
    SymbolTreeView *m_goroutinesView;
    SymbolTreeState m_varsState;
    SymbolTreeState m_watchState;
    SymbolTreeState m_goroutinesState;
    TextOutput *m_debugLogEdit;
    QMenu      *m_watchMenu;
    QAction    *m_addWatchAct;
    //QAction    *m_addLocalWatchAct;
    QAction    *m_removeWatchAct;
    QAction     *m_removeAllWatchAct;
    QMap<QString,QString> m_watchMap;
};

#endif // DEBUGWIDGET_H

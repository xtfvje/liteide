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
// Module: golangfmtoption.cpp
// Creator: visualfc <visualfc@gmail.com>

#include "golangfmtoption.h"
#include "golangfmt_global.h"
#include "ui_golangfmtoption.h"
//lite_memory_check_begin
#if defined(WIN32) && defined(_MSC_VER) &&  defined(_DEBUG)
     #define _CRTDBG_MAP_ALLOC
     #include <stdlib.h>
     #include <crtdbg.h>
     #define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
     #define new DEBUG_NEW
#endif
//lite_memory_check_end

GolangFmtOption::GolangFmtOption(LiteApi::IApplication *app,QObject *parent) :
    LiteApi::IOption(parent),
    m_liteApp(app),
    m_widget(new QWidget),
    ui(new Ui::GolangFmtOption)
{
    ui->setupUi(m_widget);

    bool fixImports = m_liteApp->settings()->value(GOLANGFMT_FIXIMPORTS,false).toBool();
    bool autofmt = m_liteApp->settings()->value(GOLANGFMT_AUTOFMT,true).toBool();
    bool syncfmt = m_liteApp->settings()->value(GOLANGFMT_USESYNCFMT,true).toBool();
    int timeout = m_liteApp->settings()->value(GOLANGFMT_SYNCTIMEOUT,500).toInt();
    ui->checkBoxUseGoimports->setChecked(fixImports);
    ui->checkBoxAutoFmt->setChecked(autofmt);
    ui->enableSyncCheckBox->setChecked(syncfmt);
    ui->syncTimeoutLineEdit->setText(QString("%1").arg(timeout));
}

GolangFmtOption::~GolangFmtOption()
{
    delete m_widget;
    delete ui;
}

QWidget *GolangFmtOption::widget()
{
    return m_widget;
}

QString GolangFmtOption::name() const
{
    return "GolangFmt";
}

QString GolangFmtOption::mimeType() const
{
    return "option/golangfmt";
}

void GolangFmtOption::apply()
{
    bool goimports = ui->checkBoxUseGoimports->isChecked();
    bool autofmt = ui->checkBoxAutoFmt->isChecked();
    bool syncfmt = ui->enableSyncCheckBox->isChecked();
    m_liteApp->settings()->setValue(GOLANGFMT_FIXIMPORTS,goimports);
    m_liteApp->settings()->setValue(GOLANGFMT_AUTOFMT,autofmt);
    m_liteApp->settings()->setValue(GOLANGFMT_USESYNCFMT,syncfmt);
    int timeout = ui->syncTimeoutLineEdit->text().toInt();
    if (timeout < 500) {
        timeout = 500;
    }
    ui->syncTimeoutLineEdit->setText(QString("%1").arg(timeout));
    m_liteApp->settings()->setValue(GOLANGFMT_SYNCTIMEOUT,timeout);
}

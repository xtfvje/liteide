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
// Module: litebuild.h
// Creator: visualfc <visualfc@gmail.com>

#ifndef LITEBUILD_H
#define LITEBUILD_H

#include "liteapi/liteapi.h"
#include "liteenvapi/liteenvapi.h"
#include "litebuildapi/litebuildapi.h"
#include <QTextCursor>

#define LITEBUILD_TAG "lightbuild/navtag"

class BuildManager;
class QComboBox;
class ProcessEx;
class TextOutput;
class QLabel;
class QCheckBox;
class QStandardItemModel;

struct BuildBarInfo;
class LiteBuild : public LiteApi::ILiteBuild
{
    Q_OBJECT
public:
    explicit LiteBuild(LiteApi::IApplication *app, QObject *parent = 0);
    virtual ~LiteBuild();
public:
    virtual QString buildTag() const;
    virtual QMap<QString,QString> buildEnvMap() const;
    virtual QMap<QString,QString> liteideEnvMap() const;
    virtual LiteApi::TargetInfo getTargetInfo();
    virtual QString envValue(LiteApi::IBuild *build, const QString &value);
    virtual QString buildPathEnvValue(LiteApi::IBuild *build, const QString &buildFilePath, const QString &value);
    virtual LiteApi::IBuildManager *buildManager() const;
    virtual void appendOutput(const QString &str, const QBrush &brush, bool active, bool updateExistsTextColor = true);
    virtual void execCommand(const QString &cmd, const QString &args, const QString &workDir,bool updateExistsTextColor = true, bool activateOutputCheck = true, bool navigate = true, bool command = true);
    virtual bool execGoCommand(const QStringList &args, const QString &workDir, bool waitFinish);
    QMap<QString,QString> buildEnvMap(LiteApi::IBuild *build, const QString &buildTag) const;
public:
    QString envToValue(const QString &value,QMap<QString,QString> &liteEnv,const QProcessEnvironment &env);
    void setCurrentBuild(LiteApi::IBuild *build);
    void updateBuildConfig(LiteApi::IBuild *);
    void updateBuildConfigHelp(LiteApi::IBuild *build, const QString &buildRootPath, QStandardItemModel *liteideModel, QStandardItemModel *configModel, QStandardItemModel *customModel, QStandardItemModel *actionModel);
    void loadProjectInfo(const QString &filePath);
    void loadEditorInfo(const QString &filePath);
    void loadTargetInfo(LiteApi::IBuild *build);
    LiteApi::IBuild *findProjectBuildByEditor(LiteApi::IEditor *editor);
    LiteApi::IBuild *findProjectBuild(LiteApi::IProject *project);
    void setDynamicBuild();
    void loadBuildPath(const QString &buildPath, const QString &buildName, const QString &buildInfo);
    void loadBuildType(const QString &mimeType);
    bool isLockBuildRoot() const;
    QString currentBuildPath() const;
    void lockBuildRootByMimeType(const QString &path, const QString &mimeType);
signals:
    void buildPathChanged(const QString &buildPath);
public slots:
    void appLoaded();
    void debugBefore();
    void currentEnvChanged(LiteApi::IEnv*);
    void currentProjectChanged(LiteApi::IProject*);
    void reloadProject();
    void editorCreated(LiteApi::IEditor *editor);
    void currentEditorChanged(LiteApi::IEditor*);
    void execBuildAction(LiteApi::IBuild*,LiteApi::BuildAction*);
    void buildTask(LiteApi::IBuild *build, bool killOld, const QStringList &taskList);
    void execAction(const QString &mime,const QString &id);
    void extOutput(const QByteArray &output,bool bError);
    void extFinish(bool error,int exitCode, QString msg);
    void debugEnvOutput(const QByteArray &output,bool bError);
    void stopAction();
    void dbclickBuildOutput(const QTextCursor &cur);
    void enterTextBuildOutput(QString);
    void config();
    void aboutToShowFolderContextMenu(QMenu *menu, LiteApi::FILESYSTEM_CONTEXT_FLAG flag, const QFileInfo &info);
    void fmctxExecuteFile();
    void fmctxDebugFile();
    void fmctxGoLockBuild();
    void fmctxGoBuildConfigure();
    void fmctxGoTool();
    void fmctxGofmt();
    void fmctxGodoc();
    void applyOption(QString);
    void lockBuildRoot(bool b);
    void setOutputLineWrap(bool b);
    void setOutputAutoClear(bool b);
    void setOutputAutoPosCursor(bool b);
protected:
    QMenu *m_nullMenu;
    LiteApi::IApplication   *m_liteApp;
    BuildManager    *m_buildManager;
    LiteApi::IBuild *m_build;
    LiteApi::IEnvManager *m_envManager;
    QToolBar    *m_buildToolBar;
    QMenu       *m_buildMenu;
    QMap<QString,BuildBarInfo*> m_buildBarInfoMap;
    QString m_workDir;
    ProcessEx *m_process;
    ProcessEx *m_debugEnvProcess;
    TextOutput *m_output;
    QMenu      *m_outputMenu;
    QAction    *m_outputLineWrapAct;
    QAction    *m_outputAutoClearAct;
    QAction    *m_outputAutoPosCursorAct;
    QAction     *m_configAct;
    QAction     *m_stopAct;
    QAction     *m_clearAct;
    QAction     *m_outputAct;
    QMenu       *m_fmctxGoToolMenu;
    QMenu       *m_fmctxNoGoToolMenu;
    QAction     *m_fmctxExecuteFileAct;
    QAction     *m_fmctxDebugFileAct;
    QAction     *m_fmctxGoLockBuildAct;
    QAction     *m_fmctxGoBuildConfigAct;
    QAction     *m_fmctxGoBuildAct;
    QAction     *m_fmctxGoBuildAllAct;
    QAction     *m_fmctxGoInstallAct;
    QAction     *m_fmctxGoInstallAllAct;
    QAction     *m_fmctxGoTestAct;
    QAction     *m_fmctxGoTestAllAct;
    QAction     *m_fmctxGoCleanAct;
    QAction     *m_fmctxGoCleanAllAct;
    QAction     *m_fmctxGoGetAct;
    QAction     *m_fmctxGoGetUpdateAct;
    QAction     *m_fmctxGoGetForceAct;
    QAction     *m_fmctxGoFmtAct;
    QAction     *m_fmctxGoVetAct;
    QAction     *m_fmctxGoVetAllCheckAct;
    QAction     *m_fmctxGodocAct;
    QCheckBox   *m_checkBoxLockBuild;
    QFileInfo   m_fmctxInfo;
    QString     m_outputRegex;
    QString     m_buildMimeType;
    QString     m_buildRootPath;
    QString     m_buildRootName;
    bool        m_bOutputAutoClear;
    bool        m_bLockBuildRoot;
    bool        m_bDynamicBuild;
    bool        m_bProjectBuild;
    QMap<QString,QString> m_liteAppInfo;
    QMap<QString,QString> m_editorInfo;
    QMap<QString,QString> m_buildInfo;
    QMap<QString,QString> m_projectInfo;
    QMap<QString,QString> m_targetInfo;
};

#endif // LITEBUILD_H

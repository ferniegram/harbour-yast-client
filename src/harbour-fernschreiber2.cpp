/*
    Copyright (C) 2020 Sebastian J. Wolf and other contributors

    This file is part of Fernschreiber.

    Fernschreiber is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Fernschreiber is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Fernschreiber. If not, see <http://www.gnu.org/licenses/>.
*/

#include "ferniemain.h"

#include <sailfishapp.h>
#include <QQuickView>
//#include <QtQml>
#include <QQmlEngine>
#include <QGuiApplication>
#include <QSysInfo>
#include <QSettings>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

#include "voicenoterecorder.h"

void migrateSettings() {
    const QStringList sailfishOSVersion = QSysInfo::productVersion().split(".");
    int sailfishOSMajorVersion = sailfishOSVersion.value(0).toInt();
    int sailfishOSMinorVersion = sailfishOSVersion.value(1).toInt();
    if ((sailfishOSMajorVersion == 4 && sailfishOSMinorVersion >= 4) || sailfishOSMajorVersion > 4) {
        LOG("Checking if we need to migrate settings...");
        QSettings settings(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/io.github.roundedrectangle/fernschreiber2/settings.conf", QSettings::NativeFormat);
        if (settings.contains("migrated")) {
            return;
        }
        QSettings oldSettings(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/harbour-fernschreiber2/settings.conf", QSettings::NativeFormat);
        const QStringList oldKeys = oldSettings.allKeys();
        if (oldKeys.isEmpty()) {
            return;
        }
        LOG("SailfishOS >= 4.4 and old configuration file detected, migrating settings to new location...");
        for (const QString &key : oldKeys) {
            settings.setValue(key, oldSettings.value(key));
        }

        QDir oldDataLocation(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/harbour-fernschreiber2/harbour-fernschreiber2");
        LOG("Old data directory: " + oldDataLocation.path());
        if (oldDataLocation.exists()) {
            LOG("Old data files detected, migrating files to new location...");
            const int oldDataPathLength = oldDataLocation.absolutePath().length();
            QString dataLocationPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            QDir dataLocation(dataLocationPath);
            QDirIterator oldDataIterator(oldDataLocation, QDirIterator::Subdirectories);
            while (oldDataIterator.hasNext()) {
                oldDataIterator.next();
                QFileInfo currentFileInfo = oldDataIterator.fileInfo();
                if (!currentFileInfo.isHidden()) {
                    const QString subPath = currentFileInfo.absoluteFilePath().mid(oldDataPathLength);
                    const QString targetPath = dataLocationPath + subPath;
                    if (currentFileInfo.isDir()) {
                        LOG("Creating new directory " + targetPath);
                        dataLocation.mkpath(targetPath);
                    } else if(currentFileInfo.isFile()) {
                        LOG("Copying file to " + targetPath);
                        QFile::copy(currentFileInfo.absoluteFilePath(), targetPath);
                    }
                }
            }
        }

        settings.setValue("migrated", true);
    }
}

int main(int argc, char *argv[]) {
    FernieMain::setupLogging();

    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
    QSharedPointer<QQuickView> view(SailfishApp::createView()); // FIXME: should we actually use QScopedPointer here?

    QQmlContext *context = view.data()->rootContext();

    migrateSettings();

    QScopedPointer<FernieMain::AppContext> appContext(FernieMain::registerTypes(argc, argv, view));

    VoiceNoteRecorder *voiceNoteRecorder = new VoiceNoteRecorder(argc, argv, appContext->appSettings, view.data());
    context->setContextProperty("voiceNoteRecorder", voiceNoteRecorder);
    qmlRegisterUncreatableType<VoiceNoteRecorder>(appContext->uri, 1, 0, "VoiceNoteRecorder", QString());

#ifdef NO_HARBOUR_COMPLIANCE
    context->setContextProperty("NO_HARBOUR_COMPLIANCE", true);
#else
    context->setContextProperty("NO_HARBOUR_COMPLIANCE", false);
#endif

    view->setSource(SailfishApp::pathTo("qml/harbour-fernschreiber2.qml"));
    view->show();
    return app->exec();
}

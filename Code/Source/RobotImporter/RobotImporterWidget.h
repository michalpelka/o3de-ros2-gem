/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#if !defined(Q_MOC_RUN)
#include "RobotImporter/URDF/RobotImporter.h"
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <QCheckBox>
#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QTimer>
#include <QWidget>
#include <QWizard>
#include <QTableWidget>
#endif

namespace ROS2
{

    class FileSelectionPage : public QWizardPage{
        Q_OBJECT
    public:
        explicit FileSelectionPage(QWizard* parent);
        bool isComplete() const override;
        QString getFileName() const{
            if (m_fileExists)
            {
                return m_textEdit->text();
            }
            return "";
        }

    private:
        QFileDialog* m_fileDialog;
        QPushButton* m_button;
        QLineEdit* m_textEdit;
        void onLoadButtonPressed();
        void onFileSelected(const QString &file);
        void onTextChanged(const QString &text);
        bool m_fileExists{false};
    };

    class CheckUrdfPage : public QWizardPage{
        Q_OBJECT
    public:
        explicit CheckUrdfPage(QWizard* parent);
        void ReportURDFResult(const QString & result, bool is_success);
        bool isComplete() const override;
    private:
        QTextEdit* m_log;
        QString m_fileName;
        bool m_success;
    };

    class CheckAssetPage : public QWizardPage{
        Q_OBJECT
    public:
        explicit CheckAssetPage(QWizard* parent);
        void ReportURDFResult(const QString & result, bool is_success);
        bool isComplete() const override;
    private:
        bool m_success;
        QTableWidget* m_table;
    };

    class URDFPrefabMaker;
    //! Handles UI for the process of URDF importing
    class RobotImporterWidget : public  QWizard
    {
        Q_OBJECT
    public:
        explicit RobotImporterWidget(QWidget* parent = nullptr);

    private:
        //! Report an error to the user.
        //! Populates the log, sets status information in the status label and shows an error popup with the message
        //! @param errorMessage error message to display to the user
        void ReportError(const AZStd::string& errorMessage);

        //! Report an information to the user.
        //! Populates the log and sets status information in the status label
        //! @param infoMessage info message to display to the user
        void ReportInfo(const AZStd::string& infoMessage);
//        QLabel m_statusLabel;
//        QTextEdit m_statusText;
//        QPushButton m_selectFileButton;
        QTimer m_importerUpdateTimer;
        FileSelectionPage* m_fileSelectPage;
        CheckUrdfPage* m_checkUrdfPage;
        CheckAssetPage* m_assetPage;
        urdf::ModelInterfaceSharedPtr m_parsedUrdf;
        RobotImporter m_robotImporter;

        void ImporterTimerUpdate();
        void onCurrentIdChanged(int id);
    };
} // namespace ROS2

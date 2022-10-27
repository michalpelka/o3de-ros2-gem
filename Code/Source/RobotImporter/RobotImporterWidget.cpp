/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/IO/Path/Path.h>
#include <AzCore/Utils/Utils.h>
#include <QMessageBox>
#include <QVBoxLayout>

#include <QWizardPage>
#include <QFileSystemModel>

#include "RobotImporter/RobotImporterWidget.h"
#include "RobotImporter/RobotImporterWidgetUtils.h"
#include "RobotImporter/URDF/RobotImporter.h"
#include "RobotImporter/URDF/UrdfParser.h"

namespace ROS2
{
    QWizardPage *createIntroPage()
    {
        QWizardPage *page = new QWizardPage;
        page->setTitle("Introduction");

        QLabel *label = new QLabel("This wizard allows you to build robot simulation"
                                   "out of URDF description."
                                   "<br> before proceed please make that all assets are imported</br>");
        label->setWordWrap(true);

        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(label);
        page->setLayout(layout);

        return page;
    }

    QWizardPage *createOutroPage()
    {
        QWizardPage *page = new QWizardPage;
        page->setTitle("Outro");

        QLabel *label = new QLabel("Done");
        label->setWordWrap(true);

        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(label);
        page->setLayout(layout);

        return page;
    }

    FileSelectionPage::FileSelectionPage(QWizard* parent)
        : QWizardPage(parent)
    {
        m_fileDialog = new QFileDialog(this);
        m_button = new QPushButton("...", this);
        m_textEdit = new QLineEdit("", this);
        setTitle("Load URDF file");
        QVBoxLayout *layout = new QVBoxLayout;
        layout->addStretch();
        layout->addWidget(new QLabel( "URDF file : ", this));
        QHBoxLayout *layout_in = new QHBoxLayout;
        layout_in->addWidget(m_button);
        layout_in->addWidget(m_textEdit);
        layout->addLayout(layout_in);
        layout->addStretch();
        this->setLayout(layout);
        connect(m_button,&QPushButton::pressed, this, &FileSelectionPage::onLoadButtonPressed);
        connect(m_fileDialog, &QFileDialog::fileSelected, this, &FileSelectionPage::onFileSelected);
        connect(m_textEdit, &QLineEdit::textChanged, this, &FileSelectionPage::onTextChanged);
    }
    void FileSelectionPage::onLoadButtonPressed(){
        m_fileDialog->show();
    }
    void FileSelectionPage::onFileSelected(const QString &file){
        m_textEdit->setText(file);
    }
    void FileSelectionPage::onTextChanged(const QString &text){
        m_fileExists = QFileInfo::exists(text);
        emit completeChanged();
    }
    bool FileSelectionPage::isComplete() const {
        return m_fileExists;
    };

    CheckUrdfPage::CheckUrdfPage(QWizard* parent)
        : QWizardPage(parent)
        , m_success(false)
    {
        m_log  = new QTextEdit(this);
        setTitle("URDF opening results:");
        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(m_log);
        m_log->acceptRichText();
        m_log->setReadOnly(true);
        this->setLayout(layout);
    }

    void CheckUrdfPage::ReportURDFResult(const QString & status, bool is_success){

        m_log->setMarkdown(status);
        m_success = is_success;
        emit completeChanged();
    }

    bool CheckUrdfPage::isComplete() const
    {
        return m_success;
    }

    CheckAssetPage::CheckAssetPage(QWizard* parent)
        : QWizardPage(parent)
        , m_success(false)
    {
        m_table = new QTableWidget(parent);
        setTitle("Resolved meshes");
        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(m_table);
        m_table->setColumnCount(2);
        m_table->setRowCount(1);

        m_table->setItem(0,0,new QTableWidgetItem("Link Name"));
        m_table->setItem(0,1,new QTableWidgetItem("o3de Asset ID"));
        this->setLayout(layout);
    }

    bool CheckAssetPage::isComplete() const {
        return m_success;
    };


    RobotImporterWidget::RobotImporterWidget(QWidget* parent)
        : QWizard(parent)
//        , m_statusText("", this)
//        , m_selectFileButton(QObject::tr("Load"), this)
//        , m_statusLabel(QObject::tr("Created Entities:"), this)
        , m_importerUpdateTimer(this)
        , m_robotImporter(
              [this](RobotImporter::LogLevel level, const AZStd::string& message)
              {
                  switch (level)
                  {
                  case RobotImporter::LogLevel::Info:
                      ReportInfo(message);
                      break;
                  case RobotImporter::LogLevel::Error:
                      ReportError(message);
                      break;
                  }
              })
    {
        m_fileSelectPage = new FileSelectionPage(this);
        m_checkUrdfPage = new CheckUrdfPage(this);
        //m_assetPage = new CheckAssetPage(this);
        addPage(createIntroPage());
        addPage(m_fileSelectPage);
        addPage(m_checkUrdfPage);
        //addPage(m_assetPage);
        addPage(createOutroPage());

        connect(this, &QWizard::currentIdChanged, this, &RobotImporterWidget::onCurrentIdChanged);
        setWindowTitle("Robot Import Wizard");
        connect(this, &QDialog::finished, [this](int id){
                    AZ_Printf("page", "QDialog::finished : %d", id);
                        parentWidget()->close();
//                      AzToolsFramework::CloseViewPane("Robot Importer");

                });

//        setWindowTitle(QObject::tr("Robot definition file importer"));
//        QVBoxLayout* mainLayout = new QVBoxLayout(this);
//        QLabel* captionLabel = new QLabel(QObject::tr("Select a Unified Robot Description Format (URDF) file to import"), this);
//        captionLabel->setWordWrap(true);
//        mainLayout->addWidget(captionLabel);
//        mainLayout->addWidget(&m_selectFileButton);
//        mainLayout->addWidget(&m_statusLabel);
//        mainLayout->addWidget(&m_statusText);
//        m_statusText.setReadOnly(true);
//        connect(
//            &m_importerUpdateTimer,
//            &QTimer::timeout,
//            [this]
//            {
//                AZStd::string progress = m_robotImporter.GetProgress();
//                m_statusText.setText(progress.c_str());
//                m_robotImporter.CheckIfAssetsWereLoadedAndCreatePrefab(
//                    [this]()
//                    {
//                        m_importerUpdateTimer.stop();
//                        m_selectFileButton.setEnabled(true);
//                    });
//            });
//
//        QObject::connect(
//            &m_selectFileButton,
//            &QPushButton::clicked,
//            this,
//            [&]()
//            {
//                m_wizard.show();
//
////                AZStd::optional<AZStd::string> urdfPath = RobotImporterWidgetUtils::QueryUserForURDFPath(this);
////                if (!urdfPath)
////                {
////                    return;
////                }
////
////                AZ::IO::Path prefabName(AZ::IO::PathView(urdfPath.value()).Filename());
////                prefabName.ReplaceExtension("prefab");
////                const AZ::IO::Path prefabDefaultPath(AZ::IO::Path(AZ::Utils::GetProjectPath()) / "Assets" / "Importer" / prefabName);
////                auto prefabPath =
////                    RobotImporterWidgetUtils::ValidatePrefabPathExistenceAndQueryUserForNewIfNecessary(prefabDefaultPath, this);
////                if (!prefabPath)
////                {
////                    ReportError("User cancelled");
////                    return;
////                }
////                m_robotImporter.ParseURDFAndStartLoadingAssets({ urdfPath.value(), prefabPath->c_str() });
////
////                // Disable the button until the import is complete to prevent the user from clicking it again
////                m_selectFileButton.setEnabled(false);
////                // Check whether import is still in progress every 0.5 seconds
////                m_importerUpdateTimer.start(500);
//            });
//
//        setLayout(mainLayout);
    }

    void RobotImporterWidget::ReportError(const AZStd::string& errorMessage)
    {
//        QMessageBox::critical(this, QObject::tr("Error"), QObject::tr(errorMessage.c_str()));
//        AZStd::string progress = m_robotImporter.GetProgress();
//        m_statusText.setText(QObject::tr((progress + errorMessage).c_str()));
//        AZ_Error("RobotImporterWidget", false, errorMessage.c_str());
    }
    void RobotImporterWidget::onCurrentIdChanged(int id)
    {
        AZ_Printf("Wizard", "Wizard at page %d", id);
        if (currentPage() == m_checkUrdfPage){
            const AZStd::string fileName( m_fileSelectPage->getFileName().toUtf8().constData());
            if (!fileName.empty())
            {
                AZ_Printf("Wizard", "Testing urdf file : %s", fileName.c_str());
                m_parsedUrdf = UrdfParser::ParseFromFile(fileName);
                QString report;
                const auto log = UrdfParser::getUrdfParsingLog();
                if (m_parsedUrdf){
                    report+="# The URDF was parsed and opened successfully\n";
                }else{
                    report+="# The URDF was not opened\n";
                    report+="URDF parser returned following errors:\n\n";
                }
                report += "`";
                report +=  QString::fromUtf8(log.data(), int(log.size()));
                report += "`";
                m_checkUrdfPage->ReportURDFResult(report, m_parsedUrdf!=nullptr);
            }
        }

    }
    void RobotImporterWidget::ReportInfo(const AZStd::string& infoMessage)
    {
//        AZStd::string progress = m_robotImporter.GetProgress();
//        m_statusText.setText(QObject::tr((progress + infoMessage).c_str()));
//        AZ::Debug::Trace::Instance().Output("RobotImporterWidget", infoMessage.c_str());
    }
} // namespace ROS2

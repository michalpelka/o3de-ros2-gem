/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/IO/FileIO.h>
#include <AzCore/IO/Path/Path.h>
#include <AzCore/Utils/Utils.h>

#include "RobotImporter/RobotImporterWidget.h"
#include "RobotImporter/RobotImporterWidgetUtils.h"
#include "RobotImporter/URDF/URDFPrefabMaker.h"
#include "RobotImporter/URDF/UrdfParser.h"
#include "RobotImporter/Utils/RobotImporterUtils.h"

namespace ROS2
{
    QWizardPage* createIntroPage()
    {
        QWizardPage* page = new QWizardPage;
        page->setTitle("Introduction");

        QLabel* label = new QLabel("This wizard allows you to build robot simulation"
                                   "out of URDF description."
                                   "<br> before proceed please make that all assets are imported</br>");
        label->setWordWrap(true);

        QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget(label);
        page->setLayout(layout);
        return page;
    }

    FileSelectionPage::FileSelectionPage(QWizard* parent)
        : QWizardPage(parent)
    {
        m_fileDialog = new QFileDialog(this);
        m_fileDialog->setDirectory(QString::fromUtf8(AZ::Utils::GetProjectPath().data()));
        m_fileDialog->setNameFilter("URDF (*.urdf *.udf)");
        m_button = new QPushButton("...", this);
        // TODO remove this!
        m_textEdit = new QLineEdit("/home/michal/01-myfirst_prismatic.urdf", this);
        setTitle("Load URDF file");
        QVBoxLayout* layout = new QVBoxLayout;
        layout->addStretch();
        layout->addWidget(new QLabel("URDF file : ", this));
        QHBoxLayout* layout_in = new QHBoxLayout;
        layout_in->addWidget(m_button);
        layout_in->addWidget(m_textEdit);
        layout->addLayout(layout_in);
        layout->addStretch();
        this->setLayout(layout);
        connect(m_button, &QPushButton::pressed, this, &FileSelectionPage::onLoadButtonPressed);
        connect(m_fileDialog, &QFileDialog::fileSelected, this, &FileSelectionPage::onFileSelected);
        connect(m_textEdit, &QLineEdit::textChanged, this, &FileSelectionPage::onTextChanged);
        FileSelectionPage::onTextChanged(m_textEdit->text());
    }

    void FileSelectionPage::onLoadButtonPressed()
    {
        m_fileDialog->show();
    }
    void FileSelectionPage::onFileSelected(const QString& file)
    {
        m_textEdit->setText(file);
    }
    void FileSelectionPage::onTextChanged(const QString& text)
    {
        m_fileExists = QFileInfo::exists(text);
        emit completeChanged();
    }
    bool FileSelectionPage::isComplete() const
    {
        return m_fileExists;
    };

    CheckUrdfPage::CheckUrdfPage(QWizard* parent)
        : QWizardPage(parent)
        , m_success(false)
    {
        m_log = new QTextEdit(this);
        setTitle("URDF opening results:");
        QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget(m_log);
        m_log->acceptRichText();
        m_log->setReadOnly(true);
        this->setLayout(layout);
    }

    void CheckUrdfPage::ReportURDFResult(const QString& status, bool is_success)
    {
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
        , m_success(true)
    {
        m_table = new QTableWidget(parent);
        setTitle("Resolved meshes, [[placeholder]]");
        QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget(m_table);
        m_table->setColumnCount(3);
        m_table->setRowCount(0);
        m_table->setShowGrid(true);
        m_table->setSelectionMode(QAbstractItemView::SingleSelection);
        m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_table->setHorizontalHeaderLabels({ "URDF mesh path", "Type", "o3de asset id" });
        m_table->horizontalHeader()->setStretchLastSection(true);
        this->setLayout(layout);
    }

    bool CheckAssetPage::isComplete() const
    {
        return m_success;
    };

    void CheckAssetPage::ReportAsset(const AZStd::string& urdfPath, const AZStd::string& type, const AZ::Data::AssetId& assetId)
    {
        int i = m_table->rowCount();
        m_table->setRowCount(i + 1);
        const auto asset_str = assetId.ToString<AZStd::string>();
        QString qurdfPath = QString::fromUtf8(urdfPath.data(), int(urdfPath.size()));
        QString qtype = QString::fromUtf8(type.data(), int(type.size()));
        QString qassetId = QString::fromUtf8(asset_str.data(), int(asset_str.size()));
        m_table->setItem(i, 0, new QTableWidgetItem(qurdfPath));
        m_table->setItem(i, 1, new QTableWidgetItem(qtype));
        m_table->setItem(i, 2, new QTableWidgetItem(qassetId));
    }

    void CheckAssetPage::ClearAssetsList()
    {
        m_table->setRowCount(0);
    }

    PrefabMakerPage::PrefabMakerPage(RobotImporterWidget* parent)
        : QWizardPage(parent)
        , m_parentImporterWidget(parent)
        , m_success(false)
    {
        m_prefabName = new QLineEdit(this);
        m_createButton = new QPushButton("Create Prefab", this);
        m_log = new QTextEdit(this);
        setTitle("Prefab creation");
        QVBoxLayout* layout = new QVBoxLayout;
        QHBoxLayout* layoutInner = new QHBoxLayout;
        layoutInner->addWidget(m_prefabName);
        layoutInner->addWidget(m_createButton);
        layout->addLayout(layoutInner);
        layout->addWidget(m_log);
        this->setLayout(layout);
        connect(m_createButton, &QPushButton::pressed, this, &PrefabMakerPage::onCreateButtonPressed);
    }

    void PrefabMakerPage::setProposedPrefabName(const AZStd::string prefabName)
    {
        m_prefabName->setText(QString::fromUtf8(prefabName.data(), int(prefabName.size())));
    }

    void PrefabMakerPage::onCreateButtonPressed()
    {
        AZStd::string prefabName(m_prefabName->text().toUtf8().constData());
        m_parentImporterWidget->CreatePrefab(prefabName);
    }
    void PrefabMakerPage::reportProgress(const AZStd::string& progressForUser)
    {
        m_log->setText(QString::fromUtf8(progressForUser.data(), int(progressForUser.size())));
    }
    void PrefabMakerPage::setSuccess(bool success)
    {
        m_success = success;
        emit completeChanged();
    }
    bool PrefabMakerPage::isComplete() const
    {
        return m_success;
    }

    RobotImporterWidget::RobotImporterWidget(QWidget* parent)
        : QWizard(parent)
    {
        m_fileSelectPage = new FileSelectionPage(this);
        m_checkUrdfPage = new CheckUrdfPage(this);
        m_assetPage = new CheckAssetPage(this);
        m_prefabMakerPage = new PrefabMakerPage(this);

        addPage(createIntroPage());
        addPage(m_fileSelectPage);
        addPage(m_checkUrdfPage);
        addPage(m_assetPage);
        addPage(m_prefabMakerPage);

        connect(this, &QWizard::currentIdChanged, this, &RobotImporterWidget::onCurrentIdChanged);
        setWindowTitle("Robot Import Wizard");
        connect(
            this,
            &QDialog::finished,
            [this](int id)
            {
                AZ_Printf("page", "QDialog::finished : %d", id);
                parentWidget()->close();
                //                      AzToolsFramework::CloseViewPane("Robot Importer");
            });
    }

    void RobotImporterWidget::onCurrentIdChanged(int id)
    {
        AZ_Printf("Wizard", "Wizard at page %d", id);
        if (currentPage() == m_checkUrdfPage)
        {
            m_urdfPath = AZStd::string(m_fileSelectPage->getFileName().toUtf8().constData());
            if (!m_urdfPath.empty())
            {
                AZ_Printf("Wizard", "Testing urdf file : %s", m_urdfPath.c_str());
                m_parsedUrdf = UrdfParser::ParseFromFile(m_urdfPath);
                QString report;
                const auto log = UrdfParser::getUrdfParsingLog();
                if (m_parsedUrdf)
                {
                    report += "# The URDF was parsed and opened successfully\n";
                    // get rid of old prefab maker
                    m_prefabMaker.reset();
                    // let us skip this page
                    AZ_Printf("Wizard", "Wizard skips m_checkUrdfPage since there is no errors in URDF");
                    next();
                }
                else
                {
                    report += "# The URDF was not opened\n";
                    report += "URDF parser returned following errors:\n\n";
                }
                if (!log.empty())
                {
                    report += "`";
                    report += QString::fromUtf8(log.data(), int(log.size()));
                    report += "`";
                }
                m_checkUrdfPage->ReportURDFResult(report, m_parsedUrdf != nullptr);
            }
        }
        else if (currentPage() == m_assetPage)
        {
            m_assetPage->ClearAssetsList();
            if (m_parsedUrdf)
            {
                auto colliders_names = Utils::getMeshesFilenames(m_parsedUrdf->getRoot(), false, true);
                auto visual_names = Utils::getMeshesFilenames(m_parsedUrdf->getRoot(), true, false);
                // TODO - code is placeholder
                for (auto& str : visual_names)
                {
                    m_assetPage->ReportAsset(str, "Visual", AZ::Data::AssetId());
                }
                for (auto& str : colliders_names)
                {
                    m_assetPage->ReportAsset(str, "Collider", AZ::Data::AssetId());
                }
                if (colliders_names.size() == 0 && visual_names.size() == 0)
                {
                    AZ_Printf("Wizard", "Wizard skips m_assetPage since there is no meshes in URDF");
                    next();
                }
            }
        }
        else if (currentPage() == m_prefabMakerPage)
        {
            if (m_parsedUrdf)
            {
                AZStd::string robotName = AZStd::string(m_parsedUrdf->getName().c_str(), m_parsedUrdf->getName().size()) + ".prefab";
                m_prefabMakerPage->setProposedPrefabName(robotName);
            }
        }
    }

    void RobotImporterWidget::CreatePrefab(AZStd::string prefabName)
    {
        const AZ::IO::Path prefabPath(AZ::IO::Path(AZ::Utils::GetProjectPath()) / "Assets" / "Importer" / prefabName);
        bool fileExists = AZ::IO::FileIOBase::GetInstance()->Exists(prefabPath.c_str());
        if (fileExists)
        {
            QMessageBox msgBox;
            msgBox.setText("Prefab with this name already exists[[placeholder]]");
            msgBox.setInformativeText("Do you want to overwrite existing prefab [[placeholder]]?");
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Cancel);
            int ret = msgBox.exec();
            if (ret == QMessageBox::Cancel)
            {
                m_prefabMakerPage->setSuccess(false);
                return;
            }
        }
        m_prefabMaker = AZStd::make_unique<URDFPrefabMaker>(m_urdfPath, m_parsedUrdf, prefabPath.String());
        auto prefabOutcome = m_prefabMaker->CreatePrefabFromURDF();
        if (prefabOutcome.IsSuccess())
        {
            AZStd::string status = m_prefabMaker->getStatus();
            m_prefabMakerPage->reportProgress(status);
            m_prefabMakerPage->setSuccess(true);
        }
        else
        {
            AZStd::string status = "Failed to create prefab\n";
            status += prefabOutcome.GetError() + "\n";
            status += m_prefabMaker->getStatus();
            m_prefabMakerPage->reportProgress(status);
            m_prefabMakerPage->setSuccess(false);
        }
    }
} // namespace ROS2

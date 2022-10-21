/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "UrdfParser.h"

#include <fstream>

#include <AzCore/Debug/Trace.h>
#include <AzCore/std/string/string.h>
#include <AzCore/StringFunc/StringFunc.h>

#include <urdf_model/model.h>
#include <AssetDatabase/AssetDatabaseConnection.h>

#include <filesystem>

namespace ROS2
{
    namespace Internal
    {
        void checkIfCurrentLocaleHasDotAsADecimalSeparator()
        {
            // Due to the fact that URDF parser takes into account the locale information, incompatibility between URDF file locale and
            // system locale might lead to incorrect URDF parsing. Mainly it affects floating point numbers, and its decimal separator. When
            // locales are set to system with comma as decimal separator and URDF file is created with dot as decimal separator, URDF parser
            // will trim the floating point number after comma. For example, if parsing 0.1, URDF parser will parse it as 0.
            // This might lead to incorrect URDF loading. Most widely used separator is a dot. If in current locale it is not the case a
            // warning is presented to the user that his system's locale seem unusual, and he should double-check it

            std::locale currentLocale("");
            if (std::use_facet<std::numpunct<char>>(currentLocale).decimal_point() != '.')
            {
                AZ_Warning(
                    "UrdfParser", false, "Locale %s might be incompatible with the URDF file content.\n", currentLocale.name().c_str());
            }
        }
    } // namespace Internal

    urdf::ModelInterfaceSharedPtr UrdfParser::Parse(const AZStd::string& xmlString)
    {
        Internal::checkIfCurrentLocaleHasDotAsADecimalSeparator();
        return urdf::parseURDF(xmlString.c_str());
    }

    urdf::ModelInterfaceSharedPtr UrdfParser::ParseFromFile(const AZStd::string& filePath)
    {
        std::ifstream istream(filePath.c_str());
        if (!istream)
        {
            AZ_Error("UrdfParser", false, "File %s does not exist", filePath.c_str());
            return nullptr;
        }

        std::string xmlStr((std::istreambuf_iterator<char>(istream)), std::istreambuf_iterator<char>());
        return Parse(xmlStr.c_str());
    }

    void UrdfParser::importMeshesFromURDF(urdf::ModelInterfaceSharedPtr  urdf){

        AZStd::unordered_set<AZStd::string> meshes;
        std::function<void(urdf::LinkSharedPtr)> scanLink = [&](urdf::LinkSharedPtr link)->void{
            for (auto childLink : link->child_links)
            {

                for (auto v : childLink->visual_array)
                {
                    if (v->geometry == nullptr || v->geometry->type!= urdf::Geometry::MESH)
                    {
                        continue;
                    }
                    auto meshGeometry = std::dynamic_pointer_cast<urdf::Mesh>(v->geometry);
                    meshes.insert(AZStd::string(meshGeometry->filename.c_str(),meshGeometry->filename.size()));
                    AZ_Printf("URDF :", "%s %s", childLink->name.c_str(), meshGeometry->filename.c_str());
                }

                scanLink(childLink);
            }
        };
        std::vector<urdf::LinkSharedPtr > links;
        urdf->getLinks(links);
        for (auto childLink : links){
            scanLink(childLink);
        }

        AZStd::vector<AZStd::string> assetSafeFolders;
        bool success{false};
        AzToolsFramework::AssetSystemRequestBus::BroadcastResult(success, &AzToolsFramework::AssetSystemRequestBus::Events::GetAssetSafeFolders, assetSafeFolders);
        if (success && assetSafeFolders.size() > 0)
        {
            for (const auto& m : meshes){
                auto resolved_urdf = resolveURDFPath(m);
                auto squashed_name = squashName(m);
                auto target = (assetSafeFolders.front()+"/"+squashed_name);
                AZ_Printf("UrdfParser", "Copying meshes (%s) %s -> %s\n", m.c_str(), resolved_urdf.c_str(), (assetSafeFolders.front()+"/"+squashed_name).c_str());
                std::filesystem::copy_file(resolved_urdf.c_str(), target.c_str(),  std::filesystem::copy_options::update_existing);
                AzToolsFramework::AssetSystemRequestBus::BroadcastResult(success, &AzToolsFramework::AssetSystemRequestBus::Events::GetAssetSafeFolders, assetSafeFolders);
            }
        }
    }

    AZStd::string UrdfParser::resolveURDFPath(AZStd::string urdfPath){
        // global path
        if (urdfPath.starts_with("file:///"))
        {
            AZ::StringFunc::Replace(urdfPath, "file://", "", true, true);
            return urdfPath;
        }
        //TODO package:// and relative

        return "";
    }

    AZStd::string UrdfParser::squashName(const AZStd::string& urdfPath)
    {
        auto last_dot = AZ::StringFunc::Find(urdfPath,'/',0,true);
        auto base_name  = urdfPath.substr(last_dot+1);
        auto hash = AZ::Crc32(urdfPath.substr(0,last_dot));
        return AZStd::string::format("%09u_%s", uint32_t(hash),base_name.c_str());

    }

} // namespace ROS2

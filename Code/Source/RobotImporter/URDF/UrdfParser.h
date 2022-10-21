/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/std/string/string.h>
#include <urdf_parser/urdf_parser.h>

namespace ROS2
{
    //! Class for parsing URDF data.
    class UrdfParser
    {
    public:
        //! Parse string with URDF data and generate model.
        //! @param xmlString a string that contains URDF data (XML format).
        //! @return model represented as a tree of parsed links.
        static urdf::ModelInterfaceSharedPtr Parse(const AZStd::string& xmlString);

        //! Parse file with URDF data and generate model.
        //! @param filePath is a path to file with URDF data that will be loaded and parsed.
        //! @return model represented as a tree of parsed links.
        static urdf::ModelInterfaceSharedPtr ParseFromFile(const AZStd::string& filePath);

        //! provide simplified name (base_name + orginal path's hash - that is a name on which asset will be imported
        static AZStd::string squashName(const AZStd::string& urdfPath);

        //! finds global filename for urdf
        static AZStd::string resolveURDFPath(AZStd::string urdfPath);


        ///! allows to import meshes to assets
        static void importMeshesFromURDF(urdf::ModelInterfaceSharedPtr  urdf);

    };
} // namespace ROS2
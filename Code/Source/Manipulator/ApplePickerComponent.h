/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <AzCore/Component/TickBus.h>
#include <AzCore/Component/Component.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <AzFramework/Physics/PhysicsSystem.h>
#include <AzFramework/Physics/PhysicsScene.h>
#include <AzFramework/Physics/Shape.h>
#include <AzFramework/Physics/SystemBus.h>
#include <AzFramework/Physics/Common/PhysicsSceneQueries.h>
#include <AzFramework/Physics/Configuration/RigidBodyConfiguration.h>
#include <AzFramework/Physics/Common/PhysicsSimulatedBody.h>

namespace ROS2
{

    class ApplePickerComponent : public AZ::Component,
                                 public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(ApplePickerComponent, "{95E562FD-DDF1-46C6-B90E-C9761B04A32C}", AZ::Component);
        ApplePickerComponent();

        // AZ::Component interface implementation.
        void Activate() override;
        void Deactivate() override;
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        // Required Reflect function.
        static void Reflect(AZ::ReflectContext* context);

    private:
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        bool m_callback_registered {false};
        int m_apple_picked{0};
        AzPhysics::SimulatedBodyEvents::OnTriggerEnter::Handler m_onTriggerHandleBeginHandler;
        bool m_negative_pick{false};

    };
} // namespace ROS2

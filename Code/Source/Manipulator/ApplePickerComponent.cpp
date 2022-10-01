/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "Manipulator/ApplePickerComponent.h"
#include <AzCore/Component/Entity.h>
#include <AzCore/Debug/Trace.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzFramework/Physics/PhysicsSystem.h>
#include <AzFramework/Physics/RigidBodyBus.h>
#include <AzToolsFramework/Entity/EditorEntityHelpers.h>

#include <AtomLyIntegration/CommonFeatures/Mesh/MeshComponentBus.h>
#include <AtomLyIntegration/CommonFeatures/Mesh/MeshComponentConstants.h>
namespace ROS2
{
    ApplePickerComponent::ApplePickerComponent()
    {

        m_onTriggerHandleBeginHandler = AzPhysics::SimulatedBodyEvents::OnTriggerEnter::Handler([&]([[maybe_unused]] AzPhysics::SimulatedBodyHandle bodyHandle,[[maybe_unused]] const AzPhysics::TriggerEvent& event){

            const AZ::EntityId& e1 = event.m_otherBody->GetEntityId();
            const AZ::EntityId& e2 = event.m_triggerBody->GetEntityId();
            const AZ::EntityId& collideToEntityId = this->GetEntityId() == e1 ? e2 : e1;
            //AzPhysics::SimulatedBody* collideToEntityId = this->GetEntityId() == e1 ?  event.m_triggerBody : event.m_otherBody;

            bool is_visible = false;

            const bool apple_should_disapper = !m_negative_pick;
            if (apple_should_disapper)
            {
                AZ::Render::MeshComponentRequestBus::EventResult(is_visible, collideToEntityId,
                                                                 &AZ::Render::MeshComponentRequests::GetVisibility);
                if (is_visible) {
                    AZ::Render::MeshComponentRequestBus::Event(collideToEntityId,
                                                               &AZ::Render::MeshComponentRequests::SetVisibility,
                                                               false);
                    this->m_apple_picked++;
                }
            }

            if (m_negative_pick)
            {
                Physics::RigidBodyRequestBus::Event(collideToEntityId, &Physics::RigidBodyRequests::SetGravityEnabled, true);
                Physics::RigidBodyRequestBus::Event(collideToEntityId, &Physics::RigidBodyRequests::SetKinematic, false);


            }

AZ_Printf("m_onTriggerHandleBeginHandler", "m_onTriggerHandleBeginHandler %s %s %s\n", e1.ToString().c_str(), e2.ToString().c_str(), this->GetEntityId().ToString().c_str());
                                      });

    }
    void ApplePickerComponent::Activate()
    {
        m_callback_registered = false;
        AZ::TickBus::Handler::BusConnect();
    }

    void ApplePickerComponent::Deactivate()
    {
    }

    void ApplePickerComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<ApplePickerComponent, AZ::Component>()->Version(1)
                    ->Field("NegativePicker", &ApplePickerComponent::m_negative_pick);

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<ApplePickerComponent>("ApplePickerComponent", "ApplePickerComponent")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "ROS2")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"))
                    ->DataElement(AZ::Edit::UIHandlers::Default, &ApplePickerComponent::m_negative_pick, "NegativePicker", "NegativePicker");
            }
        }
    }

    void ApplePickerComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        //       required.push_back(AZ_CRC("VehicleModelService"));
    }

    void ApplePickerComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        // provided.push_back(AZ_CRC_CE("ROS2RobotControl"));
    }
    void ApplePickerComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time){

        AZ_Printf("apple_score", "m_apple_picked : %d \n", m_apple_picked);
        if (!m_callback_registered)
        {

            auto* physicsSystem = AZ::Interface<AzPhysics::SystemInterface>::Get();
            auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();
            auto [physicScene, physicBody]  = physicsSystem->FindAttachedBodyHandleFromEntityId(GetEntityId());
            if (physicBody != AzPhysics::InvalidSimulatedBodyHandle && physicScene != AzPhysics::InvalidSceneHandle)
            {
                AzPhysics::SimulatedBody* simulated_body = sceneInterface->GetSimulatedBodyFromHandle(physicScene, physicBody);
                simulated_body->RegisterOnTriggerEnterHandler(m_onTriggerHandleBeginHandler);
                AZ_Printf("TICK-REG", " %f %f %f\n", simulated_body->GetPosition().GetX(),simulated_body->GetPosition().GetY(),simulated_body->GetPosition().GetZ());
                m_callback_registered = true;
            }
        }


    }
} // namespace ROS2

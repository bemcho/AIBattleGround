//
// Created by bemcho on 15.01.18.
//

#ifndef AIBATTLEGROUND_EPISODE_HPP
#define AIBATTLEGROUND_EPISODE_HPP
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Skybox.h>
#include <Urho3D/Graphics/Terrain.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/Text.h>
#include "../Base/AIBattleGround.hpp"

class Episode {
    // Enable type information.
 public:
    virtual Urho3D::Scene *InitScene()=0;
    virtual void InitObjects()=0;
    virtual Urho3D::SharedPtr<Urho3D::Node> InitCamera()=0;
    virtual void InitViewPort()=0;
    virtual void MoveCamera(float timeStep)=0;
    /// Handle the logic update event.
    virtual void HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)=0;
    virtual void SubscribeToEvents()=0;
    virtual void HandlePostRenderUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)=0;
    virtual void CreateInstructions()=0;

 protected:
    /// Reflection camera scene node.
    Urho3D::SharedPtr<Urho3D::Node> reflectionCameraNode_;
    /// Instruction text UI-element.
    Urho3D::Text* instructionText_;
    //Drone camera
    Urho3D::SharedPtr<Urho3D::Node> rttCameraNode_;

    //Drone screen
    Urho3D::SharedPtr<Urho3D::Node> screenBox_;

    Urho3D::SharedPtr<Urho3D::Node> screenNode_;
};

// All Urho3D classes reside in namespace Urho3D

#endif //AIBATTLEGROUND_EPISODE_HPP

//
// Copyright (c) 2008-2018 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/RenderSurface.h>
#include <Urho3D/Graphics/Skybox.h>
#include <Urho3D/Graphics/Terrain.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/File.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>

#include "AIBattleGroundApp.hpp"
#include "Source/Intro/Intro.hpp"
#include <Urho3D/DebugNew.h>
using namespace Urho3D;

URHO3D_DEFINE_APPLICATION_MAIN(AIBattleGroundApp)

AIBattleGroundApp::AIBattleGroundApp(Context *context) :
  AIBattleGround(context) {
    currentEpisode_ = std::make_shared<Intro>(context);
}

void AIBattleGroundApp::Start() {
    // Execute base class startup
    AIBattleGround::Start();

    // Create the scene content
    CreateScene();

    // Create the UI content
    CreateInstructions();

    // Setup the viewport for displaying the scene
    SetupViewport();

    // Hook up to the frame update event
    SubscribeToEvents();

    // Set the mouse mode to use in the AIBattleGround
    AIBattleGround::InitMouseMode(MM_RELATIVE);
}

void AIBattleGroundApp::CreateScene() {

    cameraNode_ = currentEpisode_->InitCamera();
    scene_ = currentEpisode_->InitScene();
    currentEpisode_->InitObjects();
}

void AIBattleGroundApp::CreateInstructions() {
  currentEpisode_->CreateInstructions();
}

void AIBattleGroundApp::SetupViewport() {
    currentEpisode_->InitViewPort();
}

void AIBattleGroundApp::SubscribeToEvents() {
    // Subscribe HandleUpdate() function for processing update events
    // currentEpisode_->SubscribeToEvents();
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(AIBattleGroundApp, HandleUpdate));
}

void AIBattleGroundApp::MoveCamera(float timeStep) {
}

void AIBattleGroundApp::HandleUpdate(StringHash eventType, VariantMap &eventData) {
    currentEpisode_->HandleUpdate(eventType, eventData);
}

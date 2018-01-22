//
// Created by bemcho on 15.01.18.
//
#include <vector>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/Animation.h>
#include <Urho3D/Graphics/AnimationState.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Graphics/Technique.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/IO/File.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>

#include "Intro.hpp"
#include "Mover.h"
#include "DroneMover.h"

using namespace Urho3D;
Intro::Intro(Urho3D::Context *context) : AIBattleGround(context) {

    // Register an object factory for our custom Mover component so that we can create them to scene nodes
    context->RegisterFactory<Mover>();
    context->RegisterFactory<DroneMover>();
}
Intro::~Intro() {}

Scene *Intro::InitScene() {
    auto *cache = GetSubsystem<ResourceCache>();

    scene_ = new Scene(context_);

    // Create octree, use default volume (-1000, -1000, -1000) to (1000, 1000, 1000)
    scene_->CreateComponent<Octree>();
    scene_->CreateComponent<PhysicsWorld>();

    // Create a Zone component for ambient lighting & fog control
    Node *zoneNode = scene_->CreateChild("Zone");
    auto *zone = zoneNode->CreateComponent<Zone>();
    zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
    zone->SetAmbientColor(Color(0.15f, 0.15f, 0.15f));
    zone->SetFogColor(Color(0.5f, 0.5f, 0.7f));
    zone->SetFogStart(800.0f);
    zone->SetFogEnd(1000.0f);

    // Create a directional light to the world. Enable cascaded shadows on it
    Node *lightNode = scene_->CreateChild("DirectionalLight");
    lightNode->SetDirection(Vector3(0.6f, -1.0f, 0.8f));
    auto *light = lightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);
    light->SetCastShadows(true);
    light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
    light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));
    light->SetSpecularIntensity(0.5f);
    // Apply slightly overbright lighting to match the skybox
    light->SetColor(Color(1.2f, 1.2f, 1.2f));

    // Create skybox. The Skybox component is used like StaticModel, but it will be always located at the camera, giving the
    // illusion of the box planes being far away. Use just the ordinary Box model and a suitable material, whose shader will
    // generate the necessary 3D texture coordinates for cube mapping
    Node *skyNode = scene_->CreateChild("Sky");
    skyNode->SetScale(500.0f); // The scale actually does not matter
    auto *skybox = skyNode->CreateComponent<Skybox>();
    skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
    skybox->SetMaterial(cache->GetResource<Material>("Materials/Skybox.xml"));

    // Create heightmap terrain with collision
    Node *terrainNode = scene_->CreateChild("Terrain");
    terrainNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    auto *terrain = terrainNode->CreateComponent<Terrain>();
    terrain->SetPatchSize(64);
    terrain->SetSpacing(Vector3(3.0f,
                                0.4f,
                                3.0f)); // Spacing between vertices and vertical resolution of the height map
    terrain->SetSmoothing(true);
    terrain->SetHeightMap(cache->GetResource<Image>("Textures/HeightMap.png"));
    terrain->SetMaterial(cache->GetResource<Material>("Materials/Terrain.xml"));
    // The terrain consists of large triangles, which fits well for occlusion rendering, as a hill can occlude all
    // terrain patches and other objects behind it
    terrain->SetOccluder(true);
    auto *terrainBody = terrainNode->CreateComponent<RigidBody>();
    terrainBody->SetCollisionLayer(2); // Use layer bitmask 2 for static geometry
    auto *terrainS =
      terrainNode->CreateComponent<CollisionShape>();
    terrainS->SetTerrain();

    const float boundsXY = 700.0f;

    // Create cylinders of varying sizes
    CreateObjects("Cylinder", "Models/Cylinder.mdl", "Materials/RibbonTrail.xml", 100, scene_, 100, boundsXY);

    // Create cones of varying sizes
    CreateObjects("Cone", "Models/Cone.mdl", "Materials/Mushroom.xml", 100, scene_, 300, boundsXY);

    // Create cones of varying sizes
    CreateObjects("Torus", "Models/Torus.mdl", "Materials/Water.xml", 100, scene_, 150, boundsXY);

    // Create mushrooms of varying sizes
    CreateObjects("Mushroom", "Models/Mushroom.mdl", "Materials/Mushroom.xml", 100, scene_, 1000, boundsXY);

    //Create boxes of varying sizes
    CreateObjects("Box", "Models/Box.mdl", "Materials/Particle.xml", 300, scene_, 30, boundsXY);

    //Create balls of varying sizes
    CreateObjects("Sphere", "Models/Sphere.mdl", "Materials/Stone.xml", 300, scene_, 10, boundsXY);

    // Create a water plane object that is as large as the terrain
    waterNode_ = scene_->CreateChild("AIBattleGroundApp");
    waterNode_->SetScale(Vector3(2048.0f, 1.0f, 2048.0f));
    waterNode_->SetPosition(Vector3(0.0f, 5.0f, 0.0f));
    auto *water = waterNode_->CreateComponent<StaticModel>();
    water->SetModel(cache->GetResource<Model>("Models/Plane.mdl"));
    water->SetMaterial(cache->GetResource<Material>("Materials/Water.xml"));
    // Set a different viewmask on the water plane to be able to hide it from the reflection camera
    water->SetViewMask(0x80000000);

    // Create a camera for the render-to-texture scene. Simply leave it at the world origin and let it observe the scene
    rttCameraNode_ = scene_->CreateChild("Camera");
    auto *camera = rttCameraNode_->CreateComponent<Camera>();
    camera->SetFarClip(600.0f);
    rttCameraNode_->SetPosition(Vector3(0.0f, 300.0f, -20.0f));
    rttCameraNode_->SetRotation(cameraNode_->GetRotation());

    {
        screenBox_ = scene_->CreateChild("ScreenBox");
        screenBox_->SetPosition(Vector3(0.0f, 50.0f, 0.0f));
        screenBox_->SetRotation(Quaternion(0.0f, 180.0f, 0.0f));
        screenBox_->SetScale(Vector3(21.0f, 16.0f, 0.5f));
        auto *boxObject = screenBox_->CreateComponent<StaticModel>();
        boxObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
        boxObject->SetMaterial(cache->GetResource<Material>("Materials/Water.xml"));

        screenNode_ = scene_->CreateChild("Screen");
        screenNode_->SetPosition(Vector3(0.0f, 50.0f, -0.27f));
        screenNode_->SetRotation(Quaternion(90.0f, 180.0f, 0.0f));
        screenNode_->SetScale(Vector3(20.0f, 0.0f, 15.0f));
        auto *screenObject = screenNode_->CreateComponent<StaticModel>();
        screenObject->SetModel(cache->GetResource<Model>("Models/Plane.mdl"));

        // Create a renderable texture (1024x768, RGB format), enable bilinear filtering on it
        SharedPtr<Texture2D> renderTexture(new Texture2D(context_));
        renderTexture->SetSize(1024, 768, Graphics::GetRGBFormat(), TEXTURE_RENDERTARGET);
        renderTexture->SetFilterMode(FILTER_BILINEAR);

        // Create a new material from scratch, use the diffuse unlit technique, assign the render texture
        // as its diffuse texture, then assign the material to the screen plane object
        SharedPtr<Material> renderMaterial(new Material(context_));
        renderMaterial->SetTechnique(0, cache->GetResource<Technique>("Techniques/DiffUnlit.xml"));
        renderMaterial->SetTexture(TU_DIFFUSE, renderTexture);
        // Since the screen material is on top of the box model and may Z-fight, use negative depth bias
        // to push it forward (particularly necessary on mobiles with possibly less Z resolution)
        renderMaterial->SetDepthBias(BiasParameters(-0.001f, 0.0f));
        screenObject->SetMaterial(renderMaterial);

        // Get the texture's RenderSurface object (exists when the texture has been created in rendertarget mode)
        // and define the viewport for rendering the second scene, similarly as how backbuffer viewports are defined
        // to the Renderer subsystem. By default the texture viewport will be updated when the texture is visible
        // in the main view
        RenderSurface *surface = renderTexture->GetRenderSurface();
        SharedPtr<Viewport> rttViewport(new Viewport(context_, scene_, rttCameraNode_->GetComponent<Camera>()));
        surface->SetViewport(0, rttViewport);
    }

    return scene_;
}

void Intro::CreateObjects(const String modelName,
                          const String modelPath,
                          const String materialPath,
                          const unsigned objectsCount,
                          Scene *scene,
                          const float massScalar,
                          const float boundsXY) {
    auto *cache = GetSubsystem<ResourceCache>();

    for (unsigned j = 0; j < objectsCount; ++j) {
        const float scale = Random(1, 10) + 0.5f;
        Node *boxNode = scene_->CreateChild(modelName);
        boxNode->SetPosition(Vector3(Random(boundsXY), 100.0f, Random(boundsXY)));
        boxNode->SetRotation(Quaternion(Random(360.0f), Random(360.0f), Random(360.0f)));
        boxNode->SetScale(scale);
        auto *boxObject = boxNode->CreateComponent<StaticModel>();
        boxObject->SetModel(cache->GetResource<Model>(modelPath));
        boxObject->SetMaterial(cache->GetResource<Material>(materialPath));
        boxObject->SetCastShadows(true);

        auto *body = boxNode->CreateComponent<RigidBody>();
        body->SetMass(scale*massScalar);

        auto *shape = boxNode->CreateComponent<CollisionShape>();
        if (modelName.Find("Sphere")!=String::NPOS) {
            body->SetRollingFriction(1.0f);
            shape->SetSphere(1.0f);
        } else {
            shape->SetBox(Vector3::ONE);
        }

    }
}
void Intro::InitObjects() {

    auto *cache = GetSubsystem<ResourceCache>();
    // Create animated models
    const unsigned NUM_MODELS = 700;
    const float MODEL_MOVE_SPEED = 15.0f;
    const float MODEL_ROTATE_SPEED = 100.0f;
    const float x_bound = 1000.0f;
    const float y_bound = 1000.0f;
    const BoundingBox bounds(Vector3(-x_bound, 0.0f, -y_bound), Vector3(x_bound, 0.0f, y_bound));

    std::vector<std::tuple<String, String, String>> animations{
      std::make_tuple("Models/Mutant/Mutant.mdl",
                      "Models/Mutant/Mutant_Run.ani",
                      "Models/Mutant/Materials/mutant_M.xml"),
      std::make_tuple("Models/X_Bot/X_Bot.mdl",
                      "Models/X_Bot/X_Bot_Run.ani",
                      "Models/X_Bot/Materials/X_BotSurface.xml"),
      std::make_tuple("Models/X_Bot/X_Bot.mdl",
                      "Models/X_Bot/X_Bot_Run2.ani",
                      "Models/X_Bot/Materials/X_BotSurface.xml"),
      std::make_tuple("Models/Swat/Swat.mdl", "Models/Swat/Swat_SprintFwd.ani", "Models/Mutant/Materials/mutant_M.xml"),
      std::make_tuple("Models/Mutant/Mutant.mdl",
                      "Models/Mutant/Mutant_Jump.ani",
                      "Models/Mutant/Materials/mutant_M.xml")};
    for (unsigned i = 0; i < NUM_MODELS; ++i) {
        const float scaleWeight = Random(1, 10);
        Node *modelNode = scene_->CreateChild("Jack");
        modelNode->SetPosition(Vector3(Random(x_bound/2.0f), 100.f, Random(y_bound/2.0f)));
        modelNode->SetRotation(Quaternion(0.0f, Random(360.0f), 0.0f));
        modelNode->SetScale(scaleWeight);
        // spin node
        Node *adjustNode = modelNode->CreateChild("AdjNode");
        adjustNode->SetRotation(Quaternion(180, Vector3(0, 1, 0)));

        auto *modelObject = adjustNode->CreateComponent<AnimatedModel>();

        auto & [modelPath, walkAnimationPath, materialPath] = animations[Random(4)];
        auto *walkAnimation = cache->GetResource<Animation>(walkAnimationPath);
        modelObject->SetModel(cache->GetResource<Model>(modelPath));
        modelObject->SetMaterial(cache->GetResource<Material>(materialPath));
        modelObject->SetCastShadows(true);
        AnimationState *state = modelObject->AddAnimationState(walkAnimation);
        // The state would fail to create (return null) if the animation was not found
        if (state) {
            // Enable full blending weight and looping
            state->SetWeight(1.0f);
            state->SetLooped(true);
            state->SetTime(Random(walkAnimation->GetLength()));
        }

        // Create our custom Mover component that will move & animate the model during each frame's update
        auto *mover = modelNode->CreateComponent<Mover>();
        mover->SetParameters(MODEL_MOVE_SPEED - (scaleWeight/4.0f), MODEL_ROTATE_SPEED, bounds);
        // Create rigidbody, and set non-zero mass so that the body becomes dynamic
        auto *body = modelNode->CreateComponent<RigidBody>();
        body->SetCollisionLayer(1);
        body->SetMass(scaleWeight*100);

        // Set zero angular factor so that physics doesn't turn the character on its own.
        // Instead we will control the character yaw manually
        body->SetAngularFactor(Vector3::ZERO);

        // Set the rigidbody to signal collision also when in rest, so that we get ground collisions properly
        body->SetCollisionEventMode(COLLISION_ALWAYS);

        // Set a capsule shape for collision
        auto *shape = modelNode->CreateComponent<CollisionShape>();
        shape->SetCapsule(0.7f, 1.8f, Vector3(0.0f, 0.9f, 0.0f));

    }

}
Urho3D::SharedPtr<Urho3D::Node> Intro::InitCamera() {
    // Create the camera. Set far clip to match the fog. Note: now we actually create the camera node outside
    // the scene, because we want it to be unaffected by scene load / save
    cameraNode_ = new Node(context_);
    auto *camera = cameraNode_->CreateComponent<Camera>();
    camera->SetFarClip(1000.0f);

    // Set an initial position for the camera scene node above the ground
    cameraNode_->SetPosition(Vector3(0.0f, 50.0f, 50.0f));
    return cameraNode_;
}
void Intro::InitViewPort() {
    auto *graphics = GetSubsystem<Graphics>();
    auto *renderer = GetSubsystem<Renderer>();
    auto *cache = GetSubsystem<ResourceCache>();

    // Set up a viewport to the Renderer subsystem so that the 3D scene can be seen
    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
    renderer->SetViewport(0, viewport);

    // Create a mathematical plane to represent the water in calculations
    waterPlane_ = Plane(waterNode_->GetWorldRotation()*Vector3(0.0f, 1.0f, 0.0f), waterNode_->GetWorldPosition());
    // Create a downward biased plane for reflection view clipping. Biasing is necessary to avoid too aggressive clipping
    waterClipPlane_ = Plane(waterNode_->GetWorldRotation()*Vector3(0.0f, 1.0f, 0.0f), waterNode_->GetWorldPosition() -
      Vector3(0.0f, 0.1f, 0.0f));

    // Create camera for water reflection
    // It will have the same farclip and position as the main viewport camera, but uses a reflection plane to modify
    // its position when rendering
    reflectionCameraNode_ = cameraNode_->CreateChild();
    auto *reflectionCamera = reflectionCameraNode_->CreateComponent<Camera>();
    reflectionCamera->SetFarClip(750.0);
    reflectionCamera->SetViewMask(0x7fffffff); // Hide objects with only bit 31 in the viewmask (the water plane)
    reflectionCamera->SetAutoAspectRatio(false);
    reflectionCamera->SetUseReflection(true);
    reflectionCamera->SetReflectionPlane(waterPlane_);
    reflectionCamera->SetUseClipping(true); // Enable clipping of geometry behind water plane
    reflectionCamera->SetClipPlane(waterClipPlane_);
    // The water reflection texture is rectangular. Set reflection camera aspect ratio to match
    reflectionCamera->SetAspectRatio((float) graphics->GetWidth()/(float) graphics->GetHeight());
    // View override flags could be used to optimize reflection rendering. For example disable shadows
    //reflectionCamera->SetViewOverrideFlags(VO_DISABLE_SHADOWS);

    // Create a texture and setup viewport for water reflection. Assign the reflection texture to the diffuse
    // texture unit of the water material
    int texSize = 1024;
    SharedPtr<Texture2D> renderTexture(new Texture2D(context_));
    renderTexture->SetSize(texSize, texSize, Graphics::GetRGBFormat(), TEXTURE_RENDERTARGET);
    renderTexture->SetFilterMode(FILTER_BILINEAR);
    RenderSurface *surface = renderTexture->GetRenderSurface();
    SharedPtr<Viewport> rttViewport(new Viewport(context_, scene_, reflectionCamera));
    surface->SetViewport(0, rttViewport);
    auto *waterMat = cache->GetResource<Material>("Materials/Water.xml");
    waterMat->SetTexture(TU_DIFFUSE, renderTexture);
}
void Intro::HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData) {
    using namespace Update;

    // Take the frame time step, which is stored as a float
    float timeStep = eventData[P_TIMESTEP].GetFloat();

    // Move the camera, scale movement with time step
    MoveCamera(timeStep);
}
void Intro::MoveCamera(float timeStep) {
    // Do not move if the UI has a focused element (the console)
    if (GetSubsystem<Urho3D::UI>()->GetFocusElement())
        return;

    auto *input = GetSubsystem<Input>();

    // Movement speed as world units per second
    const float MOVE_SPEED = 60.0f;
    // Mouse sensitivity as degrees per pixel
    const float MOUSE_SENSITIVITY = 0.1f;

    // Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
    IntVector2 mouseMove = input->GetMouseMove();
    yaw_ += MOUSE_SENSITIVITY*mouseMove.x_;
    pitch_ += MOUSE_SENSITIVITY*mouseMove.y_;
    pitch_ = Clamp(pitch_, -90.0f, 90.0f);

    // Construct new orientation for the camera scene node from yaw and pitch. Roll is fixed to zero
    cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));

    // Read WASD keys and move the camera scene node to the corresponding direction if they are pressed
    if (input->GetKeyDown(KEY_W))
        cameraNode_->Translate(Vector3::FORWARD*MOVE_SPEED*timeStep);
    if (input->GetKeyDown(KEY_S))
        cameraNode_->Translate(Vector3::BACK*MOVE_SPEED*timeStep);
    if (input->GetKeyDown(KEY_A))
        cameraNode_->Translate(Vector3::LEFT*MOVE_SPEED*timeStep);
    if (input->GetKeyDown(KEY_D))
        cameraNode_->Translate(Vector3::RIGHT*MOVE_SPEED*timeStep);

    // "Shoot" a physics object with left mousebutton
    if (input->GetMouseButtonPress(MOUSEB_LEFT)) {
        SpawnObject();
    } // Set destination or spawn a new jack with left mouse button
    else if (input->GetMouseButtonPress(MOUSEB_MIDDLE) || input->GetKeyPress(KEY_O)) {
        SpawnDrone();
    }
        // Check for loading/saving the scene from/to the file Data/Scenes/CrowdNavigation.xml relative to the executable directory
    else if (input->GetKeyPress(KEY_F5)) {
        File saveFile
          (context_, GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Scenes/AIBattleGround.xml", FILE_WRITE);
        scene_->SaveXML(saveFile);
    } else if (input->GetKeyPress(KEY_F7)) {
        File
          loadFile(context_, GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Scenes/AIBattleGround.xml", FILE_READ);
        scene_->LoadXML(loadFile);
    }
        // Toggle instruction text with F12
    else if (input->GetKeyPress(KEY_F12)) {
        if (instructionText_)
            instructionText_->SetVisible(!instructionText_->IsVisible());
    }
        // face towards controll display
    else if (input->GetMouseButtonPress(MOUSEB_RIGHT)) {

        const auto &camPos = cameraNode_->GetPosition();
        const auto &&screenNodePos = Vector3(0.0f, 0.0f, -0.27f);

        screenBox_->SetPosition(camPos);
        cameraNode_->Translate(Vector3::BACK*MOVE_SPEED*timeStep*5);

        screenNode_->SetPosition(screenBox_->GetPosition() + screenNodePos);
    }
    // In case resolution has changed, adjust the reflection camera aspect ratio
    auto *graphics = GetSubsystem<Graphics>();
    auto *reflectionCamera = reflectionCameraNode_->GetComponent<Camera>();
    reflectionCamera->SetAspectRatio((float) graphics->GetWidth()/(float) graphics->GetHeight());
}
void Intro::SubscribeToEvents() {
    // Subscribe HandleUpdate() function for processing update events
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Intro, HandleUpdate));

    // Subscribe HandlePostRenderUpdate() function for processing the post-render update event, sent after Renderer subsystem is
    // done with defining the draw calls for the viewports (but before actually executing them.) We will request debug geometry
    // rendering during that event
    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(Intro, HandlePostRenderUpdate));
}
void Intro::HandlePostRenderUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData) {
    // If draw debug mode is enabled, draw viewport debug geometry, which will show eg. drawable bounding boxes and skeleton
    // bones. Note that debug geometry has to be separately requested each frame. Disable depth test so that we can see the
    // bones properly
}
void Intro::SpawnObject() {

    auto *cache = GetSubsystem<ResourceCache>();
    const float scale = Random(1, 7) + 0.5f;
    Node *boxNode = scene_->CreateChild("Sphere");
    boxNode->SetPosition(cameraNode_->GetPosition());
    boxNode->SetRotation(cameraNode_->GetRotation());
    boxNode->SetScale(scale);
    auto *boxObject = boxNode->CreateComponent<StaticModel>();
    boxObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
    boxObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
    boxObject->SetCastShadows(true);

    auto *body = boxNode->CreateComponent<RigidBody>();
    body->SetMass(scale*50.0f);
    body->SetRollingFriction(1.0f);
    auto *shape = boxNode->CreateComponent<CollisionShape>();
    shape->SetSphere(1.0f);

    const float OBJECT_VELOCITY = 70.0f;

    // Set initial velocity for the RigidBody based on camera forward vector. Add also a slight up component
    // to overcome gravity better
    body->SetLinearVelocity(cameraNode_->GetRotation()*Vector3(0.0f, 0.25f, 1.0f)*(OBJECT_VELOCITY));

}

void Intro::SpawnDrone() {

    auto *cache = GetSubsystem<ResourceCache>();
    const float MODEL_MOVE_SPEED = 30.0f;
    const float MODEL_ROTATE_SPEED =200.0f;
    const float x_bound = 900.0f;
    const float y_bound = 900.0f;

    const BoundingBox bounds(Vector3(-x_bound, 0.0f, -y_bound), Vector3(x_bound, 0.0f, y_bound));

    Node *boxNode = scene_->CreateChild("MQ9");
    boxNode->SetPosition(cameraNode_->GetPosition());
    boxNode->SetScale(3);

    auto *boxObject = boxNode->CreateComponent<StaticModel>();
    boxObject->SetModel(cache->GetResource<Model>("Models/MQ_9/MQ_9.mdl"));
    boxObject->SetMaterial(cache->GetResource<Material>("Materials/Mutant/Materials/mutant_M.xml"));
    boxObject->SetCastShadows(true);

    // Create our custom Mover component that will move & animate the model during each frame's update
    auto *mover = boxNode->CreateComponent<DroneMover>();
    mover->SetParameters(MODEL_MOVE_SPEED, MODEL_ROTATE_SPEED, bounds, rttCameraNode_);

    auto *body = boxNode->CreateComponent<RigidBody>();
    body->SetMass(10.0f);

    // Set zero angular factor so that physics doesn't turn the character on its own.
    // Instead we will control the character yaw manually
    body->SetAngularFactor(Vector3::ZERO);

    // Set the rigidbody to signal collision also when in rest, so that we get ground collisions properly
    body->SetCollisionEventMode(COLLISION_ALWAYS);
    auto *shape = boxNode->CreateComponent<CollisionShape>();
    shape->SetCapsule(3.7f, 3.8f, Vector3(0.0f, 0.9f, 0.0f));

}
void Intro::CreateInstructions() {
    auto *cache = GetSubsystem<ResourceCache>();
    auto *ui = GetSubsystem<UI>();

    // Construct new Text object, set string to display and font to use
    instructionText_ = ui->GetRoot()->CreateChild<Text>();
    instructionText_->SetText(
      "Use WASD keys to move, RMB to rotate view\n"
        "LMB to spawn ball object, SHIFT+LMB to spawn a MQ9 Reaper drone\n"
        "RMB to go back and face the drone control display\n"
        "F5 to save scene, F7 to load\n"
        "F12 to toggle this instruction text"
    );
    instructionText_->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);
    // The text has multiple rows. Center them in relation to each other
    instructionText_->SetTextAlignment(HA_CENTER);

    // Position the text relative to the screen center
    instructionText_->SetHorizontalAlignment(HA_CENTER);
    instructionText_->SetVerticalAlignment(VA_CENTER);
    instructionText_->SetPosition(0, ui->GetRoot()->GetHeight()/4);

}


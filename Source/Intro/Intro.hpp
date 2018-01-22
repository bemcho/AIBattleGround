//
// Created by bemcho on 15.01.18.
//

#ifndef AIBATTLEGROUND_INTRO_HPP
#define AIBATTLEGROUND_INTRO_HPP

#include "../Base/Episode.hpp"
class Intro : public Episode , public AIBattleGround{
    // Enable type information.
 URHO3D_OBJECT(Intro, AIBattleGround)
 public:
    Intro(Urho3D::Context* context);
    virtual ~Intro();

    Urho3D::Scene *InitScene() override;
    void InitObjects() override;
    Urho3D::SharedPtr<Urho3D::Node> InitCamera() override;
    void InitViewPort() override;
    /// Handle the logic update event.
    void HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData) override;
    void MoveCamera(float timeStep) override;
    void SubscribeToEvents() override;
    void HandlePostRenderUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData) override;
    void CreateInstructions() override;

    /// Spawn a physics object from the camera position.
    void SpawnObject();

 private:

    /// Water body scene node.
    Urho3D::SharedPtr<Urho3D::Node> waterNode_;
    /// Reflection plane representing the water surface.
    Urho3D::Plane waterPlane_;
    /// Clipping plane for reflection rendering. Slightly biased downward from the reflection plane to avoid artifacts.
    Urho3D::Plane waterClipPlane_;

    void CreateObjects(const Urho3D::String modelName,
                       const Urho3D::String modelPath,
                       const Urho3D::String materialPath,
                       const unsigned int objectsCount,
                       Urho3D::Scene *scene,
                       const float massScalar,
                       const float boundsXY);
    void SpawnDrone();
};

#endif //AIBATTLEGROUND_INTRO_HPP

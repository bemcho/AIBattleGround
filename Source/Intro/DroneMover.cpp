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

#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationState.h>
#include <Urho3D/Scene/Scene.h>

#include "DroneMover.h"

DroneMover::DroneMover(Context *context) :
  LogicComponent(context),
  moveSpeed_(0.0f),
  rotationSpeed_(0.0f) {
    // Only the scene update event is needed: unsubscribe from the rest for optimization
    SetUpdateEventMask(USE_UPDATE);
}

void DroneMover::SetParameters(float moveSpeed, float rotationSpeed, const BoundingBox &bounds, Node *camera) {
    moveSpeed_ = moveSpeed;
    rotationSpeed_ = rotationSpeed;
    bounds_ = bounds;
    camera_ = camera;
}

void DroneMover::Update(float timeStep) {
    node_->Translate(Vector3::FORWARD*moveSpeed_*timeStep);

    Vector3 pos = node_->GetPosition();
    node_->SetPosition(Vector3(pos.x_, 200.0f, pos.z_));
//    if (pos.y_ < 400.f) {
//        node_->SetPosition(Vector3(pos.x_, pos.y_ + 0.5f, pos.z_));
//    } else {
//        node_->SetPosition(Vector3(pos.x_, 400.0f, pos.z_));
//    }
    // If in risk of going outside the plane, rotate the model right
    if (pos.x_ < bounds_.min_.x_
      || pos.x_ > bounds_.max_.x_
      || pos.z_ < bounds_.min_.z_
      || pos.z_ > bounds_.max_.z_) {
        node_->Yaw(rotationSpeed_*timeStep);
    }

    camera_->SetPosition(node_->GetPosition());
    camera_->SetRotation(node_->GetRotation().Inverse());
    camera_->LookAt(Vector3(0.0f, 0.0f, 0.0f), Vector3::DOWN, TransformSpace::TS_WORLD);
    // Get the model's first (only) animation state and advance its time. Note the convenience accessor to other components
    // in the same scene node
    auto *model = node_->GetComponent<AnimatedModel>(true);
    if (model && model->GetNumAnimationStates()) {
        AnimationState *state = model->GetAnimationStates()[0];
        state->AddTime(timeStep);
    }
}

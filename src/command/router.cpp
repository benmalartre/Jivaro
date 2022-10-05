//
// Copyright 2017 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//

#include "../command/router.h"

#include <pxr/base/tf/instantiateSingleton.h>
#include <pxr/base/tf/refPtr.h>

#include "../command//block.h"
#include "../command/delegate.h"


PXR_NAMESPACE_USING_DIRECTIVE 
TF_INSTANTIATE_SINGLETON(AMN::UndoRouter);

JVR_NAMESPACE_OPEN_SCOPE

UndoRouter::UndoRouter() {
    // TfDebug::Enable(USDQT_DEBUG_UNDOSTACK);
}

bool UndoRouter::TrackLayer(const SdfLayerHandle& layer) {
    layer->SetStateDelegate(UndoStateDelegate::New());
    return true;
}

void UndoRouter::AddInverse(std::function<bool()> inverse) {
    _inversion._Append(inverse);
}

UndoRouter& UndoRouter::Get() {
    return pxr::TfSingleton<UndoRouter>::GetInstance();
}

bool UndoRouter::TransferEdits(UndoInverse* inverse){
    inverse->_Adopt(Get()._inversion);
    Get()._inversion._Clear();
    return true;
}

void UndoRouter::_Mute(){
    Get()._muteDepth++;
}

void UndoRouter::_Unmute(){
    Get()._muteDepth--;
    if (Get()._muteDepth < 0){
        TF_CODING_ERROR("Mute depth error.");
    }
}

bool UndoRouter::IsMuted(){
    return Get()._muteDepth > 0;
}

JVR_NAMESPACE_CLOSE_SCOPE
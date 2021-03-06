//
//  ClipboardScriptingInterface.cpp
//  interface/src/scripting
//
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "Application.h"
#include "ClipboardScriptingInterface.h"

ClipboardScriptingInterface::ClipboardScriptingInterface() {
}

float ClipboardScriptingInterface::getClipboardContentsLargestDimension() {
    return Application::getInstance()->getEntityClipboard()->getContentsLargestDimension();
}

bool ClipboardScriptingInterface::exportEntities(const QString& filename, const QVector<EntityItemID>& entityIDs) {
    return Application::getInstance()->exportEntities(filename, entityIDs);
}

bool ClipboardScriptingInterface::exportEntities(const QString& filename, float x, float y, float z, float s) {
    return Application::getInstance()->exportEntities(filename, x, y, z, s);
}

bool ClipboardScriptingInterface::importEntities(const QString& filename) {
    return Application::getInstance()->importEntities(filename);
}

QVector<EntityItemID> ClipboardScriptingInterface::pasteEntities(glm::vec3 position) {
    return Application::getInstance()->pasteEntities(position.x, position.y, position.z);
}

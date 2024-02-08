#pragma once

#include "clip.h"
#include "skeleton.h"

Pose makeAdditivePose(Skeleton& skeleton, Clip& clip);
void add(Pose& output, Pose& inPose, Pose& addPose, Pose& additiveBasePose, int blendroot);
bool isInHierarchy(Pose& pose, unsigned int parent, unsigned int search);
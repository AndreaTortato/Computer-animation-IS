#pragma once

#include "../external/bvh-parser.h"
#include "../animation/pose.h"
#include "../animation/skeleton.h"
#include "../animation/clip.h"

constexpr auto PI = 3.14159265359;

bvh::Bvh loadBVHFile(const char* path);
Pose loadRestPose(const bvh::Bvh data);
std::vector<std::string> loadJointNames(const bvh::Bvh data);
Skeleton loadSkeleton(const bvh::Bvh data);
Clip loadAnimationClip(const bvh::Bvh data);
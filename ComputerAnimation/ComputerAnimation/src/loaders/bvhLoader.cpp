#include "bvhLoader.h"

bvh::Bvh loadBVHFile(const char* path) {
	bvh::Bvh_parser parser;
	bvh::Bvh data;
	parser.parse(path, &data);
	data.recalculateJoints(data.rootJoint());
	

	return data;
}

Pose loadRestPose(bvh::Bvh data) {
	Pose pose;
	pose.resize(data.joints().size());

	for (int i = 0; i < pose.size(); i++) {
		// load rest pose
		Transform t;
		t.position = vec3(data.joints()[i]->offset().x, data.joints()[i]->offset().y, data.joints()[i]->offset().z);

		int parentId = -1;
		if (data.joints()[i]->parent())
			parentId = data.joints()[i]->parent()->id;

		pose.setLocalTransform(i, t);
		pose.setParent(i, parentId);
	}
	return pose;
}

Skeleton loadSkeleton(const bvh::Bvh data) {
	Pose pose = loadRestPose(data);
	return Skeleton( pose, pose, loadJointNames(data) ); // the skeleton uses the rest pose as the bind pose
}

std::vector<std::string> loadJointNames(const bvh::Bvh data) {
	std::vector<std::string> jointNames;

	for (int i = 0; i < data.joints().size(); i++) {
		jointNames.push_back(data.joints()[i]->name()); // load names
	}
	return jointNames;
}

Clip loadAnimationClip(const bvh::Bvh data) {
	Clip clip;
	unsigned int numFrames = data.numFrames();
	for (int i = 0; i < data.joints().size(); i++) {

		std::shared_ptr<bvh::Joint> joint = data.joints()[i];
		unsigned int numChannels = joint->num_channels();
		std::vector<bvh::Joint::Channel> channels = joint->channels_order();

		bool hasPositionTrack = false;
		bool hasRotationTrack = false;
	
		VectorTrack& vt = clip[i].getPositionTrack();
		QuaternionTrack& qt = clip[i].getRotationTrack();
		float time = 0;
		for (unsigned int j = 0; j < numFrames; ++j) {
			VectorFrame f;
			QuaternionFrame q;
			q.value[0] = 0;
			q.value[1] = 0;
			q.value[2] = 0;
			q.value[3] = 1;

			std::vector<float> values = joint->channel_data(j); 
			for (unsigned int c = 0; c < numChannels; c++) {
				switch (channels[c])
				{
				case bvh::Joint::Channel::XPOSITION:
					hasPositionTrack = true;
					f.value[0] = values[c] + joint->offset().x;
					break;
				case bvh::Joint::Channel::YPOSITION:
					hasPositionTrack = true;
					f.value[1] = values[c] + joint->offset().y;
					break;
				case bvh::Joint::Channel::ZPOSITION:
					hasPositionTrack = true;
					f.value[2] = values[c] + joint->offset().z;
					break;
				case bvh::Joint::Channel::XROTATION:
				{
					hasRotationTrack = true;
					quat rotation = angleAxis(values[c] * PI/180, vec3(1, 0, 0)) * quat(q.value[0], q.value[1], q.value[2], q.value[3]);
					q.value[0] = rotation.x;
					q.value[1] = rotation.y;
					q.value[2] = rotation.z;
					q.value[3] = rotation.w;
					break;
				}
				case bvh::Joint::Channel::YROTATION:
				{
					hasRotationTrack = true;
					quat rotation = angleAxis(values[c] * PI / 180, vec3(0, 1, 0)) * quat(q.value[0], q.value[1], q.value[2], q.value[3]);
					q.value[0] = rotation.x;
					q.value[1] = rotation.y;
					q.value[2] = rotation.z;
					q.value[3] = rotation.w;
					break;
				}
				case bvh::Joint::Channel::ZROTATION:
				{
					hasRotationTrack = true;
					quat rotation = angleAxis(values[c] * PI / 180, vec3(0, 0, 1)) * quat(q.value[0], q.value[1], q.value[2], q.value[3]);
					q.value[0] = rotation.x;
					q.value[1] = rotation.y;
					q.value[2] = rotation.z;
					q.value[3] = rotation.w;
					break;
				}
				default:
					break;
				}
			}
			if (hasPositionTrack) {
				f.time = time;
				if (!vt.size()) {
					vt.resize(numFrames);
				}
				vt[j] = f;
			}
			
			if (hasRotationTrack) {
				q.time = time;
				if (!qt.size()) {
					qt.resize(numFrames);
				}
				qt[j] = q;
			}

			time += data.frameTime();
		}
	}
	clip.recalculateDuration();
	return clip;
}
#pragma once

#include <memory>
#include <vector>
#include <string>
#include "../math/vec3.h"
#include "../math/mat4.h"

#include <fstream>
#include <sstream>
#ifndef JOINT_H
#define JOINT_H

// from https://github.com/BartekkPL/bvh-parser (modified)
namespace bvh {

    /** Class created for storing single joint data from bvh file */
    class Joint {
    public:
        /** A struct that keep offset of joint in relation to parent */
        struct Offset {
            float x;
            float y;
            float z;
        };

        /** A enumeration type useful for set order of channels for every joint */
        enum class Channel {
            XPOSITION,
            YPOSITION,
            ZPOSITION,
            ZROTATION,
            XROTATION,
            YROTATION
        };

        /** A string names for each channel */
        const std::vector<std::string> channel_name_str = {
          "XPOSITION",
          "YPOSITION",
          "ZPOSITION",
          "ZROTATION",
          "XROTATION",
          "YROTATION"
        };

        unsigned int id;

        /** Adds single frame motion data
         *  @param  data    The motion data to be added
         */
        void add_frame_motion_data(const std::vector <float>& data) {
            channel_data_.push_back(data);
        }

        /** Gets the parent joint of this joint
         *  @return  The parent joint
         */
        std::shared_ptr <Joint> parent() const { return parent_; }

        /** Gets the name of this joint
         *  @return  The joint's name
         */
        std::string name() const { return name_; }

        /** Gets the offset of this joint
         *  @return  The joint's offset
         */
        Offset offset() const { return offset_; }

        /** Gets the channels order of this joint
         *  @return  The joint's channels order
         */
        std::vector <Channel> channels_order() const {
            return channels_order_;
        }

        /** Gets the all children joints of this joint
         *  @return  The joint's children
         */
        std::vector <std::shared_ptr <Joint>> children() const {
            return children_;
        }

        /** Gets the channels data of this joint for all frames
         *  @return  The joint's channel data
         */
        const std::vector <std::vector <float>>& channel_data() const {
            return channel_data_;
        }

        /** Gets the channel data of this joint for selected frame
         *  @param   frame   The frame for which channel data will be returned
         *  @return  The joint's channel data for selected frame
         */
        const std::vector <float>& channel_data(unsigned frame) const {
            return channel_data_[frame];
        }

        /** Gets the channel data of this joint for selected frame and channel
         *  @param   frame        The frame for which channel data will be returned
         *  @param   channel_num  The number of channel which data will be returned
         *  @return  The joint's channel data for selected frame and channel
         */
        float channel_data(unsigned frame, unsigned channel_num) const {
            return channel_data_[frame][channel_num];
        }

        /** Gets the local transformation matrix for this joint for all frames
         *  @return  The joint's local transformation matrix
         */
        std::vector <mat4> ltm() const {
            return ltm_;
        }

        /** Gets the local transformation matrix for this joint for selected frame
         *  @param   frame    The frame for which ltm will be returned
         *  @return  The joint's local transformation matrix for selected frame
         */
        mat4 ltm(unsigned frame) const {
            return ltm_[frame];
        }

        /** Gets the position for this joint for all frames
         *  @return  The joint's position
         */
        std::vector <vec3> pos() const {
            return pos_;
        }

        /** Gets the position for this joint for selected frame
         *  @param   frame    The frame for which ltm will be returned
         *  @return  The joint's position for selected frame
         */
        vec3 pos(unsigned frame) const {
            return pos_[frame];
        }

        /** Gets the number of channels of this joint
         *  @return  The joint's channels number
         */
        unsigned num_channels() const { return channels_order_.size(); }

        /** Sets the this joint parent joint
         *  @param   arg    The parent joint of this joint
         */
        void set_parent(const std::shared_ptr <Joint> arg) { parent_ = arg; }

        /** Sets the this joint name
         *  @param   arg    The name of this joint
         */
        void set_name(const std::string arg) { name_ = arg; }

        /** Sets the this joint offset
         *  @param   arg    The offset of this joint
         */
        void set_offset(const Offset arg) { offset_ = arg; }

        /** Sets the this joint channels order
         *  @param   arg    The channels order of this joint
         */
        void set_channels_order(const std::vector <Channel>& arg) {
            channels_order_ = arg;
        }

        /** Sets the this joint children
         *  @param   arg    The children of this joint
         */
        void set_children(const std::vector <std::shared_ptr <Joint>>& arg) {
            children_ = arg;
        }

        /** Sets the this joint channels data
         *  @param   arg    The channels data of this joint
         */
        void set_channel_data(const std::vector <std::vector <float>>& arg) {
            channel_data_ = arg;
        }

        /** Sets local transformation matrix for selected frame
         *  @param  matrix  The local transformation matrix to be set
         *  @param  frame   The number of frame for which you want set ltm. As
         *                  default it is set to 0.
         */
        void set_ltm(const mat4 matrix, unsigned frame = 0) {
            if (frame > 0 && frame < ltm_.size())
                ltm_[frame] = matrix;
            else
                ltm_.push_back(matrix);
        }

        /** Sets local transformation matrix for selected frame
         *  @param  pos     The position of joint in selected frame to be set
         *  @param  frame   The number of frame for which you want set position. As
         *                  default it is set to 0.
         */
        void set_pos(const vec3 pos, unsigned frame = 0) {
            if (frame > 0 && frame < pos_.size())
                pos_[frame] = pos;
            else
                pos_.push_back(pos);
        }

        /** Gets channels name of this joint
         *  @return The joint's channels name
         */
        const std::vector<std::string> get_channels_name() const {
            std::vector<std::string> channel_names;

            for (int i = 0; i < channels_order_.size(); i++)
                channel_names.push_back(channel_name_str[static_cast<int>(
                    channels_order_[i])]);

            return channel_names;
        }

    private:
        /** Parent joint in file hierarchy */
        std::shared_ptr <Joint> parent_;
        std::string name_;
        Offset offset_;
        /** Order of joint's input channels */
        std::vector <Channel> channels_order_;
        /** Pointers to joints that are children of this in hierarchy */
        std::vector <std::shared_ptr <Joint>> children_;
        /** Structure for keep joint's channel's data.
         *  Each vector keep data for one channel.
         */
        std::vector <std::vector <float> > channel_data_;
        /** Local transformation matrix for each frame */
        std::vector <mat4> ltm_;
        /** Vector x, y, z of joint position for each frame */
        std::vector <vec3> pos_;
    };

} // namespace
#endif // JOINT_H

#ifndef BVH_H
#define BVH_H

namespace bvh {

    /** Class created for storing motion data from bvh file */
    class Bvh {
    public:
        /** Constructor of Bvh object
         *  @details  Initializes local variables
         */
        Bvh() : num_frames_(0), frame_time_(0), num_channels_(0) {}

        /**
         * Recalculation of local transformation matrix for each frame in each joint
         *
         * Should be called to set local_transformation_matrix vectors in joints
         * structures.
         *
         * @param start_joint  A joint of which each child local transformation
         * matrix will be recalculated, as default it is NULL which will be resolved
         * to root_joint in method body
         */
        void recalculateJoints(std::shared_ptr<Joint> start_joint = NULL);

        /** Adds joint to Bvh object
         *  @details  Adds joint and increases number of data channels
         *  @param  joint  The joint that will be added
         */
        void addJoint(const std::shared_ptr<Joint> joint) {
            joint->id = joints_.size();
            joints_.push_back(joint);
            num_channels_ += joint->num_channels();
        }

        /** Gets the root joint
         *  @return  The root joint
         */
        const std::shared_ptr<Joint> rootJoint() const { return root_joint_; }

        /** Gets all joints
         *  @return  The all joints
         */
        const std::vector <std::shared_ptr <Joint>> joints() const {
            return joints_;
        }

        /** Gets the number of data frames
         *  @return  The number of frames
         */
        unsigned numFrames() const { return num_frames_; }

        /** Gets the frame time
         *  @return  The single frame time (in second)
         */
        double frameTime() const { return frame_time_; }

        /** Gets the total number of channels
         *  @return  The number of data channels
         */
        unsigned numChannels() const { return num_channels_; }

        /** Sets the root joint
         *  @param  arg  The root joint to be set
         */
        void setRootJoint(const std::shared_ptr<Joint> arg) { root_joint_ = arg; }

        /** Sets the all joint at once
         *  @param  arg  The all joints to be set
         */
        void setJoints(const std::vector <std::shared_ptr <Joint>> arg) {
            joints_ = arg;
        }

        /** Sets the number of data frames
         *  @param  arg  The number of frames to be set
         */
        void setNumFrames(const unsigned arg) { num_frames_ = arg; }

        /** Sets the single data frame time
         *  @param  arg  The time of frame to be set
         */
        void setFrameTime(const double arg) { frame_time_ = arg; }

    private:
        /** A root joint in this bvh file */
        std::shared_ptr<Joint> root_joint_;
        /** All joints in file in order of parse */
        std::vector <std::shared_ptr <Joint>> joints_;
        /** A number of motion frames in this bvh file */
        unsigned num_frames_;
        /** A time of single frame */
        double frame_time_;
        /** Number of channels of all joints */
        unsigned num_channels_;
    };

} // namespace
#endif  // BVH_H

#ifndef BVH_PARSER_H
#define BVH_PARSER_H
#define MULTI_HIERARCHY 0

#include <functional>
#include <locale>
namespace bvh {

    /** Bvh Parser class that is responsible for parsing .bvh file */
    class Bvh_parser {
    public:
        /** Parses single bvh file and stored data into bvh structure
         *  @param  path  The path to file to be parsed
         *  @param  bvh   The pointer to bvh object where parsed data will be stored
         *  @return  0 if success, -1 otherwise
         */
        int parse(const char* path, Bvh* bvh);

    private:
        /** Parses single hierarchy in bvh file
         *  @param  file  The input stream that is needed for reading file content
         *  @return  0 if success, -1 otherwise
         */
        int parseHierarchy(std::ifstream& file);

        /** Parses joint and its children in bvh file
         *  @param  file    The input stream that is needed for reading file content
         *  @param  parent  The pointer to parent joint
         *  @param  parsed  The output parameter, here will be stored parsed joint
         *  @return  0 if success, -1 otherwise
         */
        int parseJoint(std::ifstream& file, std::shared_ptr <Joint> parent,
            std::shared_ptr <Joint>& parsed);

        /** Parses order of channel for single joint
         *  @param  file    The input stream that is needed for reading file content
         *  @param  joint   The pointer to joint that channels order will be parsed
         *  @return  0 if success, -1 otherwise
         */
        int parseChannelOrder(std::ifstream& file, std::shared_ptr <Joint> joint);

        /** Parses motion part data
         *  @param  file    The input stream that is needed for reading file content
         *  @return  0 if success, -1 otherwise
         */
        int parseMotion(std::ifstream& file);

        /** Trims the string, removes leading and trailing whitespace from it
         *  @param  s   The string, which leading and trailing whitespace will be
         *              trimmed
         */
        inline void trim(std::string& s) {
            s.erase(std::remove_if(s.begin(), s.end(),
                std::bind(std::isspace<char>, std::placeholders::_1,
                    std::locale::classic())), s.end());
        }

        /** Converts the vector of float to string, ex. "el1, el2, el3"
         *  @param  vector  The data that will be converted to string
         *  @return  The string that will be created from input data
         */
        std::string vtos(const std::vector <float>& vector);

        /** The path to file that was parsed previously */
        const char* path_;

        /** The bvh object to store parsed data */
        Bvh* bvh_;
    };
}
#endif
/* RTC:OpenNI
 *
 * Copyright (C) 2011
 *     Geoffrey Biggs
 *     RT-Synthesis Research Group
 *     Intelligent Systems Research Institute,
 *     National Institute of Advanced Industrial Science and Technology (AIST),
 *     Japan
 *     All rights reserved.
 * Licensed under the Eclipse Public License -v 1.0 (EPL)
 * http://www.opensource.org/licenses/eclipse-1.0.txt
 *
 * Component header file.
 */


#ifndef RTC_H__
#define RTC_H__


#include "idl/pointcloud.hh"

#include <rtm/Manager.h>
#include <rtm/DataFlowComponentBase.h>
#include <rtm/idl/InterfaceDataTypes.hh>
#include <rtm/OutPort.h>
#include <XnCppWrapper.h>


class RTCOpenNI
: public RTC::DataFlowComponentBase
{
    public:
        RTCOpenNI(RTC::Manager* manager);
        ~RTCOpenNI();

        virtual RTC::ReturnCode_t onInitialize();
        virtual RTC::ReturnCode_t onFinalize();
        virtual RTC::ReturnCode_t onActivated(RTC::UniqueId ec_id);
        virtual RTC::ReturnCode_t onDeactivated(RTC::UniqueId ec_id);
        virtual RTC::ReturnCode_t onExecute(RTC::UniqueId ec_id);

    private:
        PointCloudTypes::PointCloud cloud_;
        RTC::OutPort<PointCloudTypes::PointCloud> cloud_port_;
        RTC::CameraImage depth_map_;
        RTC::OutPort<RTC::CameraImage> depth_map_port_;
        RTC::CameraImage image_;
        RTC::OutPort<RTC::CameraImage> image_port_;

        bool enable_depth_;
        unsigned int dm_fps_;
        unsigned int dm_x_;
        unsigned int dm_y_;
        bool enable_image_;
        unsigned int im_fps_;
        unsigned int im_x_;
        unsigned int im_y_;

        xn::Context xnc_;
        xn::DepthGenerator depth_gen_;
        xn::ImageGenerator image_gen_;
        XnUInt64 no_sample_val_;
        XnUInt64 shadow_val_;
        XnUInt64 min_depth_;
        XnUInt64 max_depth_;
        XnDouble pixel_size_;
        XnUInt64 depth_focal_length_;
        double proj_const_;
        double centre_x_, centre_y_;

        RTC::ReturnCode_t publish_depth();
        RTC::ReturnCode_t publish_image();
};


extern "C"
{
    DLL_EXPORT void rtc_init(RTC::Manager* manager);
};

#endif // RTC_H__


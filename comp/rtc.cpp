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
 * Component source file.
 */


#include "rtc.h"


RTCOpenNI::RTCOpenNI(RTC::Manager* manager)
    : RTC::DataFlowComponentBase(manager),
    cloud_port_("points", cloud_),
    depth_map_port_("depth_map", depth_map_),
    image_port_("image", image_),
    enable_depth_(false), dm_fps_(30), dm_x_(640), dm_y_(480),
    enable_image_(false), im_fps_(30), im_x_(640), im_y_(480),
    no_sample_val_(0.0), shadow_val_(0.0)
{
}


RTCOpenNI::~RTCOpenNI()
{
}


RTC::ReturnCode_t RTCOpenNI::onInitialize()
{
    bindParameter("enable_depth", enable_depth_, "true"),
    bindParameter("dm_fps", dm_fps_, "30");
    bindParameter("dm_x", dm_x_, "640");
    bindParameter("dm_y", dm_y_, "480");
    bindParameter("enable_image", enable_image_, "true"),
    bindParameter("im_fps", im_fps_, "30");
    bindParameter("im_x", im_x_, "640");
    bindParameter("im_y", im_y_, "480");
    /*std::string active_set =
        m_properties.getProperty("configuration.active_config", "default");
    m_configsets.update(active_set.c_str());*/

    comp.addOutPort(cloud_port_.getName(), cloud_port_);
    comp.addOutPort(depth_map_port_.getName(), depth_map_port_);
    comp.addOutPort(image_port_.getName(), image_port_);

    xn::XnStatus res(xn::XN_STATUS_OK);
    if ((res = xnc_.Init()) != xn::XN_STATUS_OK)
    {
        std::cerr << "RTC:OpenNI: Failed to initialise: " <<
            xn::xnGetStatusString(res) << '\n';
        return RTC::RTC_ERROR;
    }

    if ((res = depth_gen_.Create(xnc_)) != xn::XN_STATUS_OK)
    {
        std::cerr << "RTC:OpenNI: Failed to create depth node: " <<
            xn::xnGetStatusString(res) << '\n';
        return RTC::RTC_ERROR;
    }

    if ((res = image_gen_.Create(xnc_)) != xn::XN_STATUS_OK)
    {
        std::cerr << "RTC:OpenNI: Failed to create image node: " <<
            xn::xnGetStatusString(res) << '\n';
        return RTC::RTC_ERROR;
    }

    return RTC::RTC_OK;
}


RTC::ReturnCode_t RTCOpenNI::onFinalize()
{
    xnc_.Shutdown();

    return RTC::RTC_OK;
}


RTC::ReturnCode_t RTCOpenNI::onActivated(RTC::UniqueId ec_id)
{
    xn::XnStatus res(xn::XN_STATUS_OK);

    // Apply configuration
    xn::XnMapOutputMode output_mode;
    if (enable_depth_)
    {
        output_mode.nXRes = dm_x_;
        output_mode.nYRes = dm_y_;
        output_mode.nFPS = dm_fps_;
        if ((res = depth_gen_.SetMapOutputMode(output_mode)) != xn::XN_STATUS_OK)
        {
            std::cerr << "RTC:OpenNI: Failed to configure depth node: " <<
                xn::xnGetStatusString(res) << '\n';
            return RTC::RTC_ERROR;
        }
        depth_map_.width = dm_x_;
        depth_map_.height = dm_y_;
        depth_map_.bpp = ;
        depth_map_.format = "depthmap";
        depth_map_.fDiv = 1.0;
        depth_map_.pixels.length(dm_x_ * dm_y_ * ); // * bpp value

        cloud_.fields = ;
        cloud_.point_step = ;
        cloud_.is_bigendian = false;
        cloud_.is_dense = false;
    }
    if (enable_image_)
    {
        output_mode.nXRes = im_x_;
        output_mode.nYRes = im_y_;
        output_mode.nFPS = im_fps_;
        if ((res = image_gen_.SetMapOutputMode(output_mode)) != xn::XN_STATUS_OK)
        {
            std::cerr << "RTC:OpenNI: Failed to configure image node: " <<
                xn::xnGetStatusString(res) << '\n';
            return RTC::RTC_ERROR;
        }
        image_map_.width = im_x_;
        image_map_.height = im_y_;
        image_map_.bpp = ;
        image_map_.format = "bitmap";
        image_map_.fDiv = 1.0;
        image_map_.pixels.length(im_x_ * im_y_ * ); // * bpp value
    }
    if (enable_depth_ || enable_image_)
    {
        if ((res = depth_gen.GetIntProperty("NoSampleValue", no_sample_val_)) !=
                xn::XN_STATUS_OK)
        {
            std::cerr << "RTC:OpenNI: Failed to get no-sample value: " <<
                xn::xnGetStatusString(res) << '\n';
            return RTC::RTC_ERROR;
        }
        if ((res = depth_gen.GetIntProperty("ShadowValue", shadow_val_)) !=
                xn::XN_STATUS_OK)
        {
            std::cerr << "RTC:OpenNI: Failed to get shadow value: " <<
                xn::xnGetStatusString(res) << '\n';
            return RTC::RTC_ERROR;
        }
    }

    // Start generating data
    if ((res == xnc_.StartGeneratingAll()) != xn::XN_STATUS_OK)
    {
        std::cerr << "RTC:OpenNI: Failed to start data generation: " <<
            xn::xnGetStatusString(res) << '\n';
        return RTC::RTC_ERROR;
    }

    return RTC::RTC_OK;
}


RTC::ReturnCode_t RTCOpenNI::onDeactivated(RTC::UniqueId ec_id)
{
    // Stop data generation
    xn::XnStatus res(xn::XN_STATUS_OK);
    if ((res = xnc_.StopGeneratingAll()) != xn::XN_STATUS_OK)
    {
        std::cerr << "RTC:OpenNI: Failed to stop data generation: " <<
            xn::xnGetStatusString(res) << '\n';
        return RTC::RTC_ERROR;
    }

    return RTC::RTC_OK;
}


RTC::ReturnCode_t RTCOpenNI::onExecute(RTC::UniqueId ec_id)
{
    xn::XnStatus res(xn::XN_STATUS_OK);
    if ((res = xnc_.xnWaitNoneUpdateAll()) != xn::XN_STATUS_OK)
    {
        std::cerr << "RTC:OpenNI: Failed OpenNI update: " <<
            xn::xnGetStatusString(res) << '\n';
        return RTC::RTC_ERROR;
    }

    return RTC::RTC_OK;
}


RTC::ReturnCode_t RTCOpenNI::publish_depth()
{
    if (!enable_depth_)
    {
        return RTC::RTC_OK;
    }
    if (!depth_gen_.IsNewDataAvailable())
    {
        return RTC::RTC_OK;
    }

    // Read and publish depth data
    depth_gen_.WaitAndUpdateData();
    xn::DepthMetaData depth_md;
    depth_gen_.GetMetaData(depth_md);

    // Depth map
    depth_map_.tm.sec = ;
    depth_map_.tm.nsec = ;
    for (unsigned int ii = 0; ii < ; ii++)
    {
        depth_map_.pixels[] = depth_md[ii];
    }

    // Point cloud
    cload_.tm.sec = ;
    cloud_.tm.nsec = ;
    cloud_.seq = depth_md.FrameID();
    cloud_.height = ;
    cloud_.width = ;
    cloud_.row_step = ;
    cloud_.is_dense = false;
}


RTC::ReturnCode_t RTCOpenNI::publish_image()
{
    if (!enable_image_)
    {
        return RTC::RTC_OK;
    }
    if (!image_gen_.IsNewDataAvailable())
    {
        return RTC::RTC_OK;
    }

    // Read and publish image data
    image_gen_.WaitAndUpdateData();

    image_map_.tm.sec = ;
    image_map_.tm.nsec = ;
    for (unsigned int ii = 0; ii < ; ii++)
    {
        image_map_.pixels[] = ;
    }
}


static const char* spec[] =
{
    "implementation_id", "rtcopenni",
    "type_name",         "rtcopenni",
    "description",       "RTC:OpenNI",
    "version",           "1.0",
    "vendor",            "Geoffrey Biggs, AIST",
    "category",          "Sensor",
    "activity_type",     "PERIODIC",
    "kind",              "DataFlowComponent",
    "max_instance",      "1",
    "language",          "C++",
    "lang_type",         "compile",
    // Configuration variables
    "config.default.enable_depth", "true",
    "config.default.dm_fps",    "30",
    "config.default.dm_x",      "640",
    "config.default.dm_y",      "480",
    "config.default.enable_image", "true",
    "config.default.im_fps",    "30",
    "config.default.im_x",      "640",
    "config.default.im_y",      "480",
    // Widget
    "conf.__widget__.enable_depth", "checkbox",
    "conf.__widget__.dm_fps",    "spin",
    "conf.__widget__.dm_x",      "spin",
    "conf.__widget__.dm_y",      "spin",
    "conf.__widget__.enable_image", "checkbox",
    "conf.__widget__.im_fps",    "spin",
    "conf.__widget__.im_x",      "spin",
    "conf.__widget__.im_y",      "spin",
    // Constraints
    ""
};

extern "C"
{
    void rtc_init(RTC::Manager* manager)
    {
        coil::Properties profile(spec);
        manager->registerFactory(profile, RTC::Create<RTCOpenNI>,
                RTC::Delete<RTCOpenNI>);
    }
};


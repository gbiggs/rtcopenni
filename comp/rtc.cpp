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

#include <limits>


RTCOpenNI::RTCOpenNI(RTC::Manager* manager)
    : RTC::DataFlowComponentBase(manager),
    cloud_port_("points", cloud_),
    depth_map_port_("depth_map", depth_map_),
    image_port_("image", image_),
    enable_depth_(true), dm_fps_(30), dm_x_(640), dm_y_(480),
    enable_image_(true), im_fps_(30), im_x_(640), im_y_(480),
    no_sample_val_(0), shadow_val_(0), min_depth_(0), max_depth_(0),
    pixel_size_(0.0), depth_focal_length_(0)
{
}


RTCOpenNI::~RTCOpenNI()
{
}


RTC::ReturnCode_t RTCOpenNI::onInitialize()
{
    bindParameter("enable_depth", enable_depth_, "1"),
    bindParameter("dm_fps", dm_fps_, "30");
    bindParameter("dm_x", dm_x_, "640");
    bindParameter("dm_y", dm_y_, "480");
    bindParameter("enable_image", enable_image_, "1"),
    bindParameter("im_fps", im_fps_, "30");
    bindParameter("im_x", im_x_, "640");
    bindParameter("im_y", im_y_, "480");
    /*std::string active_set =
        m_properties.getProperty("configuration.active_config", "default");
    m_configsets.update(active_set.c_str());*/

    addOutPort(cloud_port_.getName(), cloud_port_);
    addOutPort(depth_map_port_.getName(), depth_map_port_);
    addOutPort(image_port_.getName(), image_port_);

    XnStatus res(XN_STATUS_OK);
    if ((res = xnc_.Init()) != XN_STATUS_OK)
    {
        std::cerr << "RTC:OpenNI: Failed to initialise: " <<
            xnGetStatusString(res) << '\n';
        return RTC::RTC_ERROR;
    }

    if ((res = depth_gen_.Create(xnc_)) != XN_STATUS_OK)
    {
        std::cerr << "RTC:OpenNI: Failed to create depth node: " <<
            xnGetStatusString(res) << '\n';
        return RTC::RTC_ERROR;
    }

    if ((res = image_gen_.Create(xnc_)) != XN_STATUS_OK)
    {
        std::cerr << "RTC:OpenNI: Failed to create image node: " <<
            xnGetStatusString(res) << '\n';
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
    XnStatus res(XN_STATUS_OK);

    std::cout << "Activating\n";

    // Apply configuration
    XnMapOutputMode output_mode;
    if (enable_depth_)
    {
        std::cout << "Enabling depth\n";
        output_mode.nXRes = dm_x_;
        output_mode.nYRes = dm_y_;
        output_mode.nFPS = dm_fps_;
        if ((res = depth_gen_.SetMapOutputMode(output_mode)) != XN_STATUS_OK)
        {
            std::cerr << "RTC:OpenNI: Failed to configure depth node: " <<
                xnGetStatusString(res) << '\n';
            return RTC::RTC_ERROR;
        }
        depth_map_.width = dm_x_;
        depth_map_.height = dm_y_;
        depth_map_.bpp = 32;
        depth_map_.format = "DEPTH";
        depth_map_.fDiv = 1.0;
        depth_map_.pixels.length(dm_x_ * dm_y_ * 4);

        cloud_.fields.length(3);
        cloud_.fields[0].name = "x";
        cloud_.fields[0].offset = 0;
        cloud_.fields[0].data_type = PointCloudTypes::FLOAT32;
        cloud_.fields[0].count = 4;
        cloud_.fields[0].name = "y";
        cloud_.fields[0].offset = 4;
        cloud_.fields[0].data_type = PointCloudTypes::FLOAT32;
        cloud_.fields[0].count = 4;
        cloud_.fields[0].name = "z";
        cloud_.fields[0].offset = 8;
        cloud_.fields[0].data_type = PointCloudTypes::FLOAT32;
        cloud_.fields[0].count = 4;
        cloud_.point_step = 12;
        cloud_.row_step = dm_x_ * 12;
        cloud_.is_bigendian = false;
        cloud_.is_dense = false;
        if ((res = depth_gen_.GetIntProperty("ZPD", depth_focal_length_)) !=
                XN_STATUS_OK)
        {
            std::cerr << "RTC:OpenNI: Failed to get depth focal length: " <<
                xnGetStatusString(res) << '\n';
            return RTC::RTC_ERROR;
        }
        std::cout << "Depth focal length " << depth_focal_length_ << '\n';
    }
    if (enable_image_)
    {
        output_mode.nXRes = im_x_;
        output_mode.nYRes = im_y_;
        output_mode.nFPS = im_fps_;
        if ((res = image_gen_.SetMapOutputMode(output_mode)) != XN_STATUS_OK)
        {
            std::cerr << "RTC:OpenNI: Failed to configure image node: " <<
                xnGetStatusString(res) << '\n';
            return RTC::RTC_ERROR;
        }
        image_.fDiv = 1.0;
    }
    if (enable_depth_ || enable_image_)
    {
        if ((res = depth_gen_.GetIntProperty("NoSampleValue", no_sample_val_))
                != XN_STATUS_OK)
        {
            std::cerr << "RTC:OpenNI: Failed to get no-sample value: " <<
                xnGetStatusString(res) << '\n';
            return RTC::RTC_ERROR;
        }
        if ((res = depth_gen_.GetIntProperty("ShadowValue", shadow_val_)) !=
                XN_STATUS_OK)
        {
            std::cerr << "RTC:OpenNI: Failed to get shadow value: " <<
                xnGetStatusString(res) << '\n';
            return RTC::RTC_ERROR;
        }
        if ((res = depth_gen_.GetIntProperty("MinDepthValue", min_depth_)) !=
                XN_STATUS_OK)
        {
            std::cerr << "RTC:OpenNI: Failed to get minimum depth: " <<
                xnGetStatusString(res) << '\n';
            return RTC::RTC_ERROR;
        }
        std::cout << "Min depth " << min_depth_ << '\n';
        if ((res = depth_gen_.GetIntProperty("MaxDepthValue", max_depth_)) !=
                XN_STATUS_OK)
        {
            std::cerr << "RTC:OpenNI: Failed to get maximum depth: " <<
                xnGetStatusString(res) << '\n';
            return RTC::RTC_ERROR;
        }
        std::cout << "Max depth " << max_depth_ << '\n';
        if ((res = depth_gen_.GetRealProperty("ZPPS", pixel_size_)) !=
                XN_STATUS_OK)
        {
            std::cerr << "RTC:OpenNI: Failed to get pixel size: " <<
                xnGetStatusString(res) << '\n';
            return RTC::RTC_ERROR;
        }
        std::cout << "Pixel size " << pixel_size_ << '\n';

        XnFieldOfView fov;
        depth_gen_.GetFieldOfView(fov);
        std::cout << "hFoV " << fov.fHFOV << '\n';
        std::cout << "vFoV " << fov.fVFOV << '\n';
    }

    // Start generating data
    if ((res = xnc_.StartGeneratingAll()) != XN_STATUS_OK)
    {
        std::cerr << "RTC:OpenNI: Failed to start data generation: " <<
            xnGetStatusString(res) << '\n';
        return RTC::RTC_ERROR;
    }

    return RTC::RTC_OK;
}


RTC::ReturnCode_t RTCOpenNI::onDeactivated(RTC::UniqueId ec_id)
{
    // Stop data generation
    XnStatus res(XN_STATUS_OK);
    if ((res = xnc_.StopGeneratingAll()) != XN_STATUS_OK)
    {
        std::cerr << "RTC:OpenNI: Failed to stop data generation: " <<
            xnGetStatusString(res) << '\n';
        return RTC::RTC_ERROR;
    }

    return RTC::RTC_OK;
}


RTC::ReturnCode_t RTCOpenNI::onExecute(RTC::UniqueId ec_id)
{
    XnStatus res(XN_STATUS_OK);
    if ((res = xnc_.WaitNoneUpdateAll()) != XN_STATUS_OK)
    {
        std::cerr << "RTC:OpenNI: Failed OpenNI update: " <<
            xnGetStatusString(res) << '\n';
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
    XnMapOutputMode output_mode;
    depth_gen_.GetMapOutputMode(output_mode);
    xn::DepthMetaData depth_md;
    depth_gen_.GetMetaData(depth_md);
    std::cout << "Depth time stamp is " << depth_md.Timestamp() << '\n';

    // Depth map
    depth_map_.tm.sec = 0;
    depth_map_.tm.nsec = 0;
    unsigned int num_pixels = output_mode.nXRes * output_mode.nYRes;
    for (unsigned int pixel = 0, dest = 0; pixel < num_pixels;
            pixel++, dest += 4)
    {
        if (depth_md[pixel] == no_sample_val_ ||
                depth_md[pixel] == shadow_val_)
        {
            depth_map_.pixels[dest] = std::numeric_limits<float>::quiet_NaN();
        }
        else
        {
            // Convert the distance from mm as an int to metres as a float
            *(reinterpret_cast<float*>(&(depth_map_.pixels[dest]))) =
                static_cast<float>(depth_md[pixel]) / 1000.0;
        }
    }
    depth_map_port_.write();

    // Point cloud
    cloud_.tm.sec = 0;
    cloud_.tm.nsec = 0;
    cloud_.seq = depth_md.FrameID();
    cloud_.height = output_mode.nXRes;
    cloud_.width = output_mode.nYRes;
    cloud_.data.length(depth_md.XRes() * depth_md.YRes() * cloud_.point_step);
    for (unsigned int pixel = 0, dest = 0; pixel < num_pixels;
            pixel++, dest += 12)
    {
    }

    return RTC::RTC_OK;
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
    xn::ImageMetaData image_md;
    image_gen_.GetMetaData(image_md);
    std::cout << "Image time stamp is " << image_md.Timestamp() << '\n';
    image_.tm.sec = 0;
    image_.tm.nsec = 0;
    switch (image_md.PixelFormat())
    {
        case XN_PIXEL_FORMAT_RGB24:
            image_.format = "RGB";
            image_.bpp = 24;
            break;
        case XN_PIXEL_FORMAT_YUV422:
            image_.format = "YUV422";
            image_.bpp = 32;
            break;
        case XN_PIXEL_FORMAT_GRAYSCALE_8_BIT:
            image_.format = "GREY";
            image_.bpp = 8;
            break;
        case XN_PIXEL_FORMAT_GRAYSCALE_16_BIT:
            image_.format = "GREY";
            image_.bpp = 16;
            break;
        default:
            std::cerr << "RTC:OpenNI WARNING: Unknown image pixel format: " <<
                image_md.PixelFormat() << '\n';
    };
    image_.width = image_md.XRes();
    image_.height = image_md.YRes();
    image_.pixels.length(image_md.DataSize());
    for (unsigned int ii = 0; ii < image_md.DataSize(); ii++)
    {
        image_.pixels[ii] = image_md.Data()[ii];
    }
    image_port_.write();

    return RTC::RTC_OK;
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
    "conf.default.enable_depth", "1",
    "conf.default.dm_fps",    "30",
    "conf.default.dm_x",      "640",
    "conf.default.dm_y",      "480",
    "conf.default.enable_image", "1",
    "conf.default.im_fps",    "30",
    "conf.default.im_x",      "640",
    "conf.default.im_y",      "480",
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


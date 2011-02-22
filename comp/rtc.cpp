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
    _port_("<PORT_NAME>", _),
    //svc_prov_(),
    //svc_port_("<PORT_NAME>")
    dm_fps_(30), dm_x_(640), dm_y_(480),
    im_fps_(30), im_x_(640), im_y_(480)
{
}


RTCOpenNI::~RTCOpenNI()
{
}


RTC::ReturnCode_t RTCOpenNI::onInitialize()
{
    bindParameter("dm_fps", dm_fps_, "30");
    bindParameter("dm_x", dm_x_, "640");
    bindParameter("dm_y", dm_y_, "480");
    bindParameter("im_fps", im_fps_, "30");
    bindParameter("im_x", im_x_, "640");
    bindParameter("im_y", im_y_, "480");
    /*std::string active_set =
        m_properties.getProperty("configuration.active_config", "default");
    m_configsets.update(active_set.c_str());*/

    comp.addInPort(_port_.getName(), _port_);
    comp.addOutPort(_port_.getName(), _port_);
    //svc_port_.registerProvider("<INSTANCE_NAME>", "<TYPE_NAME>", svc_prov_);
    //comp.addPort(svc_port_);

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
    output_mode.nXRes = dm_x_;
    output_mode.nYRes = dm_y_;
    output_mode.nFPS = dm_fps_;
    if ((res = depth_gen_.SetMapOutputMode(output_mode)) != xn::XN_STATUS_OK)
    {
        std::cerr << "RTC:OpenNI: Failed to configure depth node: " <<
            xn::xnGetStatusString(res) << '\n';
        return RTC::RTC_ERROR;
    }
    output_mode.nXRes = im_x_;
    output_mode.nYRes = im_y_;
    output_mode.nFPS = im_fps_;
    if ((res = image_gen_.SetMapOutputMode(output_mode)) != xn::XN_STATUS_OK)
    {
        std::cerr << "RTC:OpenNI: Failed to configure image node: " <<
            xn::xnGetStatusString(res) << '\n';
        return RTC::RTC_ERROR;
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

    if (depth_gen_.IsNewDataAvailable())
    {
        // Read and publish depth data
        depth_gen_.WaitAndUpdateData();
    }

    if (image_gen_.IsNewDataAvailable())
    {
        // Read and publish image data
        image_gen_.WaitAndUpdateData();
    }

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
    "config.default.dm_fps",    "30",
    "config.default.dm_x",      "640",
    "config.default.dm_y",      "480",
    "config.default.im_fps",    "30",
    "config.default.im_x",      "640",
    "config.default.im_y",      "480",
    // Widget
    "conf.__widget__.dm_fps",   "spin",
    "conf.__widget__.dm_x",     "spin",
    "conf.__widget__.dm_y",     "spin",
    "conf.__widget__.im_fps",   "spin",
    "conf.__widget__.im_x",     "spin",
    "conf.__widget__.im_y",     "spin",
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


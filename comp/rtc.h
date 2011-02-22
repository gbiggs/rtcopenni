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

#include <rtm/Manager.h>
#include <rtm/DataFlowComponentBase.h>
#include <rtm/OutPort.h>
//#include <rtm/CorbaPort.h>
//#include "service_impl.h"
#include <XnCppWrapper.h>

using namespace RTC;


// Base exception
class BaseRTCError : public std::runtime_error
{
    public:
        BaseRTCError(const std::string &arg)
            : std::runtime_error(std::string("Base error ") + arg)
        {}
};


class RTCOpenNI
: public RTC::DataFlowComponentBase
{
    public:
        RTCOpenNI(RTC::Manager* manager);
        ~RTCOpenNI();

        virtual RTC::ReturnCode_t onInitialize();
        virtual RTC::ReturnCode_t onFinalize();
        virtual RTC::ReturnCode_t onActivated(RTC::UniqueId ec_id);
        //virtual RTC::ReturnCode_t onDeactivated(RTC::UniqueId ec_id);
        virtual RTC::ReturnCode_t onExecute(RTC::UniqueId ec_id);

    private:
        RTC:: _;
        RTC::OutPort<RTC::> _port_;
        //ServiceProvider svc_prov_;
        //RTC::CorbaPort svc_port_;

        unsigned int dm_fps_;
        unsigned int dm_x;
        unsigned int dm_y;
        unsigned int im_fps_;
        unsigned int im_x;
        unsigned int im_y;

        xn::Context xnc_;
        xn::DepthGenerator depth_gen_;
        xn::ImageGenerator image_gen_;
};


extern "C"
{
    DLL_EXPORT void rtc_init(RTC::Manager* manager);
};

#endif // RTC_H__


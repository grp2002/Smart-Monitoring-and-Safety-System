/**
 * 
 * Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
 * Copyright 2025 Bernd Porr
 * Copyright 2025 Pragya Shilakari, Gregory Paphiti, Abhishek Jain, Ninad Shende, Ugochukwu Elvis Som Anene, Hankun Ma	
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SENSOR_MSG_SUBSCRIBER_H
#define SENSOR_MSG_SUBSCRIBER_H

#include <functional>
#include "SensorMsgPubSubTypes.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

//using namespace eprosima::fastdds::dds;

class SensorMsgSubscriber
{
private:
    eprosima::fastdds::dds::DomainParticipant* participant_;
    eprosima::fastdds::dds::Subscriber* subscriber_;
    eprosima::fastdds::dds::DataReader* reader_;
    eprosima::fastdds::dds::Topic* topic_;
    eprosima::fastdds::dds::TypeSupport type_;

    class SubListener : public eprosima::fastdds::dds::DataReaderListener
    {
    public:
        SubListener();
        SubListener(SensorMsgSubscriber* parent);
        ~SubListener() override;

        void on_subscription_matched(
            eprosima::fastdds::dds::DataReader* reader,
            const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

        void on_data_available(eprosima::fastdds::dds::DataReader* reader) override;
    
    private:
        SensorMsgSubscriber* parent_;
    };
    SubListener* listener_;

public:
    SensorMsgSubscriber();
    virtual ~SensorMsgSubscriber();

    bool init();
    std::function<void(int, double)> onTemperatureRead;
};

#endif // SENSOR_MSG_SUBSCRIBER_H

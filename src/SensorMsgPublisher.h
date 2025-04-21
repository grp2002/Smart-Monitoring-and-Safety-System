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

/**
 * @file SensorMsgPublisher.h
 *
 */

#ifndef SENSOR_MSG_PUBLISHER_H
#define SENSOR_MSG_PUBLISHER_H

#include "SensorMsgPubSubTypes.h"

#include <atomic>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

class SensorMsgPublisher
{
private:
    eprosima::fastdds::dds::DomainParticipant* participant_;
    eprosima::fastdds::dds::Publisher* publisher_;
    eprosima::fastdds::dds::Topic* topic_;
    eprosima::fastdds::dds::DataWriter* writer_;
    eprosima::fastdds::dds::TypeSupport type_;

    class PubListener : public eprosima::fastdds::dds::DataWriterListener
    {
    public:
        PubListener();
        ~PubListener() override;

        void on_publication_matched(
            eprosima::fastdds::dds::DataWriter* writer,
            const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

        std::atomic_int matched_;
    } listener_;

public:
    SensorMsgPublisher();
    virtual ~SensorMsgPublisher();

    bool init();
    bool publish(SensorMsg& msg);
};

#endif // SENSOR_MSG_PUBLISHER_H

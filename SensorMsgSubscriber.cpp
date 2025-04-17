// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file HelloWorldSubscriber.cpp
 *
 */

#include "SensorMsgPubSubTypes.h"

#include <chrono>
#include <thread>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

using namespace eprosima::fastdds::dds;

class SensorMsgSubscriber
{
private:

    DomainParticipant* participant_ = nullptr;

    Subscriber* subscriber_ = nullptr;

    DataReader* reader_ = nullptr;

    Topic* topic_ = nullptr;

    TypeSupport type_;

    class SubListener : public DataReaderListener
    {
    public:

        SubListener() {}

        ~SubListener() override {}

        void on_subscription_matched(
                DataReader*,
                const SubscriptionMatchedStatus& info) override
        {
            if (info.current_count_change == 1)
            {
                std::cout << "Subscriber matched." << std::endl;
            }
            else if (info.current_count_change == -1)
            {
                std::cout << "Subscriber unmatched." << std::endl;
            }
            else
            {
                std::cout << info.current_count_change
                        << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
            }
        }

	// callback
        void on_data_available(DataReader* reader) override
        {
            SampleInfo info;
	        SensorMsg msg;
            if (reader->take_next_sample(&msg, &info) == ReturnCode_t::RETCODE_OK)
            {
                if (info.valid_data)
                {
                    std::cout << "Message: Sensor Id " << msg.sensor_id()<< " with temperature: " << msg.temperature()
                                << "at " << msg.timestamp() << " RECEIVED" << std::endl;   
                }
            }
        }

    } listener_;

public:

    SensorMsgSubscriber() : type_(new SensorMsgPubSubType()) {}

    virtual ~SensorMsgSubscriber()
    {
        if (reader_ != nullptr)
        {
            subscriber_->delete_datareader(reader_);
        }
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        if (subscriber_ != nullptr)
        {
            participant_->delete_subscriber(subscriber_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    //!Initialize the subscriber
    bool init()
    {
        DomainParticipantQos participantQos;
        participantQos.name("Participant_subscriber");
        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, participantQos);

        if (participant_ == nullptr)
        {
            return false;
        }

        // Register the Type
        type_.register_type(participant_);

        // Create the subscriptions Topic
	    // **** Important that this matches with the name of message defined in HelloWorldMsg.idl ****
        topic_ = participant_->create_topic("Topic", "SensorMsg", TOPIC_QOS_DEFAULT);

        if (topic_ == nullptr)
        {
            return false;
        }

        // Create the Subscriber
        subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);

        if (subscriber_ == nullptr)
        {
            return false;
        }

        // Create the DataReader
        reader_ = subscriber_->create_datareader(topic_, DATAREADER_QOS_DEFAULT, &listener_);

        if (reader_ == nullptr)
        {
            return false;
        }

        return true;
    }

};

int main(
        int,
        char**)
{
    std::cout << "Starting subscriber. Press any key to stop it." << std::endl;

    SensorMsgSubscriber mysub;
    if(!mysub.init())
    {
	std::cerr << "Could not init the subscriber." << std::endl;
	return -1;
    }

    // do nothing here
    getchar();

    return 0;
}

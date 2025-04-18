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

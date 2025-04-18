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

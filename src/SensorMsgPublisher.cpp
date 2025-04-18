/**
 * @file SensorMsgPublisher.cpp
 *
 */

 #include "SensorMsgPublisher.h"

 #include <iostream>
 #include <chrono>
 #include <thread>
 #include "SafePrint.h" //safe printf in multi-threaded environment
 #include <fastdds/dds/domain/DomainParticipantFactory.hpp>
 
 using namespace eprosima::fastdds::dds;
 
 // PubListener implementation
 SensorMsgPublisher::PubListener::PubListener()
     : matched_(0)
 {
 }
 
 SensorMsgPublisher::PubListener::~PubListener() = default;
 
 void SensorMsgPublisher::PubListener::on_publication_matched(
     DataWriter*,
     const PublicationMatchedStatus& info)
 {
     if (info.current_count_change == 1)
     {
         matched_ = info.total_count;
         SafePrint::printf("Publisher matched...\n\r");
     }
     else if (info.current_count_change == -1)
     {
         matched_ = info.total_count;
         SafePrint::printf("Publisher unmatched.\n\r");
     }
     else
     {
         SafePrint::printf("%d is not a valid value for PublicationMatchedStatus current count change.\n\r", info.current_count_change);
     }
 }
 
 // Constructor
 SensorMsgPublisher::SensorMsgPublisher()
     : participant_(nullptr)
     , publisher_(nullptr)
     , topic_(nullptr)
     , writer_(nullptr)
     , type_(new SensorMsgPubSubType())
 {
 }
 
 // Destructor
 SensorMsgPublisher::~SensorMsgPublisher()
 {
     if (writer_ != nullptr)
     {
         publisher_->delete_datawriter(writer_);
     }
     if (publisher_ != nullptr)
     {
         participant_->delete_publisher(publisher_);
     }
     if (topic_ != nullptr)
     {
         participant_->delete_topic(topic_);
     }
     if (participant_ != nullptr)
     {
         DomainParticipantFactory::get_instance()->delete_participant(participant_);
     }
 }
 
 // Initialization
 bool SensorMsgPublisher::init()
 {
     DomainParticipantQos participantQos;
     participantQos.name("Participant_publisher");
     participant_ = DomainParticipantFactory::get_instance()->create_participant(0, participantQos);
 
     if (participant_ == nullptr)
     {
         return false;
     }
 
     type_.register_type(participant_);
 
     topic_ = participant_->create_topic("Topic", "SensorMsg", TOPIC_QOS_DEFAULT);
     if (topic_ == nullptr)
     {
         return false;
     }
 
     publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
     if (publisher_ == nullptr)
     {
         return false;
     }
 
     writer_ = publisher_->create_datawriter(topic_, DATAWRITER_QOS_DEFAULT, &listener_);
     if (writer_ == nullptr)
     {
         return false;
     }
 
     return true;
 }
 
 // Publish method
 bool SensorMsgPublisher::publish(SensorMsg& msg)
 {
     if (listener_.matched_ > 0)
     {
         writer_->write(&msg);
         return true;
     }
     return false;
 }
 

/*
int main(
    int,
    char**)
{
    std::cout << "Starting publisher." << std::endl;

    SensorMsgPublisher mypub;
    uint32_t samples_sent = 1;

    if(!mypub.init())
	{
	    std::cerr << "Pub not init'd." << std::endl;
	    return -1;
	}

    while(true){
    SensorMsg msg;
	msg.sensor_id(1);
    msg.temperature(25.00);
    msg.timestamp("Thu 17 Apr 04:56:37 BST 2025");
	if (mypub.publish(msg))
	{
	    std::cout << "Message: Sensor Id " << msg.sensor_id()<< " with temperature: " << msg.temperature()
		      << "at " << msg.timestamp() << " SENT" << std::endl;
	    samples_sent++;
	} else {
	    std::cout << "No messages sent as there is no listener." << std::endl;
	}

    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
      
    }
    }
*/

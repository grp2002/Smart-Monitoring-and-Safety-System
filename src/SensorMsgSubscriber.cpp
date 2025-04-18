/**
 * @file SensorMsgSubscriber.cpp
 *
 */

#include "SafePrint.h" //safe printf in multi-threaded environment
#include <QApplication>
#include <csignal>
#include "window.h"
#include "SensorMsgSubscriber.h"
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
 
 using namespace eprosima::fastdds::dds;
 
 // Constructor for the listener
 SensorMsgSubscriber::SubListener::SubListener(){}

 SensorMsgSubscriber::SubListener::SubListener(SensorMsgSubscriber* parent){
    parent_ = parent;
 }
 
 // Destructor for the listener
 SensorMsgSubscriber::SubListener::~SubListener(){}
 
 void SensorMsgSubscriber::SubListener::on_subscription_matched(
         DataReader*,
         const SubscriptionMatchedStatus& info)
 {
     if (info.current_count_change == 1)
     {
         SafePrint::printf("Subscriber matched.\n\r");
     }
     else if (info.current_count_change == -1)
     {
         SafePrint::printf("Subscriber unmatched.\n\r");
     }
     else
     {
         SafePrint::printf("%d is not a valid value for SubscriptionMatchedStatus current count change.\n\r",info.current_count_change);
     }
 }
 
 void SensorMsgSubscriber::SubListener::on_data_available(DataReader* reader)
 {
     SampleInfo info;
     SensorMsg msg;
     if (reader->take_next_sample(&msg, &info) == ReturnCode_t::RETCODE_OK)
     {
        /*
         if (info.valid_data)
         {
            SafePrint::printf("Message: Sensor Id {%d} with temperature: at {%f} RECEIVED on %s\n\r",msg.sensor_id(), msg.temperature(), msg.timestamp());
            onTemperatureRead(msg.sensor_id,msg.temperature);
         }
        */
        if (parent_ && parent_->onTemperatureRead)
        {
            parent_->onTemperatureRead(msg.sensor_id(), msg.temperature());
        }
     }
 }
 
 // Constructor
 SensorMsgSubscriber::SensorMsgSubscriber()
     : participant_(nullptr)
     , subscriber_(nullptr)
     , reader_(nullptr)
     , topic_(nullptr)
     , type_(new SensorMsgPubSubType())
     , listener_(nullptr)
 {
    listener_ = new SubListener(this);
 }
 
 // Destructor
 SensorMsgSubscriber::~SensorMsgSubscriber()
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
     if (participant_ != nullptr)
     {
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant_);
     }
     delete listener_; 
 }
 
 bool SensorMsgSubscriber::init()
 {
     DomainParticipantQos participantQos;
     participantQos.name("Participant_subscriber");
     participant_ = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(0, participantQos);
 
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
 
     subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
     if (subscriber_ == nullptr)
     {
         return false;
     }
 
     reader_ = subscriber_->create_datareader(topic_, DATAREADER_QOS_DEFAULT, listener_);
     if (reader_ == nullptr)
     {
         return false;
     }

    
 
     return true;
 }
 

 int main(int argc, char *argv[]) {

    SafePrint::printf("Starting subscriber...\n\r");

    SensorMsgSubscriber msgSubscriber;
    
    if(!msgSubscriber.init())
    {
	SafePrint::printf("Could not init the subscriber.\n\r");
	return -1;
    }

	QApplication app(argc, argv);
    
    Window sensor1Window, sensor2Window;
	sensor1Window.setWindowTitle("TMP117 Sensor 1 client");
	sensor2Window.setWindowTitle("TMP117 Sensor 2 client");

	// Position windows
	sensor1Window.move(400, 100);     // Top window
	sensor2Window.move(400, 500);     // Below the first window

	sensor1Window.show();
	sensor2Window.show();

    msgSubscriber.onTemperatureRead = [&](int sensor_id, double temperature) {
        if(sensor_id == 1)
            QMetaObject::invokeMethod(&sensor1Window, "updateTemperature", Qt::QueuedConnection,
                                    Q_ARG(double, temperature));
        else 
            QMetaObject::invokeMethod(&sensor2Window, "updateTemperature", Qt::QueuedConnection,
                                    Q_ARG(double, temperature));
    };

    return app.exec();
}

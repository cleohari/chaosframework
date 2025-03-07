/*
 * Copyright 2012, 2017 INFN
 *
 * Licensed under the EUPL, Version 1.2 or – as soon they
 * will be approved by the European Commission - subsequent
 * versions of the EUPL (the "Licence");
 * You may not use this work except in compliance with the
 * Licence.
 * You may obtain a copy of the Licence at:
 *
 * https://joinup.ec.europa.eu/software/page/eupl
 *
 * Unless required by applicable law or agreed to in
 * writing, software distributed under the Licence is
 * distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the Licence for the specific language governing
 * permissions and limitations under the Licence.
 */

#include <chaos/common/utility/TimingUtil.h>

#include <chaos/common/network/URLHAServiceFeeder.h>

#define RETRY_TIME 1000

#define URLHASF_INFO    INFO_LOG(URLHAServiceFeeder) << "["<<getName()<<"] - "
#define URLHASF_DBG     DBG_LOG(URLHAServiceFeeder) << "["<<getName()<<"] - "
#define URLHASF_ERR     ERR_LOG(URLHAServiceFeeder) << "["<<getName()<<"] - "

using namespace chaos::common::network;
using namespace chaos::common::utility;

URLHAServiceFeeder::URLHAServiceFeeder(std::string alias,
                                       URLServiceFeederHandler *_handler,
                                       URLHAServiceCheckerFeederHandler *_service_checker_handler):
URLServiceFeeder(alias,
                 _handler),
service_checker_handler(_service_checker_handler) {
    setRetryTime(1000, 10000);
}

URLHAServiceFeeder::~URLHAServiceFeeder() {}

void URLHAServiceFeeder::setRetryTime(const uint32_t _min_retry_time,
                                      const uint32_t _max_retry_time) {
    min_retry_time = _min_retry_time;
    max_retry_time = _max_retry_time;
}

uint32_t URLHAServiceFeeder::addURL(const URL& new_url,
                                    uint32_t priority) {
    return URLServiceFeeder::addURL(new_url,
                                    priority);
}

void URLHAServiceFeeder::removeURL(const URL& url_to_remove,
                                   bool dispose_service) {
    URLServiceFeeder::removeURL(URLServiceFeeder::getIndexFromURL(url_to_remove.getURL()),
                                dispose_service);
}


void URLHAServiceFeeder::clear(bool dispose_service) {
    URLServiceFeeder::clear(dispose_service);
}

void* URLHAServiceFeeder::getService() {
    //!befor return renable respauned services
    boost::unique_lock<boost::mutex> wr(mutex_queue);
    while (respawned_queue.size()) {
        URLServiceFeeder::setURLOnline(respawned_queue.front());
        respawned_queue.pop();
    }
    wr.unlock();
    return URLServiceFeeder::getService();
}

//! set url has offline
void URLHAServiceFeeder::setURLAsOffline(const std::string& remote_address) {
    boost::unique_lock<boost::mutex> wr(mutex_queue);
    uint32_t url_index = getIndexFromURL(remote_address);
    URLServiceFeeder::setURLOffline(url_index);
    ChaosSharedPtr<ServiceRetryInformation> sri(new ServiceRetryInformation(url_index,
                                                                               remote_address));
    sri->retry_timeout = TimingUtil::getTimeStamp()+min_retry_time;
    
    retry_queue.push(sri);
    
    URLHASF_INFO << "Service for  " << sri->offline_url << " gone offline check after " <<sri->retry_timeout;
}

//! set url has offline
void URLHAServiceFeeder::setIndexAsOffline(const uint32_t remote_index) {
    boost::unique_lock<boost::mutex> wr(mutex_queue);
    URLServiceFeeder::setURLOffline(remote_index);
    ChaosSharedPtr<ServiceRetryInformation> sri(new ServiceRetryInformation(remote_index,
                                                                               getURLForIndex(remote_index)));
    sri->retry_timeout = TimingUtil::getTimeStamp()+min_retry_time;
    retry_queue.push(sri);
    URLHASF_INFO << "Service for  " << sri->offline_url << " gone offline check after " <<sri->retry_timeout;
}

void URLHAServiceFeeder::checkForAliveService() {
    
    uint64_t current_ts = TimingUtil::getTimeStamp();
    boost::unique_lock<boost::mutex> wr(mutex_queue);
    size_t max_element = retry_queue.size();
    
    while (retry_queue.size() &&
           max_element-- > 0) {
        ChaosSharedPtr<ServiceRetryInformation> sri = retry_queue.front();retry_queue.pop();
        wr.unlock();
        if(current_ts>= sri->retry_timeout) {
            sri->retry_times++;
            URLHASF_INFO << "Check if service " << sri->offline_url << " has respawn";
            if(service_checker_handler->serviceOnlineCheck(URLServiceFeeder::getService(sri->offline_index))){
                //!service returned online
                URLHASF_INFO << "Service " << sri->offline_url << " returned online";
                wr.lock();
                respawned_queue.push(sri->offline_index);
            } else {
                //service still in offline
                sri->retry_timeout = TimingUtil::getTimeStamp() + ((sri->retry_times * 1000)%10000);
                
                URLHASF_INFO << "Service " << sri->offline_url << " still offline wait for " << sri->retry_timeout-current_ts << " milliseocnds";
                wr.lock();
                retry_queue.push(sri);
            }
        }else{
            wr.lock();
            retry_queue.push(sri);
        }
    }
}

size_t URLHAServiceFeeder::getOfflineSize() {
    boost::unique_lock<boost::mutex> wr(mutex_queue);
    return retry_queue.size();
}

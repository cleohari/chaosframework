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
#include <chaos/common/data/CDataWrapper.h>
#include <chaos/common/utility/InetUtility.h>
#include <chaos/common/utility/ObjectFactoryRegister.h>
#include <chaos/common/configuration/GlobalConfiguration.h>

#include <chaos/common/direct_io/DirectIOClient.h>
#include <chaos/common/direct_io/channel/DirectIOVirtualClientChannel.h>
#include <chaos/common/direct_io/DirectIOClientConnectionSharedMetricIO.h>
#include <chaos/common/direct_io/DirectIOClientConnectionMetricCollector.h>

#include <boost/regex.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

using namespace chaos::common::direct_io;
namespace chaos_data = chaos::common::data;

namespace b_algo = boost::algorithm;

#define DIO_LOG_HEAD "["<<getName()<<"] - "

#define DIOLAPP_ INFO_LOG(DirectIOClient)
#define DIOLDBG_ DBG_LOG(DirectIOClient)
#define DIOLERR_ ERR_LOG(DirectIOClient)

DirectIOClient::DirectIOClient(std::string alias):
NamedService(alias),
map_connections(this){
    
}

DirectIOClient::~DirectIOClient() {
}

void DirectIOClient::forwardEventToClientConnection(DirectIOClientConnection *client,
                                                    DirectIOClientConnectionStateType::DirectIOClientConnectionStateType event_type) {
    client->lowLevelManageEvent(event_type);
}

//! Initialize instance
void DirectIOClient::init(void *init_data)  {
    
}


//! Deinit the implementation
void DirectIOClient::deinit()  {
    map_shared_collectors.clear();
}

DirectIOClientConnection *DirectIOClient::getNewConnection(const std::string& server_description_with_endpoint) {
    uint16_t endpoint;
    std::string server_description;
    DirectIOClientConnection *result = NULL;
    DEBUG_CODE(DIOLDBG_ << "Requested a new connection for a server description " << server_description_with_endpoint;)
    if(decodeServerDescriptionWithEndpoint(server_description_with_endpoint, server_description, endpoint)) {
        DEBUG_CODE(DIOLDBG_ << "scomposed into server description " << server_description << " and endpoint " << endpoint;)
        
        result = _getNewConnectionImpl(server_description,
                                       endpoint);
        
        if(result &&
           GlobalConfiguration::getInstance()->getConfiguration()->hasKey(InitOption::OPT_DIRECT_IO_LOG_METRIC) &&
           GlobalConfiguration::getInstance()->getConfiguration()->getBoolValue(InitOption::OPT_DIRECT_IO_LOG_METRIC)) {
            //lock the map
            boost::unique_lock<boost::mutex> wl(mutex_map_shared_collectors);
            
            //prepare key in case with need to split themetric for endpoint
            std::string server_key = server_description;
            if(GlobalConfiguration::getInstance()->getConfiguration()->getBoolValue(InitOption::OPT_DIRECT_IO_CLIENT_LOG_METRIC_MERGED_ENDPOINT)) {
                //we need to merge the endpoint
                server_key = "merge_together";
            } else {
                server_key = server_description_with_endpoint;
            }
            //create the collector key
            SharedCollectorKey key(server_key);
            ChaosSharedPtr<DirectIOClientConnectionSharedMetricIO> shared_collector;
            
            //check if we have already allcoated thecollector
            if(map_shared_collectors.count(key)==0) {
                shared_collector.reset(new DirectIOClientConnectionSharedMetricIO(getName(), server_description_with_endpoint));
                map_shared_collectors.insert(make_pair(key, shared_collector));
            } else {
                shared_collector = map_shared_collectors[key];
            }
            
            //the metric allocator of direct io is a direct subclass of DirectIODispatcher
            result = new DirectIOClientConnectionMetricCollector(server_description,
                                                                 endpoint,
                                                                 shared_collector,
                                                                 result);
        }
        
    }
    return result;
}

//! Release the connection
void DirectIOClient::releaseConnection(DirectIOClientConnection *connection_to_release) {
    if(connection_to_release) {
        if(GlobalConfiguration::getInstance()->getConfiguration()->getBoolValue(InitOption::OPT_DIRECT_IO_LOG_METRIC)) {
            //the metric allocator of direct io is a direct subclass of DirectIODispatcher
            DirectIOClientConnectionMetricCollector *metric_collector = dynamic_cast<DirectIOClientConnectionMetricCollector*>(connection_to_release);
            if(metric_collector) {
                _releaseConnectionImpl(metric_collector->wrapped_connection);
                delete(metric_collector);
            }
        } else  {
            _releaseConnectionImpl(connection_to_release);
        }
    }
}

void DirectIOClient::freeObject(const DCKeyObjectContainer::TKOCElement& element) {
    
}

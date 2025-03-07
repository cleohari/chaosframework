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

#ifndef __CHAOSFramework__DirectIOClientConnectionSharedMetricIO__
#define __CHAOSFramework__DirectIOClientConnectionSharedMetricIO__

#include <chaos/common/metric/metric.h>
namespace chaos {
    namespace common {
        namespace direct_io {
            class DirectIOClientConnectionSharedMetricIO:
            public chaos::common::metric::MetricCollectorIO {
            protected:
                void fetchMetricForTimeDiff(uint64_t time_diff);
            public:
                DirectIOClientConnectionSharedMetricIO(const std::string& client_impl,
                                                       const std::string& server_endpoint);
                ~DirectIOClientConnectionSharedMetricIO();
                
                void incrementPackCount();
                void incrementBandWidth(uint64_t increment);
            };
        }
    }
}

#endif /* defined(__CHAOSFramework__DirectIOClientConnectionSharedMetricIO__) */

find_library(NEWRELIC_TRANSACTION_LIBRARY NAMES newrelic-transaction PATHS /usr/lib /usr/local/lib)
find_library(NEWRELIC_COLLECTOR_CLIENT_LIBRARY NAMES newrelic-collector-client PATHS /usr/lib /usr/local/lib)
find_library(NEWRELIC_COMMON_LIBRARY NAMES newrelic-common PATHS /usr/lib /usr/local/lib)

include_directories(/usr/include)
include_directories(/usr/local/include)

HHVM_EXTENSION(newrelic newrelic.cpp)
HHVM_SYSTEMLIB(newrelic newrelic.php)

target_link_libraries(newrelic
	${NEWRELIC_TRANSACTION_LIBRARY}
	${NEWRELIC_COMMON_LIBRARY}
	${NEWRELIC_COLLECTOR_CLIENT_LIBRARY}

)

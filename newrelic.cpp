#include "hphp/runtime/base/base-includes.h"
#include "newrelic_transaction.h"
#include "newrelic_collector_client.h"
#include "newrelic_common.h"

#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <thread>
#include <unistd.h>

using namespace std;

namespace HPHP {

bool keep_running = true;

class ScopedGenericSegment : public SweepableResourceData {
public:
	DECLARE_RESOURCE_ALLOCATION(ScopedGenericSegment)
	CLASSNAME_IS("scoped_generic_segment")

	virtual const String& o_getClassNameHook() const { return classnameof(); }

	explicit ScopedGenericSegment(string name) : name(name) {
		segment_id = newrelic_segment_generic_begin(NEWRELIC_AUTOSCOPE, NEWRELIC_AUTOSCOPE, name.c_str());
	}

	virtual ~ScopedGenericSegment() {
		if (segment_id < 0) return;
		newrelic_segment_end(NEWRELIC_AUTOSCOPE, segment_id);
	}

private:
	long segment_id;
	string name;
};

void ScopedGenericSegment::sweep() { }

class ScopedDatastoreSegment : public SweepableResourceData {
public:
	DECLARE_RESOURCE_ALLOCATION(ScopedDatastoreSegment)
	CLASSNAME_IS("scoped_database_segment")

	virtual const String& o_getClassNameHook() const { return classnameof(); }

	explicit ScopedDatastoreSegment(string table, string operation) : table(table), operation(operation) {
		segment_id = newrelic_segment_datastore_begin(NEWRELIC_AUTOSCOPE, NEWRELIC_AUTOSCOPE, table.c_str(), operation.c_str());
	}

	virtual ~ScopedDatastoreSegment() {
		if (segment_id < 0) return;
		newrelic_segment_end(NEWRELIC_AUTOSCOPE, segment_id);
	}

private:
	long segment_id;
	string table;
	string operation;
};

void ScopedDatastoreSegment::sweep() { }

static int64_t HHVM_FUNCTION(hhvm_newrelic_transaction_begin) {
	long transaction_id = newrelic_transaction_begin();
	return transaction_id;
}

static int HHVM_FUNCTION(hhvm_newrelic_transaction_set_name, const String & name) {
	return newrelic_transaction_set_name(NEWRELIC_AUTOSCOPE, name.c_str());
}

static int HHVM_FUNCTION(hhvm_newrelic_transaction_set_request_url, const String & request_url) {
	return newrelic_transaction_set_request_url(NEWRELIC_AUTOSCOPE, request_url.c_str());
}

static int HHVM_FUNCTION(hhvm_newrelic_transaction_set_threshold, int threshold) {
	return newrelic_transaction_set_threshold(NEWRELIC_AUTOSCOPE, threshold);
}

static int HHVM_FUNCTION(hhvm_newrelic_transaction_end) {
	return newrelic_transaction_end(NEWRELIC_AUTOSCOPE);
}

static int64_t HHVM_FUNCTION(hhvm_newrelic_segment_generic_begin, const String & name) {
	return newrelic_segment_generic_begin(NEWRELIC_AUTOSCOPE, NEWRELIC_AUTOSCOPE, name.c_str());
}

static int64_t HHVM_FUNCTION(hhvm_newrelic_segment_datastore_begin, const String & table, const String & operation) {
	return newrelic_segment_datastore_begin(NEWRELIC_AUTOSCOPE, NEWRELIC_AUTOSCOPE, table.c_str(), operation.c_str());
}

static int HHVM_FUNCTION(hhvm_newrelic_segment_end, int64_t id) {
	return newrelic_segment_end(NEWRELIC_AUTOSCOPE, id);
}

static Variant HHVM_FUNCTION(hhvm_newrelic_get_scoped_generic_segment, const String & name) {
	ScopedGenericSegment * segment = nullptr;
	segment = NEWOBJ(ScopedGenericSegment)(name.c_str());
	return Resource(segment);
}

static Variant HHVM_FUNCTION(hhvm_newrelic_get_scoped_database_segment, const String & table, const String & operation) {
	ScopedDatastoreSegment * segment = nullptr;
	segment = NEWOBJ(ScopedDatastoreSegment)(table.c_str(), operation.c_str());
	return Resource(segment);
}

static class NewRelicExtension : public Extension {
public:
	NewRelicExtension () : Extension("newrelic") {
		config_loaded = false;
	}

	virtual void init_newrelic() {
		newrelic_register_message_handler(newrelic_message_handler);
		newrelic_init(license_key.c_str(), app_name.c_str(), app_language.c_str(), app_language_version.c_str());
	}

	virtual void moduleLoad(Hdf config) {
		if (!config.exists("EnvVariables")) return;

		Hdf env_vars = config["EnvVariables"];

		// Make any environment variables set in the hdf file available to the
		// new relic libraries if the names start with NEWRELIC
		for (Hdf child = env_vars.firstChild(); child.exists(); child = child.next()) {
			std::string name = child.getName();
			std::string val = env_vars[name].getString();
			std::string newrelic_namespace("NEWRELIC");

			if (name.compare(0, newrelic_namespace.length(), newrelic_namespace) == 0) {
				if (name == "NEWRELIC_LICENSE_KEY") {
					license_key = val;
				} else if (name == "NEWRELIC_APP_NAME") {
					app_name = val;
				} else if (name == "NEWRELIC_APP_LANGUAGE") {
					app_language = val;
				} else if (name == "NEWRELIC_APP_LANGUAGE_VERSION") {
					app_language_version = val;
				}

				// make the environment variable accessible elsewhere...
				setenv(name.c_str(), val.c_str(), 1);
			}

		}

		if (!license_key.empty() && !app_name.empty() && !app_language.empty() && !app_language_version.empty())
			config_loaded = true;

	}

	virtual void moduleInit () {
		if (config_loaded) init_newrelic();

		HHVM_FE(hhvm_newrelic_transaction_begin);
		HHVM_FE(hhvm_newrelic_transaction_set_name);
		HHVM_FE(hhvm_newrelic_transaction_set_request_url);
		HHVM_FE(hhvm_newrelic_transaction_set_threshold);
		HHVM_FE(hhvm_newrelic_transaction_end);
		HHVM_FE(hhvm_newrelic_segment_generic_begin);
		HHVM_FE(hhvm_newrelic_segment_datastore_begin);
		HHVM_FE(hhvm_newrelic_segment_end);
		HHVM_FE(hhvm_newrelic_get_scoped_generic_segment);
		HHVM_FE(hhvm_newrelic_get_scoped_database_segment);

		loadSystemlib();
	}

private:
	std::string license_key;
	std::string app_name;
	std::string app_language;
	std::string app_language_version;

	bool config_loaded;

} s_newrelic_extension;

HHVM_GET_MODULE(newrelic)

} // namespace HPHP

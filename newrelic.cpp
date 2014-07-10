#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/std/ext_std_errorfunc.h"
#include "hphp/runtime/base/php-globals.h"
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
    int64_t segment_id;
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
    int64_t segment_id;
    string table;
    string operation;
};

void ScopedDatastoreSegment::sweep() { }


class ScopedExternalSegment : public SweepableResourceData {
public:
    DECLARE_RESOURCE_ALLOCATION(ScopedExternalSegment)
    CLASSNAME_IS("scoped_external_segment")

    virtual const String& o_getClassNameHook() const { return classnameof(); }

    explicit ScopedExternalSegment(string host, string name) : host(host), name(name) {
        segment_id = newrelic_segment_external_begin(NEWRELIC_AUTOSCOPE, NEWRELIC_AUTOSCOPE, host.c_str(), name.c_str());
    }

    virtual ~ScopedExternalSegment() {
        if (segment_id < 0) return;
        newrelic_segment_end(NEWRELIC_AUTOSCOPE, segment_id);
    }

private:
    int64_t segment_id;
    string host;
    string name;
};

void ScopedExternalSegment::sweep() { }


static int64_t HHVM_FUNCTION(newrelic_start_transaction_intern) {
    int64_t transaction_id = newrelic_transaction_begin();
    return transaction_id;
}

static int64_t HHVM_FUNCTION(newrelic_name_transaction_intern, const String & name) {
    return newrelic_transaction_set_name(NEWRELIC_AUTOSCOPE, name.c_str());
}

static int64_t HHVM_FUNCTION(newrelic_transaction_set_request_url, const String & request_url) {
    return newrelic_transaction_set_request_url(NEWRELIC_AUTOSCOPE, request_url.c_str());
}

static int64_t HHVM_FUNCTION(newrelic_transaction_set_max_trace_segments, int threshold) {
    return newrelic_transaction_set_max_trace_segments(NEWRELIC_AUTOSCOPE, threshold);
}

static int64_t HHVM_FUNCTION(newrelic_transaction_set_threshold, int threshold) {
    //deprecated
    return false;
}

static int64_t HHVM_FUNCTION(newrelic_end_transaction) {
    return newrelic_transaction_end(NEWRELIC_AUTOSCOPE);
}

static int64_t HHVM_FUNCTION(newrelic_segment_generic_begin, const String & name) {
    return newrelic_segment_generic_begin(NEWRELIC_AUTOSCOPE, NEWRELIC_AUTOSCOPE, name.c_str());
}

static int64_t HHVM_FUNCTION(newrelic_segment_datastore_begin, const String & table, const String & operation) {
    return newrelic_segment_datastore_begin(NEWRELIC_AUTOSCOPE, NEWRELIC_AUTOSCOPE, table.c_str(), operation.c_str());
}

static int64_t HHVM_FUNCTION(newrelic_segment_external_begin, const String & host, const String & name) {
    return newrelic_segment_external_begin(NEWRELIC_AUTOSCOPE, NEWRELIC_AUTOSCOPE, host.c_str(), name.c_str());
}

static int64_t HHVM_FUNCTION(newrelic_segment_end, int64_t id) {
    return newrelic_segment_end(NEWRELIC_AUTOSCOPE, id);
}

static int64_t HHVM_FUNCTION(newrelic_notice_error_intern, const String & exception_type,  const String & error_message,  const String & stack_trace,  const String & stack_frame_delimiter) {
    return newrelic_transaction_notice_error(NEWRELIC_AUTOSCOPE, exception_type.c_str(), error_message.c_str(), stack_trace.c_str(), stack_frame_delimiter.c_str());
}

static int64_t HHVM_FUNCTION(newrelic_add_attribute_intern, const String & name, const String & value) {
    return newrelic_transaction_add_attribute(NEWRELIC_AUTOSCOPE, name.c_str(), value.c_str());
}

static Variant HHVM_FUNCTION(newrelic_get_scoped_generic_segment, const String & name) {
    ScopedGenericSegment * segment = nullptr;
    segment = NEWOBJ(ScopedGenericSegment)(name.c_str());
    return Resource(segment);
}

static Variant HHVM_FUNCTION(newrelic_get_scoped_database_segment, const String & table, const String & operation) {
    ScopedDatastoreSegment * segment = nullptr;
    segment = NEWOBJ(ScopedDatastoreSegment)(table.c_str(), operation.c_str());
    return Resource(segment);
}

static Variant HHVM_FUNCTION(newrelic_get_scoped_external_segment, const String & host, const String & name) {
    ScopedExternalSegment * segment = nullptr;
    segment = NEWOBJ(ScopedExternalSegment)(host.c_str(), name.c_str());
    return Resource(segment);
}

const StaticString
  s__NR_ERROR_CALLBACK("NewRelicExtensionHelper::errorCallback"),
  s__NR_EXCEPTION_CALLBACK("NewRelicExtensionHelper::exceptionCallback"),
  s__SERVER("_SERVER"),
  s__REQUEST_URI("REQUEST_URI"),
  s__SCRIPT_NAME("SCRIPT_NAME");

static class NewRelicExtension : public Extension {
public:
    NewRelicExtension () : Extension("newrelic") {
        config_loaded = false;
    }

    virtual void init_newrelic() {
        newrelic_register_message_handler(newrelic_message_handler);
        newrelic_init(license_key.c_str(), app_name.c_str(), app_language.c_str(), app_language_version.c_str());
    }

    virtual void moduleLoad(const IniSetting::Map& ini, Hdf config) {
        if (!config.exists("EnvVariables")) return;

        Hdf env_vars = config["EnvVariables"];

        license_key = Config::GetString(ini, env_vars["NEWRELIC_LICENSE_KEY"]);
        app_name = Config::GetString(ini, env_vars["NEWRELIC_APP_NAME"]);
        app_language = Config::GetString(ini, env_vars["NEWRELIC_APP_LANGUAGE"]);
        app_language_version = Config::GetString(ini, env_vars["NEWRELIC_APP_LANGUAGE_VERSION"]);

        setenv("NEWRELIC_LICENSE_KEY", license_key.c_str(), 1);
        setenv("NEWRELIC_APP_NAME", app_name.c_str(), 1);
        setenv("NEWRELIC_APP_LANGUAGE", app_language.c_str(), 1);
        setenv("NEWRELIC_APP_LANGUAGE_VERSION", app_language_version.c_str(), 1);

        if (!license_key.empty() && !app_name.empty() && !app_language.empty() && !app_language_version.empty())
            config_loaded = true;

    }


    virtual void moduleInit () {
        if (config_loaded) init_newrelic();

        HHVM_FE(newrelic_start_transaction_intern);
        HHVM_FE(newrelic_name_transaction_intern);
        HHVM_FE(newrelic_transaction_set_request_url);
        HHVM_FE(newrelic_transaction_set_max_trace_segments);
        HHVM_FE(newrelic_transaction_set_threshold);
        HHVM_FE(newrelic_end_transaction);
        HHVM_FE(newrelic_segment_generic_begin);
        HHVM_FE(newrelic_segment_datastore_begin);
        HHVM_FE(newrelic_segment_external_begin);
        HHVM_FE(newrelic_segment_end);
        HHVM_FE(newrelic_get_scoped_generic_segment);
        HHVM_FE(newrelic_get_scoped_database_segment);
        HHVM_FE(newrelic_get_scoped_external_segment);
        HHVM_FE(newrelic_notice_error_intern);
        HHVM_FE(newrelic_add_attribute_intern);

        loadSystemlib();
    }

    virtual void requestShutdown() {

        newrelic_transaction_end(NEWRELIC_AUTOSCOPE);
    }

    virtual void requestInit() {
        f_set_error_handler(s__NR_ERROR_CALLBACK);
        f_set_exception_handler(s__NR_EXCEPTION_CALLBACK);
        //TODO: make it possible to disable that via ini
        newrelic_transaction_begin();
        String request_url = php_global(s__SERVER).toArray()[s__REQUEST_URI].toString();
        newrelic_transaction_set_request_url(NEWRELIC_AUTOSCOPE, request_url.c_str());
        String script_name = php_global(s__SERVER).toArray()[s__SCRIPT_NAME].toString();
        newrelic_transaction_set_name(NEWRELIC_AUTOSCOPE, script_name.c_str());
    }

private:
    std::string license_key;
    std::string app_name;
    std::string app_language;
    std::string app_language_version;
    int64_t global_transaction_id = 0;
    bool config_loaded;

} s_newrelic_extension;

HHVM_GET_MODULE(newrelic)

} // namespace HPHP

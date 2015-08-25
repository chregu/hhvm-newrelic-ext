#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/std/ext_std_errorfunc.h"
#include "hphp/runtime/base/php-globals.h"
#include "hphp/runtime/base/hphp-system.h"
#include "hphp/runtime/ext/hotprofiler/ext_hotprofiler.h"
#include "newrelic_transaction.h"
#include "newrelic_collector_client.h"
#include "newrelic_common.h"
#include "newrelic_profiler.h"
#include "hphp/runtime/server/transport.h"

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

    explicit ScopedDatastoreSegment(string table, string operation, string sql, string sql_trace_rollup_name) : table(table), operation(operation), sql(sql), sql_trace_rollup_name(sql_trace_rollup_name) {
        //TODO sql_trace_rollup_name
        if (sql_trace_rollup_name == "") {
            segment_id = newrelic_segment_datastore_begin(NEWRELIC_AUTOSCOPE, NEWRELIC_AUTOSCOPE, table.c_str(), operation.c_str(), sql.c_str(), NULL, NULL);
        } else {
            segment_id = newrelic_segment_datastore_begin(NEWRELIC_AUTOSCOPE, NEWRELIC_AUTOSCOPE, table.c_str(), operation.c_str(), sql.c_str(), sql_trace_rollup_name.c_str(), NULL);
        }
    }

    virtual ~ScopedDatastoreSegment() {
        if (segment_id < 0) return;
        newrelic_segment_end(NEWRELIC_AUTOSCOPE, segment_id);
    }

private:
    int64_t segment_id;
    string table;
    string operation;
    string sql;
    string sql_trace_rollup_name;
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

// Profiler factory- for starting and stopping the profiler
DECLARE_EXTERN_REQUEST_LOCAL(ProfilerFactory, s_profiler_factory);

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

static int64_t HHVM_FUNCTION(newrelic_segment_datastore_begin, const String & table, const String & operation, const String & sql, const String & sql_trace_rollup_name, const String & sql_obfuscator) {
    return newrelic_segment_datastore_begin(NEWRELIC_AUTOSCOPE, NEWRELIC_AUTOSCOPE, table.c_str(), operation.c_str(), sql.c_str(), sql_trace_rollup_name.c_str(), NULL);
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

static int64_t HHVM_FUNCTION(newrelic_custom_metric, const String & name, double value) {
    return newrelic_record_metric(name.c_str(), value);
}

static void HHVM_FUNCTION(newrelic_set_external_profiler, int64_t maxdepth ) {
    Profiler *pr = new NewRelicProfiler(maxdepth);
    s_profiler_factory->setExternalProfiler(pr);
}

static Variant HHVM_FUNCTION(newrelic_get_scoped_generic_segment, const String & name) {
    ScopedGenericSegment * segment = nullptr;
    // NEWOBJ existsonly until HHVM 3.4
    #ifdef NEWOBJ
        segment = NEWOBJ(ScopedGenericSegment)(name.c_str());
    #else
        segment = newres<ScopedGenericSegment>(name.c_str());
    #endif
    return Resource(segment);
}

static Variant HHVM_FUNCTION(newrelic_get_scoped_database_segment, const String & table, const String & operation, const String & sql, const String & sql_trace_rollup_name) {
    ScopedDatastoreSegment * segment = nullptr;
    // NEWOBJ existsonly until HHVM 3.4
    #ifdef NEWOBJ
        segment = NEWOBJ(ScopedDatastoreSegment)(table.c_str(), operation.c_str(), sql.c_str(), sql_trace_rollup_name.c_str());
    #else
        segment = newres<ScopedDatastoreSegment>(table.c_str(), operation.c_str(), sql.c_str(), sql_trace_rollup_name.c_str());
    #endif
    return Resource(segment);
}

static Variant HHVM_FUNCTION(newrelic_get_scoped_external_segment, const String & host, const String & name) {
    ScopedExternalSegment * segment = nullptr;
    // NEWOBJ existsonly until HHVM 3.4
    #ifdef NEWOBJ
        segment = NEWOBJ(ScopedExternalSegment)(host.c_str(), name.c_str());
    #else
        segment = newres<ScopedExternalSegment>(host.c_str(), name.c_str());
    #endif
    return Resource(segment);
}

const StaticString
  s__NR_ERROR_CALLBACK("NewRelicExtensionHelper::errorCallback"),
  s__NR_EXCEPTION_CALLBACK("NewRelicExtensionHelper::exceptionCallback");

static class NewRelicExtension : public Extension {
public:
    NewRelicExtension () : Extension("newrelic", NO_EXTENSION_VERSION_YET) {
        config_loaded = false;
    }

    virtual void init_newrelic() {
        newrelic_register_message_handler(newrelic_message_handler);
        newrelic_init(license_key.c_str(), app_name.c_str(), app_language.c_str(), app_language_version.c_str());
    }

    virtual void moduleLoad(const IniSetting::Map& ini, Hdf config) {

        license_key = RuntimeOption::EnvVariables["NEWRELIC_LICENSE_KEY"];
        app_name = RuntimeOption::EnvVariables["NEWRELIC_APP_NAME"];
        app_language = RuntimeOption::EnvVariables["NEWRELIC_APP_LANGUAGE"];
        app_language_version = RuntimeOption::EnvVariables["NEWRELIC_APP_LANGUAGE_VERSION"];

        if (app_language.empty()) {
            app_language = "php-hhvm";
        }

        if (app_language_version.empty()) {
            app_language_version = HPHP::getHphpCompilerVersion();
        }


        setenv("NEWRELIC_LICENSE_KEY", license_key.c_str(), 1);
        setenv("NEWRELIC_APP_NAME", app_name.c_str(), 1);
        setenv("NEWRELIC_APP_LANGUAGE", app_language.c_str(), 1);
        setenv("NEWRELIC_APP_LANGUAGE_VERSION", app_language_version.c_str(), 1);

        if (!license_key.empty() && !app_name.empty() && !app_language.empty() && !app_language_version.empty())
            config_loaded = true;

    }


    void moduleInit () override {
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
        HHVM_FE(newrelic_set_external_profiler);

        loadSystemlib();
    }

    virtual void requestShutdown() {

        newrelic_transaction_end(NEWRELIC_AUTOSCOPE);
    }

    void requestInit() override {
        auto serverVars = php_global(s__SERVER).toArray();

        f_set_error_handler(s__NR_ERROR_CALLBACK);
        f_set_exception_handler(s__NR_EXCEPTION_CALLBACK);
        //TODO: make it possible to disable that via ini
        newrelic_transaction_begin();
        String request_url = serverVars[s__REQUEST_URI].toString();
        String https = serverVars[s__HTTPS].toString();
        String http_host = serverVars[s__HTTP_HOST].toString();
        String script_name = serverVars[s__SCRIPT_NAME].toString();
        String query_string = serverVars[s__QUERY_STRING].toString();
        String full_uri;

        if (request_url == s__EMPTY) {
            full_uri = script_name;
        } else {
            if (https == s__EMPTY) {
                full_uri = s__PROTO_HTTP;
            } else {
                full_uri = s__PROTO_HTTPS;
            }
            full_uri += http_host + request_url;
        }

        newrelic_transaction_set_request_url(NEWRELIC_AUTOSCOPE, full_uri.c_str());
        //set request_url strips query parameter, add a custom attribute with the full param
        if (query_string != s__EMPTY) {
            newrelic_transaction_add_attribute(NEWRELIC_AUTOSCOPE, "FULL_URL", full_uri.c_str());
        }

        //build transaction name
        String transaction_name = request_url == s__EMPTY ? script_name : request_url;
        size_t get_param_loc = transaction_name.find('?');
        if(get_param_loc != string::npos) {
            transaction_name = transaction_name.substr(0, get_param_loc);
        }

        newrelic_transaction_set_name(NEWRELIC_AUTOSCOPE, transaction_name.c_str());

        // add http request headers to transaction attributes
        Transport *transport = g_context->getTransport();
        if (transport) {
            HeaderMap headers;
            transport->getHeaders(headers);
            for (auto& iter : headers) {
                const auto& values = iter.second;
                if (!values.size()) {
                    continue;
                }
                if (iter.first == "User-Agent") {
                    newrelic_transaction_add_attribute(NEWRELIC_AUTOSCOPE, "request.headers.User-Agent", values.back().c_str());
                } else if (iter.first == "Accept") {
                    newrelic_transaction_add_attribute(NEWRELIC_AUTOSCOPE, "request.headers.Accept", values.back().c_str());
                } else if (iter.first == "Accept-Language") {
                    newrelic_transaction_add_attribute(NEWRELIC_AUTOSCOPE, "request.headers.Accept-Language", values.back().c_str());
                } else if (iter.first == "Api-Version") {
                    newrelic_transaction_add_attribute(NEWRELIC_AUTOSCOPE, "request.headers.Api-Version", values.back().c_str());
                }
            }
        }

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

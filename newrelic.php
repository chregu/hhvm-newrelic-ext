<?hh

//not implemented yet
function newrelic_set_appname(string $name, string $key, bool $xmit): mixed {}

//not implemented yet
function newrelic_custom_metric(string $name, float $value) {}

//not implemented yet
function newrelic_add_custom_parameter(string $name, string $value) {}

//not implemented yet
function newrelic_disable_autorum() {}

//not implemented yet
function newrelic_notice_error(string $message, \Exception $e = null)  {}

//not implemented yet
function newrelic_background_job(bool $true) {}

function newrelic_start_transaction(string $appname, string $license = null) {
    newrelic_start_transaction_intern();
    newrelic_transaction_set_request_url($_SERVER["REQUEST_URI"]);
}

function newrelic_name_transaction(string $name) {
    newrelic_name_transaction_intern($name);
    newrelic_transaction_set_request_url($_SERVER["REQUEST_URI"]);
}

function newrelic_profiling_enable(int $level) {
    if (function_exists("newrelic_hotprofiling_enabled_intern")) {
        xhprof_enable(256,array(0 => $level));
    } else {
        NewRelicExtensionHelper::setMaxDepth($level);
        fb_setprofile(array("NewRelicExtensionHelper","profile"));
    }
}

function newrelic_profiling_disable() {
    if (function_exists("newrelic_hotprofiling_enabled_intern")) {
        xhprof_disable();
    } else {
        NewRelicExtensionHelper::endAll();
        fb_setprofile(null);
    }
}
    
    
class NewRelicExtensionHelper {

    protected static Vector<int> $stack = Vector {};
    // there's an issue with the depth, if you enable/disable it at different depths... have to figure something out
    protected static int $depth =  0;
    protected static int $maxdepth = 7;
    
    static function profile (string $mode, string $name, array $options = null): void {
        if ($name) {
            if ($mode == 'enter')  {
                if (self::$depth < self::$maxdepth) {
                    self::$stack->add(newrelic_segment_generic_begin($name));
                } else {
                    self::$stack->add(0);
                }
                self::$depth++;
            } else {
                $id =  self::$stack->pop();
                if ($id) {
                    newrelic_segment_end($id);
                }
                self::$depth--;
            }
        }
    }
    
    static function setMaxDepth(int $depth): void {
        self::$maxdepth = $depth;
    }
    
    static function endAll(): void {
        while (self::$stack->count()) {
            $id = self::$stack->pop();
            if ($id) {
                 newrelic_segment_end($id);
            }
        }
        self::$depth = 0;
    }
}


<<__Native>>
function newrelic_start_transaction_intern(): int;

<<__Native>>
function newrelic_name_transaction_intern(string $name): int;

<<__Native>>
function newrelic_transaction_set_request_url(string $name): int;

<<__Native>>
function newrelic_transaction_set_threshold(int $threshold): int;

<<__Native>>
function newrelic_end_transaction(): int;

<<__Native>>
function newrelic_segment_generic_begin(string $name): int;

<<__Native>>
function newrelic_segment_datastore_begin(string $table, string $operation): int;

<<__Native>>
function newrelic_segment_end(int $id): int;

<<__Native>>
function newrelic_get_scoped_generic_segment(string $name): mixed;

<<__Native>>
function newrelic_get_scoped_database_segment(string $table, string $operation): mixed;

<<__Native>>
function newrelic_transaction_set_max_trace_segments(int $threshold): int;




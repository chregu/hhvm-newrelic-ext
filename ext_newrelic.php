<?hh

//not implemented yet
function newrelic_set_appname(string $name, string $key, bool $xmit): mixed {}

//not implemented yet
function newrelic_custom_metric(string $name, float $value) {}

//The same as newrelic_add_attribute, but like in the officical NewRelic PHP API
function newrelic_add_custom_parameter(string $name, string $value) {
    newrelic_add_attribute_intern($name, $value);
}

//not implemented yet
function newrelic_disable_autorum() {}

function newrelic_notice_error(?string $error_message, \Exception $e = null)  {
    if ($e) {
        if (!$error_message) {
            $error_message = $e->getMessage();
        }
        $exception_type = get_class($e);
        $stack_trace = $e->getTraceAsString();
    } else {
        $exception_type = "";
        $stack_trace = NewRelicExtensionHelper::debug_backtrace_string();
    }
        $stack_frame_delimiter = "\n";
    newrelic_notice_error_intern( $exception_type,  $error_message,  $stack_trace,  $stack_frame_delimiter);
}

//not implemented yet
function newrelic_background_job(bool $true) {}

function newrelic_transaction_begin(): int {
    return newrelic_start_transaction();
}

function newrelic_transaction_end(): int {
    return newrelic_end_transaction();
}

function newrelic_start_transaction(string $appname = null, string $license = null): int {
    $id = newrelic_start_transaction_intern();
    newrelic_transaction_set_request_url($_SERVER["REQUEST_URI"]);
    return $id;
}

function newrelic_name_transaction(string $name) {
    newrelic_name_transaction_intern($name);
    newrelic_transaction_set_request_url($_SERVER["REQUEST_URI"]);
}

function newrelic_transaction_set_name(string $name) {
    newrelic_name_transaction($name);
}

//not implemented yet
function newrelic_ignore_transaction() {}

//not implemented yet
function newrelic_ignore_apdex() {}

function newrelic_profiling_enable(int $level) {
    if (function_exists("newrelic_hotprofiling_enabled_intern")) {
        xhprof_enable(0x800,array(0 => $level));
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

//not implemented yet
function newrelic_capture_params($enable) {}

//not implemented yet
function newrelic_get_browser_timing_header($flag) {}

//not implemented yet
function newrelic_get_browser_timing_footer($flag) {}

//not implemented yet
function newrelic_set_user_attributes($user, $account, $product) {}

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
                    //self::$stack->add(0);
                }
                self::$depth++;
            } else {
                if (self::$depth < self::$maxdepth + 1) {
                    try {
                        $id =  self::$stack->pop();
                        if ($id) {
                            newrelic_segment_end($id);
                        }
                    } catch (Exception $e) {}
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

    static function errorCallback($type, $message, $c) {
        $exception_type = self::friendlyErrorType($type);
        $error_message = $message;
        $stack_trace = self::debug_backtrace_string();
        $stack_frame_delimiter = "\n";
        newrelic_notice_error_intern( $exception_type,  $error_message,  $stack_trace,  $stack_frame_delimiter);
        return false;
    }

    static function exceptionCallback($e) {
        $exception_type = get_class($e);
        $error_message = $e->getMessage();
        $stack_trace = $e->getTraceAsString();
        $stack_frame_delimiter = "\n";
        newrelic_notice_error_intern( $exception_type,  $error_message,  $stack_trace,  $stack_frame_delimiter);
    }

    static function debug_backtrace_string() {
        $stack = '';
        $i = 1;
        $trace = debug_backtrace();
        unset($trace[0]); //Remove call to this function from stack trace
        foreach($trace as $key => $node) {

            $stack .= "#$i ";
            if (isset($node['file'])) {
                $stack .= $node['file'] ."(" .$node['line']."): ";
            }
            if ($key > 1) {
                if(isset($node['class'])) {
                    $stack .= $node['class'] . "->";
                }
                $stack .= $node['function'] . "()" . PHP_EOL;
            } else {
                $stack .= PHP_EOL;
            }
            $i++;
        }
        return $stack;
    }

    static function friendlyErrorType($type)
    {
        switch($type)
        {
            case E_ERROR: // 1 //
                return 'E_ERROR';
            case E_WARNING: // 2 //
                return 'E_WARNING';
            case E_PARSE: // 4 //
                return 'E_PARSE';
            case E_NOTICE: // 8 //
                return 'E_NOTICE';
            case E_CORE_ERROR: // 16 //
                return 'E_CORE_ERROR';
            case E_CORE_WARNING: // 32 //
                return 'E_CORE_WARNING';
            case E_CORE_ERROR: // 64 //
                return 'E_COMPILE_ERROR';
            case E_CORE_WARNING: // 128 //
                return 'E_COMPILE_WARNING';
            case E_USER_ERROR: // 256 //
                return 'E_USER_ERROR';
            case E_USER_WARNING: // 512 //
                return 'E_USER_WARNING';
            case E_USER_NOTICE: // 1024 //
                return 'E_USER_NOTICE';
            case E_STRICT: // 2048 //
                return 'E_STRICT';
            case E_RECOVERABLE_ERROR: // 4096 //
                return 'E_RECOVERABLE_ERROR';
            case E_DEPRECATED: // 8192 //
                return 'E_DEPRECATED';
            case E_USER_DEPRECATED: // 16384 //
                return 'E_USER_DEPRECATED';
        }
        return "UNKNOWN ERROR TYPE";
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
function newrelic_segment_external_begin(string $host, string $name): int;

<<__Native>>
function newrelic_segment_end(int $id): int;

<<__Native>>
function newrelic_get_scoped_generic_segment(string $name): mixed;

<<__Native>>
function newrelic_get_scoped_database_segment(string $table, string $operation): mixed;

<<__Native>>
function newrelic_get_scoped_external_segment(string $host, string $name): mixed;

<<__Native>>
function newrelic_transaction_set_max_trace_segments(int $threshold): int;

<<__Native>>
function newrelic_notice_error_intern(string $exception_type, string $error_message, string $stack_trace, string $stack_frame_delimiter): int;

<<__Native>>
function newrelic_add_attribute_intern(string $name, string $value): int;


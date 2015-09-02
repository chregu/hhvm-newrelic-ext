<?hh

//not implemented yet
function newrelic_set_appname(string $name, string $key, bool $xmit): mixed {}

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

function newrelic_start_transaction(string $appname = null, string $license = null): int {
    return newrelic_start_transaction_intern();
}

function newrelic_name_transaction(string $name): int {
    return newrelic_name_transaction_intern($name);
}

//not implemented yet
function newrelic_ignore_transaction() {}

//not implemented yet
function newrelic_ignore_apdex() {}

function newrelic_profiling_enable(int $level) {
    if (newrelic_has_native_hotprofiler()) {
        newrelic_set_external_profiler($level);
        xhprof_enable(0x400);
    } else {
        NewRelicExtensionHelper::setMaxDepth($level);
        fb_setprofile(array("NewRelicExtensionHelper","profile"));
        newrelic_add_custom_parameter("HotProfiler","emulated");
    }
}

function newrelic_profiling_disable() {
    if (newrelic_has_native_hotprofiler()) {
        xhprof_disable();
    } else {
        NewRelicExtensionHelper::endAll();
        fb_setprofile(null);
    }
}

function newrelic_has_native_hotprofiler() {
    return (defined("EXTERNAL_HOTPROFILER_FIX") || version_compare(HHVM_VERSION, "3.6", ">="));
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
function newrelic_segment_datastore_begin(string $table, string $operation, string $sql = "", string $sql_trace_rollup_name = ""): int;

<<__Native>>
function newrelic_segment_external_begin(string $host, string $name): int;

<<__Native>>
function newrelic_segment_end(int $id): int;

<<__Native>>
function newrelic_get_scoped_generic_segment(string $name): mixed;

<<__Native>>
function newrelic_get_scoped_database_segment(string $table, string $operation, string $sql = "", string $sql_trace_rollup_name = ""): mixed;

<<__Native>>
function newrelic_get_scoped_external_segment(string $host, string $name): mixed;

<<__Native>>
function newrelic_transaction_set_max_trace_segments(int $threshold): int;

<<__Native>>
function newrelic_notice_error_intern(string $exception_type, string $error_message, string $stack_trace, string $stack_frame_delimiter): int;

<<__Native>>
function newrelic_add_attribute_intern(string $name, string $value): int;

<<__Native>>
function newrelic_set_external_profiler(int $maxdepth = 7): void;

<<__Native>>
function newrelic_custom_metric(string $name, float $value): int;

/**
 *    Core Overrides
 */

// sql helper/parser
function _newrelic_parse_query($query): array {
	if (preg_match( '/^\s*SELECT/i', $query)) {
		if (preg_match('/\s+FROM\s+[`\'"]?([a-z\d_\.]+)[`\'"]?/i', $query, $match)) {
			return ['select', $match[1]];
		} else if (preg_match('/\s+[`\'"]?([a-z\d_\.]+)[`\'"]?/i', $query, $match)) {
			return ['select', $match[1]];
		} else {
			return ['select', 'unknown'];
		}
	} else if (preg_match( '/^\s*INSERT/i', $query)) {
		if (preg_match('/\s+INTO\s+[`\'"]?([a-z\d_\.]+)[`\'"]?/i', $query, $match)) {
			return ['insert', $match[1]];
		} else {
			return ['insert', 'unknown'];
		}
	} else if (preg_match( '/^\s*UPDATE/i', $query)) {
		if (preg_match('/UPDATE\s+[`\'"]?([a-z\d_\.]+)[`\'"]?/i', $query, $match)) {
			return ['update', $match[1]];
		} else {
			return ['update', 'unknown'];
		}
	} else if (preg_match( '/^\s*DELETE/i', $query)) {
		if (preg_match('/DELETE\s+[`\'"]?([a-z\d_\.]+)[`\'"]?/i', $query, $match)) {
			return ['delete', $match[1]];
		} else {
			return ['delete', 'unknown'];
		}
	} else if (preg_match( '/^\s*SHOW/i', $query)) {
		if (preg_match('/^\s*?(SHOW\s+[a-z\d_]+)/i', $query, $match)) {
			return ['select', $match[1]];
		} else {
			return ['select', 'SHOW'];
		}
	} else if (preg_match( '/^\s*BEGIN/i', $query)) {
		return ['select', 'BEGIN'];
	} else if (preg_match( '/^\s*COMMIT/i', $query)) {
		return ['select', 'COMMIT'];
	} else if (preg_match( '/^\s*ROLLBACK/i', $query)) {
		return ['select', 'ROLLBACK'];
        }

	return ['select', 'undefined'];
}

// PDO intercepts
function newrelic_pdo_intercept() {
    // PDO::exec and PDO::query will be harder due to lifecycle of objects
    //fb_intercept('PDO::exec', function ($name, $obj, $args, $data, &$done) { $done=false;});
    //fb_intercept('PDO::query', function ($name, $obj, $args, $data, &$done) { $done=false;});
    fb_intercept('PDOStatement::execute', function ($name, $obj, $args, $data, &$done) {
        $query = $obj->queryString;
        $a = _newrelic_parse_query($query);

        $obj->_newrelic_segment = newrelic_get_scoped_database_segment($a[1], $a[0], $query);
        $done=false;
    });
}

// mysqli
function newrelic_mysqli_intercept() {
    fb_intercept('mysqli::hh_real_query', function ($name, $obj, $args, $data, &$done) {
        if (isset($obj->_newrelic_segment) && $obj->_newrelic_segment) {
                newrelic_segment_end($obj->_newrelic_segment);
        }
        $query = $args[0];
        $a = _newrelic_parse_query($query);

        $obj->_newrelic_segment = newrelic_segment_datastore_begin($a[1], $a[0], $query);
        $done = false;
    });

    fb_intercept('mysqli::store_result', '_newrelic_mysqli_segment_end');

    fb_intercept('mysqli::use_result', '_newrelic_mysqli_segment_end');
}
function _newrelic_mysqli_segment_end($name, $obj, $args, $data, &$done) {
    if (isset($obj->_newrelic_segment) && $obj->_newrelic_segment) {
            newrelic_segment_end($obj->_newrelic_segment);
    }
    $obj->_newrelic_segment = 0;
    $done = false;
}




// file_get_contents (e.g. solr)
function newrelic_file_get_contents(string $filename, bool $use_include_path = false, resource $context = null, int $offset = -1, int $maxlen = -1) {
    $seg = newrelic_segment_external_begin($filename, 'file_get_contents');
    $resp = @obs_file_get_contents($filename, $use_include_path, $context, $offset, $maxlen);
    newrelic_segment_end($seg);
    return $resp;
}

function newrelic_file_get_contents_intercept() {
    fb_rename_function('file_get_contents', 'obs_file_get_contents');
    fb_rename_function('newrelic_file_get_contents', 'file_get_contents');
}


// fread and fwrite (e.g. Redis)
function newrelic_fread(resource $handle, int $length) {
    if (stream_get_meta_data($handle)['wrapper_type'] != 'plainfile') {
        $seg = newrelic_segment_external_begin('sock_read[' . stream_socket_get_name($handle,true) . ']', 'fread');
    } else {
        $seg = newrelic_segment_external_begin('file', 'fread');
    }
    $resp = @obs_fread($handle, $length);
    newrelic_segment_end($seg);
    return $resp;
}

function newrelic_fwrite( resource $handle, string $string, int $length = -1 ) {
    if (stream_get_meta_data($handle)['wrapper_type'] != 'plainfile') {
        $seg = newrelic_segment_external_begin('sock_write[' . stream_socket_get_name($handle,true) . ']', 'fwrite');
    } else {
        $seg = newrelic_segment_external_begin('file', 'fwrite');
    }
    $resp = @obs_fwrite($handle, $string, $length);
    newrelic_segment_end($seg);
    return $resp;
}

function newrelic_fread_fwrite_intercept() {
    fb_rename_function('fread', 'obs_fread');
    fb_rename_function('newrelic_fread', 'fread');

    fb_rename_function('fwrite', 'obs_fwrite');
    fb_rename_function('newrelic_fwrite', 'fwrite');
}

// curl
function newrelic_curl_exec( resource $ch ) {
    $url = curl_getinfo($ch, CURLINFO_EFFECTIVE_URL);
    $url = 'curl[' . substr($url, 0, strpos($url, '/', 8)) . ']';

    $seg = newrelic_segment_external_begin($url, 'curl');
    $resp = @obs_curl_exec($ch);
    newrelic_segment_end($seg);
    return $resp;
}
function newrelic_curl_intercept() {
    fb_rename_function('curl_exec', 'obs_curl_exec');
    fb_rename_function('newrelic_curl_exec', 'curl_exec');
}

// socket_read and socket_write (e.g. MongoDB (mongofill))
function newrelic_socket_read(resource $socket, int $length, int $type = PHP_BINARY_READ) {
    if (stream_get_meta_data($socket)['wrapper_type'] != 'plainfile') {
        $seg = newrelic_segment_external_begin('sock_read[' . stream_socket_get_name($socket,true) . ']', 'socket_read');
    } else {
        $seg = newrelic_segment_external_begin('file', 'socket_read');
    }
    $resp = @obs_socket_read($socket, $length. $type);
    newrelic_segment_end($seg);
    return $resp;
}

function newrelic_socket_write( resource $socket, string $string, int $length = 0 ) {
    if (stream_get_meta_data($socket)['wrapper_type'] != 'plainfile') {
        $seg = newrelic_segment_external_begin('sock_write[' . stream_socket_get_name($socket,true) . ']', 'socket_write');
    } else {
        $seg = newrelic_segment_external_begin('file', 'socket_write');
    }
    $resp = @obs_socket_write($socket, $string, $length);
    newrelic_segment_end($seg);
    return $resp;
}

function newrelic_socket_recv( resource $socket , &$buf , int $len , int $flags ) {
    if (stream_get_meta_data($socket)['wrapper_type'] != 'plainfile') {
        $seg = newrelic_segment_external_begin('sock_read[' . stream_socket_get_name($socket,true) . ']', 'socket_read');
    } else {
        $seg = newrelic_segment_external_begin('file', 'socket_read');
    }
    $resp = obs_socket_recv($socket, $buf, $len, $flags);
    newrelic_segment_end($seg);
    return $resp;
}

function newrelic_socket_read_write_intercept() {
    fb_rename_function('socket_read', 'obs_socket_read');
    fb_rename_function('newrelic_socket_read', 'socket_read');

    fb_rename_function('socket_write', 'obs_socket_write');
    fb_rename_function('newrelic_socket_write', 'socket_write');

    fb_rename_function('socket_recv', 'obs_socket_recv');
    fb_rename_function('newrelic_socket_recv', 'socket_recv');
}


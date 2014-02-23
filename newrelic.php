<?hh

//not implemented yet
function newrelic_set_appname(string $name, string $key, bool $xmit): mixed {}

function newrelic_start_transaction(string $appname, string $license = null) {
    newrelic_start_transaction_intern();
    newrelic_transaction_set_request_url($_SERVER["REQUEST_URI"]);
}

function newrelic_name_transaction(string $name) {
    newrelic_name_transaction_intern($name);
    newrelic_transaction_set_request_url($_SERVER["REQUEST_URI"]);
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




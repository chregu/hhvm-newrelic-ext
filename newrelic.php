<?hh
<<__Native>>
function hhvm_newrelic_transaction_begin(): int;

<<__Native>>
function hhvm_newrelic_transaction_set_name(string $name): int;

<<__Native>>
function hhvm_newrelic_transaction_set_request_url(string $name): int;

<<__Native>>
function hhvm_newrelic_transaction_set_threshold(int $threshold): int;

<<__Native>>
function hhvm_newrelic_transaction_end(): int;

<<__Native>>
function hhvm_newrelic_segment_generic_begin(string $name): int;

<<__Native>>
function hhvm_newrelic_segment_datastore_begin(string $table, string $operation): int;

<<__Native>>
function hhvm_newrelic_segment_end(int $id): int;

<<__Native>>
function hhvm_newrelic_get_scoped_generic_segment(string $name): mixed;

<<__Native>>
function hhvm_newrelic_get_scoped_database_segment(string $table, string $operation): mixed;

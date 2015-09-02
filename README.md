# (Unofficial) New Relic extension for HHVM

This is an unofficial New Relic extension for [HHVM](http://hhvm.com/) which implements the [New Relic PHP agent API](https://docs.newrelic.com/docs/agents/php-agent/configuration/php-agent-api) as much as possible.

It's build on the [New Relic SDK agent](https://docs.newrelic.com/docs/agents/agent-sdk/using-agent-sdk/getting-started-agent-sdk), which doesn't support all the features needed for rebuilding the PHP agent API and has some other caveats

It currently supports the following features:

* Reporting time of execution for each call to your HHVM
* Naming transactions
* Reporting errors, exceptions and notices
* Function level profiling (not by default, [see below](#using-function-level-profiling))
* Reporting of SQL and remote HTTP calls (not by default, [see below](#automatic-database-and-external-services-profiling))
* Add custom parameters
* Add custom metrics

Not supported/implemented

* Disabling collection of timing data or disabling a transaction (not implemented yet)
* All the ini options of the php agent
* Change App Name, License Key after startup of HHVM (not supported by the SDK agent)
* Get browser header/footer (not supported by the SDK agent)
* Auto-instrumentation for browser performance (not supported by the SDK agent)


For more info, see also the following blog posts:

* http://blog.newrelic.com/2014/02/10/agentsdk-blog-post/
* http://blog.liip.ch/archive/2014/03/27/hhvm-and-new-relic.html
* http://blog.liip.ch/archive/2015/01/19/new-relic-extension-for-hhvm-updated-to-latest-version.html



## Installation

This extension is known to work with HHVM 3.9. For older HHVM versions, see the other branches.

If you don't need function level profiling data or are using HHVM 3.7, you can use the HHVM packages provided by Facebook. If you want to use function level profiling on HHVM 3.5, [see below](#using-function-level-profiling-in-hhvm-35).

### Install the New Relic SDK files


Check on https://download.newrelic.com/agent_sdk/ for the latest version, currently the following works:

```
mkdir newrelic
cd newrelic
wget https://download.newrelic.com/agent_sdk/nr_agent_sdk-v0.16.1.0-beta.x86_64.tar.gz
tar -xzf nr_agent_sdk-*.tar.gz
cp nr_agent_sdk-*/include/* /usr/local/include/
cp nr_agent_sdk-*/lib/* /usr/local/lib/
```

### Install hhvm-dev

See https://github.com/facebook/hhvm/wiki/Prebuilt-Packages-for-HHVM about how to install HHVM in general on different operating systems

For compiling the extension, you need the dev package as well, eg. on Ubuntu/Debian with

```
sudo apt-get install hhvm-dev
```

### Compile the extension

```
git clone git@github.com:chregu/hhvm-newrelic-ext.git
cd hhvm-newrelic-ext

# the following is needed if you use HHVM 3.5, since one header file is missing
wget -O /usr/include/hphp/runtime/version.h https://raw.githubusercontent.com/facebook/hhvm/HHVM-3.5/hphp/runtime/version.h

hphpize
cmake .
make
make install
```

### Configuring hhvm

Add the lines from [php.ini](https://github.com/chregu/hhvm-newrelic-ext/blob/master/php.ini) to your php.ini and replace NEWRELIC_LICENSE_KEY and NEWRELIC_APP_NAME with your values. The minimum lines are (see the example file for more):

```
hhvm.env_variables[NEWRELIC_LICENSE_KEY] = YOUR_KEY_HERE
hhvm.env_variables[NEWRELIC_APP_NAME] = YOUR_APP_NAME
hhvm.extensions[]=newrelic.so
```

Now restart hhvm and you should be all set and profiling data should show up in New Relic. You don't have to change anything in your app.

## Using Function Level Profiling

When you use the PHP agent of New Relic, you're certainly used to have function level profiling, meaning you see which function took how much time. This is also possible with this extension, but with the following caveats:

* You have to explicitely enable it
* It slows down your request by factor 2 or more
* It can only show up to 2'000 function calls
* If you use an unpatched HHVM 3.5 or less (fixed in HHVM 3.6), it's even slower

### Enabling Function Level Profiling

Add this to your code where you want to start profiling

```
newrelic_profiling_enable($level)
```

The level tells the profiler how deep you want to collet data. Since New Relic doesn't show more than 2'000 entries, it's advised to not set that too high, but it depends on your app.

You can start it at any point in your application. If you only want to profile one command, just put it there.

### Using Function Level Profiling in HHVM 3.5

As mentioned above, an unpatched HHVM 3.5 (or less) makes your profiling event slower, since there's something wrong with using an external hotprofiler and we have to fall back to "user level profiling". It works, but it's slow.

You have two option to get it faster: Either install at least hhvm 3.6. Or compile HHVM 3.5 by yourself with this patch: https://github.com/chregu/hhvm/compare/facebook:HHVM-3.5...newrelic-profiling-3.5.diff, or do

````
git checkout https://github.com/chregu/hhvm.git
git checkout -b newrelic-profiling chregu/newrelic-profiling-3.5
cmake .
make
````

(this can take a while and you need all the HHVM dependencies, see other places about that)

### Why is Function Level Profiling slow?

It has to do a lot more, of course. But this extension is not slower than the built-in HHVM hotprofilers like xhprof. So not much we can do about it. Let's hope New Relic finds a way to make it faster with their hopefully coming HHVM agent.

Advice:

* Don't use Function Level Profiling if you don't absolutely need it (or just randomly every 100th request or so)
* Or only use it in small parts of your code
* Use the automatic database and external service profiling mentioned below
* Add your own segments



## Automatic Database and External Services profiling

This was contributed by Jared Kipe, see also https://jaredkipe.com/blog/website-development/newrelic-hhvm-automatic-segments/ for more details.

### For PDO / Datastore segments

Include this function somewhere in your application's entry script.

````
newrelic_pdo_intercept();
newrelic_mysqli_intercept();
````

### For External services through curl, file_get_contents, and fwrite/fread

You must explicitly enable function renaming

* CLI: `-d hhvm.jit_enable_rename_function=true`
* OR include in your php.ini file:

````
hhvm.jit_enable_rename_function=true
````

Include these functions somewhere in your application's entry script.

````
newrelic_file_get_contents_intercept();
newrelic_fread_fwrite_intercept();
newrelic_curl_intercept();
newrelic_socket_read_write_intercept();
````

## Add your own segments

To add your own segments, just call:

````
$id =  newrelic_segment_generic_begin($name);
// your code
newrelic_segment_end($id)
````

## The complete API

See [ext_newrelic.php](https://github.com/chregu/hhvm-newrelic-ext/blob/master/ext_newrelic.php) for all the available functions and their parameters.


## Symfony Test App

In the directory symfony-test, there's a small symfony app to check the functionality.
Put it somewhere, do `composer.phar install` and call http://localhost/fibo/20, this will calculate the fibonacci number at position 20 very inefficently. If everything is setup correctly, you should see some numbers on New Relic.




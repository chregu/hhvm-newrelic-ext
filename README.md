# (Unofficial) New Relic extension for HHVM

This is an unofficial New Relic extension for HHVM which tries to implement the [New Relic PHP agent API](https://docs.newrelic.com/docs/agents/php-agent/configuration/php-agent-api) as much as possible. 

It's build on the [New Relic SDK agent](https://docs.newrelic.com/docs/agents/agent-sdk/using-agent-sdk/getting-started-agent-sdk), which doesn't support all the features needed for rebuilding the PHP agent API and has some other caveats

It currently supports the following features:

* Reporting time of execution for each call to your HHVM
* Naming transactions 
* Reporting errors, exceptions and notices
* Function level profiling (not by default, see below)
* Reporting of SQL and remote HTTP calls (not by default, see below)
* Add custom parameters

Not supported/implemented

* Disabling collection of timing data or disabling a transaction (not implemented yet)
* Add custom metrics (not implemented yet)
* All the ini options of the php agent
* Change App Name, License Key after startup of HHVM (not supported by the SDK agent)
* Get browser header/footer (not supported by the SDK agent)
* Auto-instrumentation for browser performance (not supported by the SDK agent)


For more info, see also the following blog posts:

* http://blog.newrelic.com/2014/02/10/agentsdk-blog-post/
* http://blog.liip.ch/archive/2014/03/27/hhvm-and-new-relic.html
* http://blog.liip.ch/archive/2015/01/19/new-relic-extension-for-hhvm-updated-to-latest-version.html



## Installation

This extension is known to work with HHVM 3.5 and master (upcoming 3.6). For older HHVM versions, see the other branches.

If you don't need function level profiling data or are using HHVM master/nightly/3.6, you can use the HHVM packages provided by Facebook. If you want to use function level profiling on HHVM 3.5, see below. 

### First install the New Relic SDK files


Check on https://download.newrelic.com/agent_sdk/ for the latest version, currently the following works:

```
mkdir newrelic
cd newrelic
wget https://download.newrelic.com/agent_sdk/nr_agent_sdk-v0.16.1.0-beta.x86_64.tar.gz
tar -xzf nr_agent_sdk-*.tar.gz
cp nr_agent_sdk-*/include/* /usr/local/include/ &&  cp nr_agent_sdk-*/lib/* /usr/local/lib/ && rm -rf /tmp/nr_agent_sdk-*
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


## Configuring hhvm

The Agent SDK example ships with a sample hhvm config file called hhvm.hdf. You’ll need to make the following changes to your config file.

Replace NEWRELIC_LICENSE_KEY with your license key under the EnvVariables section
Set the newrelic path under the DynamicExtensions section to the library that you built in the previous step, e.g.: newrelic = /var/www/newrelic.so

Restart hhvm

## Using Function Level Profiling

There seems to be a problem with HHVM 3.4/3.5 if you want to use an external profiler (maybe the problem is on my side, not sure yet)
Until I (or hhvm) fixed this, you have to compile HHVM by yourself
The diff is just one line (https://github.com/chregu/hhvm/compare/facebook:HHVM-3.5...newrelic-profiling-3.5).  You can do the following

You don't need this, if you don't need function level profiling, but just "total time of script" reporting in New Relic.

* Follow the Installation prerequisites for the normal plugin, copying the newrelic agent_sdk library and header files to their necesary location
* Check out my hhvm branch with newrelic-profiling support
* Go to your hhvm sources

````
git remote add chregu https://github.com/chregu/hhvm.git
git checkout -b newrelic-profiling chregu/newrelic-profiling-3.5
````

````
cmake .
make
````
(this can take a while and you need all the HHVM dependencies, see other places about that)

## BETA - Automatic Database and External Services - jared@jaredkipe.com 7/11/2014

For PDO / Datastore segments:
Include this function somewhere in your application's entry script.
````
newrelic_pdo_intercept();
newrelic_mysqli_intercept();
````

For External services through curl, file_get_contents, and fwrite/fread:

You must explicitly enable function renaming
*CLI: `-v Eval.JitEnableRenameFunction=true`
*OR include in your hdf file:
````
Eval {
    JitEnableRenameFunction = true
}
````

Include these functions somewhere in your application's entry script.
````
newrelic_file_get_contents_intercept();
newrelic_fread_fwrite_intercept();
newrelic_curl_intercept();
newrelic_socket_read_write_intercept();
````

## Symfony Test App

In the directory symfony-test, there's a small symfony app to check the functionality.
Put it somewhere, do `composer.phar install` and call http://localhost/fibo/20, this will calculate
the fibonacci number at position 20 very inefficently. If everything is setup correctly, you should see
some numbers on New Relic.




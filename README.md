This extension tries to implement the New Relic PHP Plugin API with the help of the New Relic SDK.

The PHP API can be found here: https://docs.newrelic.com/docs/php/php-agent-api

# Installation

See also http://blog.newrelic.com/2014/02/10/agentsdk-blog-post/ and http://blog.liip.ch/archive/2014/03/27/hhvm-and-new-relic.htm

* Download the Agent SDK http://download.newrelic.com/agent_sdk/
* Copy the library files to /usr/lib or /usr/local/lib
* Copy the header files to /usr/include or /usr/local/include
* build the extension

````
export HPHP_HOME=~/dev/hhvm-profile/ #wherever your hhvm files are
$HPHP_HOME/hphp/tools/hphpize/hphpize
cmake .
make
````

This will create a library file named newrelic.so which you will point to when configuring hhvm.


# Configuring hhvm

The Agent SDK example ships with a sample hhvm config file called hhvm.hdf. You’ll need to make the following changes to your config file.

Replace NEWRELIC_LICENSE_KEY with your license key under the EnvVariables section
Set the newrelic path under the DynamicExtensions section to the library that you built in the previous step, e.g.: newrelic = /var/www/newrelic.so

Restart hhvm

# Using Auto-Instrumentation/Profiling

You have to build hhvm with HOTPROFILE support

* Follow the Installation prerequisites for the normal plugin, copying the newrelic agent_sdk library and header files to their necesary location
* Check out my hhvm branch with newrelic-profiling support
* Go to your hhvm sources

````
git remote add chregu https://github.com/chregu/hhvm.git
git checkout -b newrelic-profiling chregu/newrelic-profiling
````

* Build HHVM with HOTPROFILER Support

````
cmake -D HOTPROFILER:BOOL=ON .
make
````
(this can take a while and you need all the HHVM dependencies, see other places about that)


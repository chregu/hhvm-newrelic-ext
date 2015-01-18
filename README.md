This extension tries to implement the New Relic PHP Plugin API with the help of the New Relic SDK.

The PHP API can be found here: https://docs.newrelic.com/docs/php/php-agent-api

# Installation

See also http://blog.newrelic.com/2014/02/10/agentsdk-blog-post/ and http://blog.liip.ch/archive/2014/03/27/hhvm-and-new-relic.htm

* Download the Agent SDK http://download.newrelic.com/agent_sdk/
* Copy the library files to /usr/lib or /usr/local/lib:  `cp nr_agent_sdk*/lib/* /usr/local/lib/`
* Copy the header files to /usr/include or /usr/local/include:  `cp nr_agent_sdk*/include/* /usr/local/include/`
* build the extension

````
export HPHP_HOME=~/dev/hhvm-profile/ #wherever your hhvm files are
$HPHP_HOME/hphp/tools/hphpize/hphpize
cmake .
make
````

If you don't have the HHVM sources installed, you can also try this, if you installed one of the hhvm binaries for ubuntu/debian

````
apt-get install hhvm-dev
hphpize
cmake .
make
````

This will create a library file named newrelic.so which you will point to when configuring hhvm.

# Supported version

This branch is known to support HHVM 3.4 and theoretically also HHVM 3.3, see other branches for older/newer HHVM versions

# Configuring hhvm

The Agent SDK example ships with a sample hhvm config file called hhvm.hdf. You’ll need to make the following changes to your config file.

Replace NEWRELIC_LICENSE_KEY with your license key under the EnvVariables section
Set the newrelic path under the DynamicExtensions section to the library that you built in the previous step, e.g.: newrelic = /var/www/newrelic.so

Restart hhvm

# Using Auto-Instrumentation/Profiling

There seems to be a problem with HHVM 3.4/3.5 if you want to use an external profiler (maybe the problem is on my side, not sure yet)
Until I (or hhvm) fixed this, you have to compile HHVM by yourself
The diff is just one line (https://github.com/chregu/hhvm/compare/facebook:HHVM-3.5...newrelic-profiling-3.5).  You can do the following

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

# BETA - Automatic Database and External Services - jared@jaredkipe.com 7/11/2014

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


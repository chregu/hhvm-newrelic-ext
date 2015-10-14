FROM absalomedia/hhvm-dev

MAINTAINER Lawrence Meckan <media@absalom.biz>

RUN apt-get update \
  && apt-get -y install wget curl unzip git \
  && apt-get -y upgrade \
  && apt-get -y autoremove \
  && apt-get -y clean \
  && rm -rf /tmp/* /var/tmp/*

RUN mkdir -p /usr/src/newrelic && \
    cd /usr/src/newrelic
RUN echo "Downloading New Relic SDK ..." && wget https://download.newrelic.com/agent_sdk/nr_agent_sdk-v0.16.1.0-beta.x86_64.tar.gz
RUN tar -xzf nr_agent_sdk-*.tar.gz 
RUN cp nr_agent_sdk-*/include/* /usr/local/include/ && \
    cp nr_agent_sdk-*/lib/* /usr/local/lib/
RUN export CXX="g++-4.9"

WORKDIR /usr/src

RUN git clone https://github.com/absalomedia/hhvm-newrelic-ext.git && \
  cd hhvm-newrelic-ext && \
  hphpize && \
  cmake .  && \
  make && \
  make install
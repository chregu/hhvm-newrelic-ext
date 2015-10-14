FROM absalomedia/hhvm-dev

MAINTAINER Lawrence Meckan <media@absalom.biz>

RUN mkdir -p /usr/src/newrelic && \
    cd /usr/src/newrelic
RUN echo "Downloading New Relic Agent SDK ..." && wget -qO https://download.newrelic.com/agent_sdk/nr_agent_sdk-v0.16.1.0-beta.x86_64.tar.gz
RUN tar -xzf nr_agent_sdk-*.tar.gz && \
    cp nr_agent_sdk-*/include/* /usr/local/include/ && \
    cp nr_agent_sdk-*/lib/* /usr/local/lib/
RUN export CXX="g++-4.9"

WORKDIR /usr/src

RUN git clone https://github.com/absalomedia/hhvm-newrelic-ext.git && \
  cd hhvm-newrelic-ext && \
  hphpize && \
  cmake .  && \
  make && \
  make install && \
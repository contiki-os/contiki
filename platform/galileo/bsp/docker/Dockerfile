FROM ubuntu:16.04
RUN apt-get update \
  && apt-get install -y \
    gcc-multilib g++-multilib git make patch texinfo uuid-dev wget \
  && rm -rf /var/lib/apt/lists/*
WORKDIR /contiki
ENV TARGET galileo
CMD ["/bin/bash", "-l"]

FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
ENV PATH="$PATH:/root/.local/bin"

WORKDIR /workspace

COPY requirements.sh /tmp/requirements.sh

RUN chmod +x /tmp/requirements.sh && \
    bash /tmp/requirements.sh && \
    rm /tmp/requirements.sh

CMD ["/bin/bash"]

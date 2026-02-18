FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /workspace

# ---- System Python (Ubuntu default: 3.10) ----
RUN apt-get update && apt-get install -y \
    python3 \
    python3-venv \
    python3-pip \
 && rm -rf /var/lib/apt/lists/*

# ---- Python deps ----
COPY requirements.txt /tmp/requirements.txt
RUN python3 -m pip install --no-cache-dir -r /tmp/requirements.txt

# ---- System / toolchain deps ----
COPY requirements.sh /tmp/requirements.sh
RUN chmod +x /tmp/requirements.sh && \
    bash /tmp/requirements.sh && \
    rm /tmp/requirements.sh

CMD ["/bin/bash"]

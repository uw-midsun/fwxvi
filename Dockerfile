# docker run -it --rm \
# -v ~/projects/FW-docker/fwxvi:/workspace \
# fw-dev

# Ubuntu 22.04 (Jammy!!) pinned 
FROM ubuntu:22.04@sha256:3ba65aa20f86a0fad9df2b2c259c613df006b2e6d0bfcc8a146afb8c525a9751

ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /workspace

# ---- System Python (Ubuntu default: 3.10) ----
RUN apt-get update && apt-get install -y \
    python3=3.10.* \
    python3-venv=3.10.* \
    python3-pip=22.0.* \
 && rm -rf /var/lib/apt/lists/*

# ---- Python deps ----
COPY requirements.txt /tmp/requirements.txt
RUN python3 -m pip install --no-cache-dir -r /tmp/requirements.txt

RUN apt-get update && apt-get install -y \
    python3=3.10.* \
    python3-pip=22.0.* \
    python-is-python3 \
 && rm -rf /var/lib/apt/lists/*

RUN ln -sf /usr/bin/python3 /usr/bin/python

RUN pip3 install --upgrade scons scons-compiledb

# ---- System / toolchain deps ----
COPY requirements.sh /tmp/requirements.sh
RUN chmod +x /tmp/requirements.sh && \
    bash /tmp/requirements.sh && \
    rm /tmp/requirements.sh

CMD ["/bin/bash"]

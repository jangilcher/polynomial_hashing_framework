FROM sagemath/sagemath:10.0
WORKDIR /home/sage/framework

# Install packages
USER root
RUN apt-get update && apt-get install -y wget unzip build-essential libssl-dev


USER sage
# Install dependencies
COPY --chown=sage:sage requirements.txt ./requirements.txt
RUN sage -python -m pip install --no-cache-dir -r requirements.txt
COPY --chown=sage:sage setup_hacl.sh ./setup_hacl.sh
RUN ./setup_hacl.sh

RUN mkdir -p bench bin plots results obj asm


# Install libsodium
WORKDIR /home/sage/
RUN wget https://download.libsodium.org/libsodium/releases/old/libsodium-1.0.18-stable.tar.gz
RUN tar -xzf libsodium-1.0.18-stable.tar.gz

WORKDIR /home/sage/libsodium-stable
RUN ./configure
RUN make && make check
RUN sudo make install
RUN sudo ldconfig

WORKDIR /home/sage/
RUN rm libsodium-1.0.18-stable.tar.gz
RUN rm -rf libsodium-stable

WORKDIR /home/sage/framework

# Copy in the source code
COPY --chown=sage:sage src ./src
COPY --chown=sage:sage configs ./configs
COPY --chown=sage:sage tests ./tests
COPY --chown=sage:sage haberdashery_include ./haberdashery_include
COPY --chown=sage:sage include ./include
COPY --chown=sage:sage ref ./ref
COPY --chown=sage:sage rijndael_aead ./rijndael_aead
COPY --chown=sage:sage run.py ./run.py
COPY --chown=sage:sage Boost_LICENSE_1_0.txt ./Boost_LICENSE_1_0.txt
COPY --chown=sage:sage LICENSE ./LICENSE
COPY --chown=sage:sage Makefile ./Makefile

CMD ["bash"] 

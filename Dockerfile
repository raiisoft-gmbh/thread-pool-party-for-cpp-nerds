FROM alpine:3.18.4

RUN apk add --update-cache \
    bash \
    bash-completion \
    clang-extra-tools \
    cmake \
    g++ \
    gcovr \
    gdb \
    git \
    git-perl \
    less \
    # ninja \ Current version in alpine package repository isn't recent enough
    openssh-client \
    py3-autopep8 \
    py3-flake8 \
    python3 \
    sudo \
    vim

ARG NINJA_APK_NAME=ninja-1.10.1-r0.apk
RUN wget https://distfiles.adelielinux.org/adelie/1.0/user/x86_64/${NINJA_APK_NAME} && \
    apk add --allow-untrusted ${NINJA_APK_NAME} && \
    rm ${NINJA_APK_NAME}

WORKDIR /workspaces/thread-pool-party-for-cpp-nerds
COPY . .

# Create user 'pooler' and add him to the sudoers file
RUN adduser -s /bin/bash -G root -D -u 1001 pooler && \
    echo 'pooler ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers && \
    chown -R pooler: ${PWD} && \
    chmod 755 ${PWD}

USER pooler

# Modify terminal prompt
RUN echo 'export PS1="\[\e]0;\u@\h: \w\a\]${debian_chroot:+($debian_chroot)}\[\033[01;32m\]\u@\h\[\033[00m\]:\[\033[01;34m\]\w\[\033[00m\]\$ "' >> ~/.bashrc

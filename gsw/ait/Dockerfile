FROM registry.access.redhat.com/ubi8/ubi:8.9-1136

ARG USER=ait
ARG GROUP=ait
ARG UID=1001
ARG GID=1001
ARG HOME=/home/$USER
ARG VER=2.3.5
ENV LOG_LEVEL=INFO
ENV PROJECT_HOME=/home/$USER
ENV SETUP_DIR=/home/$USER/setup

# Install dependencies
RUN yum -y upgrade \
    && yum install -y wget gcc openssl-devel bzip2-devel libffi-devel sqlite-devel mod_ssl vim policycoreutils-python-utils cmake make git sudo nc gcc-c++ glibc-devel socat \
    && dnf install -y python3.9 python3-pip \
    && groupadd -r -g ${GID} ${GROUP} \
    && useradd -m -u ${UID} -g ${GROUP} ${USER}

# Install AIT
# Set variables for system install
USER ait
WORKDIR $PROJECT_HOME
ENV HOME=/home/ait
COPY --chown=${USER}:${GROUP} . $PROJECT_HOME/gsw-ait
# Prepare python environment for AIT work
RUN python3.9 -m pip install --user --upgrade pip setuptools \
    && python3.9 -m pip install --user virtualenv virtualenvwrapper poetry \
    && cd $PROJECT_HOME \
    && touch $PROJECT_HOME/.bashrc \
    && echo 'export VIRTUALENVWRAPPER_PYTHON=/usr/bin/python3.9' >> $PROJECT_HOME/.bashrc \
    && echo 'export WORKON_HOME=${PROJECT_HOME}/.virtualenvs' >> $PROJECT_HOME/.bashrc \
    && echo 'export PROJECT_HOME=${PROJECT_HOME}' >> $PROJECT_HOME/.bashrc \
    && echo 'export CONFIG_BUCKET_NAME=${CONFIG_BUCKET_NAME}' >> $PROJECT_HOME/.bashrc \
    && echo 'source ${PROJECT_HOME}/.local/bin/virtualenvwrapper.sh' >> $PROJECT_HOME/.bashrc \
    && source $PROJECT_HOME/.bashrc \
    && echo 'if [ $VIRTUAL_ENV ==  "/home/ait/.virtualenvs/gswait" ]; then' >> $PROJECT_HOME/.virtualenvs/postactivate \
    && echo 'export AIT_ROOT=${PROJECT_HOME}/.virtualenvs/gswait/lib/python3.9/site-packages/ait' >> $PROJECT_HOME/.virtualenvs/postactivate \
    && echo 'export AIT_CONFIG=${PROJECT_HOME}/gsw-ait/gswait/config/config.yaml' >> $PROJECT_HOME/.virtualenvs/postactivate \
    && echo 'fi' >> $PROJECT_HOME/.virtualenvs/postactivate \
    && cd gsw-ait \
    && mkvirtualenv gswait \
    && poetry install
ENTRYPOINT ["/usr/bin/bash","-c"]
CMD ["source /home/ait/.bashrc && cd gsw-ait && workon gswait && ait-server"]

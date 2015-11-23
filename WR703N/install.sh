#!/bin/sh

readonly libpthread_ipk=libpthread_0.9.33.2-1_ar71xx.ipk
readonly socat_ipk=socat_1.7.2.1-1_ar71xx.ipk
readonly socat_start_script=socat

function safe_download()
{
    target=$1
    if [ -z "${target}" ]; then
        echo nothing to download
        return
    fi
    
    wget http://${download_url}/${target}
    if [ $? -ne 0 ]; then
        echo "download ${target} failed"
        exit 1
    fi
}

function safe_install()
{
    target=$1
    if [ -z "${target}" ]; then
        echo nothing to install
        return
    fi
    
    opkg install ${libpthread_ipk}
    if [ $? -ne 0 ]; then
        echo "install ${target} failed"
        exit 1
    fi
}

function main()
{
    download_url=http://$1
    
    safe_download ${libpthread_ipk}
    safe_download ${socat_ipk}
    safe_download ${socat_start_script}

    safe_install ${libpthread_ipk}
    safe_install ${socat_ipk}


    cp ${socat_start_script} /etc/init.d/
    chmod a+x /etc/init.d/${socat_start_script}
    ln -s /etc/init.d/${socat_start_script} /etc/rc.d/S96socat

    echo install completed!
}


if [ -z "$1" ]; then
    echo "Usage: ./install.sh download-url"
    echo "e.g. ./install 192.168.1.1:8080"
    exit 1
fi

if [ `id -u` -ne 0 ]; then
    echo root user is required
    exit 1
fi

main $1
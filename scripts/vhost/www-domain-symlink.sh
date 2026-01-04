#!/usr/bin/env bash

set -euo pipefail
echo " symlink for www stuff!"
ln -sfn /home/core/code/neonsignal/public/simonedelpopolo.host \
       /home/core/code/neonsignal/public/www.simonedelpopolo.host

ln -sfn /home/core/code/neonsignal/public/nutsloop.host \
       /home/core/code/neonsignal/public/www.nutsloop.host
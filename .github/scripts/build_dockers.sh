#
# build_dockers.sh
#
# The MIT License
#
# Copyright (c) 2022 Omics Data Automation, Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

#!/bin/bash

VERSION=$(grep -E 'OMICSDS_RELEASE_VERSION ".+?" ' CMakeLists.txt | cut -d' ' -f 2 | tr -d '"')
BUILT_IMAGES=()

for TARGET in developer consumer; do
  docker build -f Dockerfile -t omicsds-lib:${TARGET} --target ${TARGET} .
  docker run \
    -e imageRef="omicsds-lib:${TARGET}" \
    -e exitCode='0' \
    -e severity='CRITICAL,HIGH' \
    -e format='table' \
    -e githubToken="${DEVELOPER_TOKEN}" \
    --volume=$(which docker):/usr/local/bin/docker:ro \
    --volume=/var/run/docker.sock:/var/run/docker.sock:ro \
    ${DOCKER_REGISTRY}/trivy-pipe:0.22.0
  case $TARGET in
    developer)
      IMAGE_NAME=omicsds-lib-developer
      ;;
    consumer)
      IMAGE_NAME=omicsds-lib
      ;;
  esac
  docker tag omicsds-lib:${TARGET} ${DOCKER_REGISTRY}/${IMAGE_NAME}:${VERSION}
  BUILT_IMAGES+=( "${DOCKER_REGISTRY}/${IMAGE_NAME}:${VERSION}" )
done


for BUILT_IMAGE in ${BUILT_IMAGES[@]}; do
  docker push ${BUILT_IMAGE}
done

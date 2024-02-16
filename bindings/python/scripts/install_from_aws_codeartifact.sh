# Copyright (c) 2024 dātma, inc™
#
# Proprietary and Confidential:
# Unauthorized copying of this file by any medium is strictly prohibited.
#
#!/bin/bash

CODEARTIFACT_DOMAIN=oda-dev
CODEARTIFACT_DOMAIN_OWNER=037083108678
AWS_DEFAULT_REGION=us-west-2
CODEARTIFACT_REPO=ODA-dev
CODEARTIFACT_AUTH_TOKEN=$(aws codeartifact get-authorization-token --domain ${CODEARTIFACT_DOMAIN} --domain-owner ${CODEARTIFACT_DOMAIN_OWNER} --query authorizationToken --output text)
PIP_EXTRA_INDEX_URL="https://aws:${CODEARTIFACT_AUTH_TOKEN}@${CODEARTIFACT_DOMAIN}-${CODEARTIFACT_DOMAIN_OWNER}.d.codeartifact.${AWS_DEFAULT_REGION}.amazonaws.com/pypi/${CODEARTIFACT_REPO}/simple/"
echo $PIP_EXTRA_INDEX_URL
pip install --extra-index-url $PIP_EXTRA_INDEX_URL omicsds 


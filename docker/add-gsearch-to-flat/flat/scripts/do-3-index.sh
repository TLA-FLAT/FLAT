#!/bin/bash

_PWD=$PWD

mkdir -p /app/flat/tmp
cd /app/flat/tmp

wget -t 1 --user fgsAdmin --password fgsAdmin -O do-search-result.html "http://localhost:8080/fedoragsearch/rest?operation=updateIndex&action=fromFoxmlFiles&value="

cd $_PWD
#!/bin/bash
wget --user fgsAdmin --password fgsAdmin -O do-search-result.html "http://localhost:8080/fedoragsearch/rest?operation=updateIndex&action=fromFoxmlFiles&value="

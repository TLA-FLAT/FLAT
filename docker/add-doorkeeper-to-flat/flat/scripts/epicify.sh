#!/bin/bash

java -Djsse.enableSNIExtension=false -jar /app/flat/lib/epicify.jar $*

#!/bin/bash
if [ $# -ne 2 ];then
  echo "Not enough parameters specified"
  echo "Please provide user acronym and bag_id of the project"
  exit 1
else
  user=$1
  bag_id=$2
fi

if [ -d /app/flat/deposit/${user}_temp ]; then
  rm -rf /app/flat/deposit/${user}_temp
fi

if [ -d /app/flat/deposit/bags/$bag_id ]; then
  rm -rf /app/flat/deposit/bags/$bag_id
fi

exit 0

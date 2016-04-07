#!/bin/bash
if [ $# -ne 2 ];then
  echo "Not enough parameters specified"
  echo "Please provide user acronym and project name"
  exit 1
else
  user=$1
  project=$2
fi

base_dir=/app/flat

bag_path=${base_dir}/deposit/bags/*/${project}
bag_id=$(echo $bag_path | awk -F 'bags' '{print $2}' | awk -F "/" '{print $2}')

echo $bag_id

for d in cmd fox fox-error
do
  if [ -d $base_dir/$d ];then
    rm -rf $base_dir/$d
  fi
done

/app/flat/do-0-convert.sh
/app/flat/do-1-fox.sh
/app/flat/do-2-import.sh
/app/flat/do-4-index.sh

exit 0



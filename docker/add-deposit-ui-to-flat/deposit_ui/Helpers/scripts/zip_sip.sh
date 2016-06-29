#!/bin/bash
if [ $# -ne 4 ];then
  echo "Not enough parameters specified"
  echo "Please provide user acronym, bundle name, bag_dir and user_bag_dir"
  exit 1

else
  user=$1
  project=$2
  bag_dir=$3
  user_bag_dir=$4
fi

nFiles=$(find "$user_bag_dir/" -type f ! -name 'bag-info.txt' ! -name 'bagit.txt' ! -name '*manifest-md5.txt' | wc -l)

if [ $nFiles -eq 0 ];then
  echo "No files found to be zipped at $user_bag_dir/"
  exit 1
fi

# show number of files to be ingested
echo "nFiles to ingest: $nFiles"

#zip all unhidden files
cd ${user_bag_dir}/..
zip -r ${project} ${project} -x ".*" -x "*/.*"

exit $?

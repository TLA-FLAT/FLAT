#!/bin/bash
if [ $# -ne 3 ];then
  echo "Not enough parameters specified"
  echo "Please provide user acronym and project name"
  exit 1
else
  user=$1
  project=$2
bag_dir=$3

fi

#bag_dir="/app/flat/deposit/bags"
user_bag_dir="$bag_dir/${user}_temp"


nFiles=$(find "$user_bag_dir/$project" -type f ! -name 'bag-info.txt' ! -name 'bagit.txt' ! -name '*manifest-md5.txt' | wc -l)
echo "nFiles to ingest: $nFiles"

echo $(dirname "$0")/$(basename "$0")
echo "date modified: " $(date -r $(dirname "$0")/$(basename "$0"))

echo "preparing bag for doorkeeper..."

cd ${user_bag_dir}/

#zip all unhidden files
zip -r ${project} ${project} -x ".*" -x "*/.*"

#prepare bag for doorkeeper
/app/flat/do-sword-upload.sh $user_bag_dir/${project}.zip
sleep 3

#Validate (needs improvemnet when in production)
bag_id=$(find $bag_dir -type d -name ${project} | awk -F 'bags' '{print $2}' | awk -F "/" '{print $2}')
if [ $(echo $bag_id | wc -c) -gt 0 ];then
  mess="Successfully created bag for project $project."
  mess2="The_Bag_ID_is: $bag_id"
  exit_code=0
  chmod -R 777 $bag_dir/$bag_id
else
  mess="error creating bag for project $project"
  exit_code=1

  cat /app/deposit/sword/tmp/$bag_id/deposit.properties
  rm -rf /app/deposit/sword/tmp/$bag_id/

fi
echo "\n"
echo $mess
echo $mess2
exit $exit_code



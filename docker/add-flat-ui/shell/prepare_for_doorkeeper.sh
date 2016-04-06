if [ $# -ne 2 ];then
  echo "Not enough parameters specified"
  echo "Please provide user acronym and project name"
  exit 1
else
  user=$1
  project=$2
fi

export PATH=/usr/local/bin/bagit-4.9.0/bin/:$PATH
#user=admin
#project=Test
bag_dir="/app/flat/deposit/bags"
user_bag_dir="/app/flat/deposit/${user}_temp"
user_server_data=/var/www/html/drupal/sites/default/files/users/${user}/

#Remove existing files
echo "cleaning folders"
if [ -d ${user_bag_dir}/${project} ];then
  rm -rf ${user_bag_dir}/${project}
fi

#Create temp dir
if [ ! -d ${user_bag_dir} ];then
  mkdir -p ${user_bag_dir}
fi

#copy project data to temp bag location (no symlinks because of right issues)
echo "copying $project data"
cp -r ${user_server_data}/${project}/data ${user_bag_dir}/${project}

#prepare bag
bag baginplace ${user_bag_dir}/${project}

#cp meta data
echo "copying $project metadata"
cp -r ${user_server_data}/${project}/metadata ${user_bag_dir}/${project}/metadata

#update checksum of bag
bag update ${user_bag_dir}/${project}

# In case bag is validated
if [ $(bag verifyvalid ${user_bag_dir}/${project} | awk -F " " '{print $3}' | tr -d ".") == 'true' ];then
  echo "preparing bag for doorkeeper..."

  cd ${user_bag_dir}/
  #zip all unhidden files
  zip -r ${project} ${project} -x ".*" -x "*/.*"

  #prepare bag for doorkeeper
  /app/flat/do-sword-upload.sh $user_bag_dir/${project}.zip

  #Check for bag ID
  bag_path=${user_bag_dir}/../bags/*/${project}
  bag_id=$(echo $bag_path | awk -F 'bags' '{print $2}' | awk -F "/" '{print $2}')

  #Validate
  if [ -d /app/flat/deposit/bags/$bag_id/${project} ];then
    mess="Successfully created bag for project $project. Bag ID is $bag_id"
    exit_code=0
  else
    mess="error creating bag for project $project"
    exit_code=1
    cat /app/deposit/sword/tmp/$bag_id/deposit.properties
    rm -rf /app/deposit/sword/tmp/$bag_id/

  fi

else
  mess="Invalid bag for project $project"
  exit_code=1
fi

#clean up
rm -rf ${user_bag_dir}/${project}

echo $mess
exit $exit_code






if [ $# -ne 2 ];then
  echo "Not enough parameters specified"
  echo "Please provide user acronym and project name"
  exit 1
else
  user=$1
  project=$2
fi
#user=admin
#project="Test2"


base_dir=/app/flat

bag_path=${base_dir}/deposit/bags/*/${project}
bag_id=$(echo $bag_path | awk -F 'bags' '{print $2}' | awk -F "/" '{print $2}')

echo $bag_id


for d in cmd fox fox-error deposit/bags/$bag_id
do
  if [ -d $base_dir/$d ];then
    rm -rf $base_dir/$d
  fi
done

unlink /var/www/owncloud/data/$user/files/${project}

sudo -u www-data php /var/www/owncloud/occ  files:scan --path="${user}/files"

rm -rf /var/www/html/drupal/sites/default/files/users/$user/${project}

exit 0


